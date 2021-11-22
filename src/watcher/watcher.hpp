
#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <Psapi.h>
#include <chrono>

#include <future>

#include <string>
#include <map>
#include <vector>
#include <array>

using namespace std;
using namespace std::chrono;

struct Proc {
	DWORD pID;
	DWORD cThread;
	string fullName;
	string name;
	unsigned int time;
};
map<HWND, Proc*> ActiveProcess;
static TCHAR fullName[MAX_PATH] = { 0 };

string normalizeTime(unsigned long long time) {
	int s = time % 60; time /= 60;
	int m = time % 60; time /= 60;
	int h = time % 24;
	return to_string(h) + "h " + to_string(m) + "m " + to_string(s) + "s";
}

HWND createProc() {
	HWND fgHandle = GetForegroundWindow();

	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	CreateProcess(
		"D:\\Proj\\empty.exe",
		LPSTR(""),
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&info,
		&processInfo
	);

	while (true) {
		HWND tmp = GetForegroundWindow();
		if (fgHandle != tmp) {
			fgHandle = tmp;
			break;
		}
		_sleep(100);
	}

	//cout << "HWND:    " << fgHandle << endl;
	//cout << "Handler: " << processInfo.hProcess << endl;
	//cout << "         " << processInfo.hThread << endl;

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
	return fgHandle;
}

#include <filesystem>
using namespace filesystem;

int startWatcher() {
	if (false) {
		// checking  collision when HWND poiners
		// result: collision can not be?

		constexpr int LEN = 1000;
		//vector<HWND> fgs;
		array<HWND, LEN> fgs;
		for (int i = 0; i < LEN; ++i) {
			HWND fg = createProc();
			cout << fg;
			auto it = find(fgs.begin(), fgs.end(), fg);
			if (it != fgs.end()) {
				cout << "collision found";
				break;
			}
			else {
				fgs[i] = move(fg);
			}
			cout << endl;
		}
		//sort(fgs.begin(), fgs.end());
		//cout << endl;
		//for (const auto& fg : fgs)
		//	cout << fg << endl;

		return 0;
	}

	setlocale(LC_ALL, "ru");

	int iters = 0;
	int summ = 0;

	// move cursor in console
	if (false) {
		HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
		cout << "Cur diff = [ns] ";
		while (true) {
			steady_clock::time_point curb = steady_clock::now();
			SetConsoleCursorPosition(output, { 16, 0 });
			steady_clock::time_point cure = steady_clock::now();
			cout << duration_cast<nanoseconds> (cure - curb).count();
			_sleep(1000);
		}
		CloseHandle(output);
		return 0;
	}

	unsigned long long i = 0;
	while (true) {
		cout << endl;
		_sleep(1000);

		steady_clock::time_point allBegin = steady_clock::now();

		HWND fgHandle = GetForegroundWindow();
		cout << "Active window: " << fgHandle << "\t iteration: " << i << endl;
		steady_clock::time_point procBegin = steady_clock::now();
		if (fgHandle) {
			auto it = ActiveProcess.find(fgHandle);
			if (it != ActiveProcess.end()) {
				it->second->time++;
			}
			else {
				Proc* proc = new Proc();
				proc->cThread = GetWindowThreadProcessId(fgHandle, &proc->pID);
				if (proc->pID || proc->cThread) {
					HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, proc->pID);
					if (hProc) {
						DWORD nlen = GetModuleFileNameEx((HANDLE)hProc, nullptr, fullName, MAX_PATH);
						CloseHandle(hProc);

						if (nlen) {
							proc->fullName = fullName;
							proc->name = proc->fullName.substr(proc->fullName.rfind('\\') + 1);
							proc->time = 0;

							ActiveProcess.emplace(fgHandle, proc);
						}
						else {
							cout << "error: nlen: " << nlen << " ";
							delete proc;
						}
					}
					else {
						cout << "error: hProc: " << &hProc << " ";
						delete proc;
					}
				}
				else {
					cout << "error: pId: " << proc->pID
						<< " cThread: " << proc->cThread;
					delete proc;
				}
			}
		}
		else {
			cout << "error handle is " << fgHandle;
		}

		steady_clock::time_point procEnd = steady_clock::now();

		WINDOWINFO* winfo = new WINDOWINFO();
		winfo->cbSize = sizeof(WINDOWINFO);
		for (const auto& it : ActiveProcess) {
			//steady_clock::time_point Get1B = steady_clock::now();
			//bool isAlive1 = GetWindowInfo(it.first, winfo); // too lazy
			//steady_clock::time_point Get1E = steady_clock::now();
			steady_clock::time_point Get2B = steady_clock::now();
			DWORD isAlive2 = GetWindowThreadProcessId(it.first, 0); // fasters than 10x
			steady_clock::time_point Get2E = steady_clock::now();

			cout << normalizeTime(it.second->time) << "  " << it.second->name
				//<< (isAlive1 ? " alive1" : " expired1")
				<< (isAlive2 ? " alive2" : " expired2")
				//<< " fir: " << duration_cast<nanoseconds> (Get1E - Get1B).count() << "[ns]"
				<< " sec: " << duration_cast<nanoseconds> (Get2E - Get2B).count() << "[ns]"
				<< endl;
		}

		steady_clock::time_point allEnd = steady_clock::now();
		std::cout << "get proc data diff = " << duration_cast<nanoseconds> (procEnd - procBegin).count() << "[ns]" << std::endl;
		std::cout << "Summary difference = " << duration_cast<nanoseconds> (allEnd - allBegin).count() << "[ns]" << std::endl;

		i++;
		continue;


		summ += duration_cast<microseconds> (allEnd - allBegin).count();
		std::cout << "Time difference = " << duration_cast<microseconds>(allEnd - allBegin).count() << "[µs]" << std::endl;
		std::cout << "Time difference = " << duration_cast<nanoseconds> (allEnd - allBegin).count() << "[ns]" << std::endl;

		_sleep(1000);
		i++;
		iters++;
	}

	cout << "is: " << summ / iters;
	// 124

	return 0;
}
