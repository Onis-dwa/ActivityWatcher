
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
#include <algorithm>
#include <execution>

#include "utility.hpp"

using namespace std;
using namespace std::chrono;
typedef unsigned int uint;
typedef unsigned long long ull;

struct ProcData {
	ProcData(string&& fn): fullName(fn), time(0) {
		nameFromFullName(fullName, name);
	}

	string fullName;
	string name;
	ull time;
};
vector<ProcData> processInfo; // all process
static size_t pSize = processInfo.size();
map<HWND, size_t> chacheProcess; // active process
static bool isRunning = false;
static ull interval = 100; // timer interval
HWND currH = nullptr; // current window

ull lastGet = 0;
ull longestGet = 0;
ull longestPrint = 0;
constexpr ull ARR_CNT = 25;
array<ull, ARR_CNT> procGet { 0 };
array<ull, ARR_CNT> printInf { 0 };

// only for GetModuleFileNameEx
static TCHAR fullName[MAX_PATH] = { 0 };

// TODO err log
inline auto findInProcInfo(string& fn) {
	return find_if(
		std::execution::parallel_unsequenced_policy(),
		processInfo.cbegin(),
		processInfo.cend(),
		[&](const ProcData& i) {return i.fullName == fn; });
}
inline size_t registryInProccInfo(string&& fn) {
	auto it = findInProcInfo(fn);
	if (it == processInfo.cend()) {
		auto ref = processInfo.emplace_back(ProcData(move(fn)));
		pSize = processInfo.size();
		if (processInfo[pSize - 1].fullName == ref.fullName)
			return pSize - 1;
		// errLog wtf?
		it = findInProcInfo(ref.fullName);
		if (it == processInfo.cend())
			return -1; // never happen (i guess)
	}
	
	return it - processInfo.cbegin(); // 0 .. size - 1
}
inline string getFullName(HWND& hw) {
	DWORD pID;
	DWORD cThread = GetWindowThreadProcessId(hw, &pID);
	if (pID || cThread) {
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pID);
		if (hProc) {
			DWORD nlen = GetModuleFileNameEx((HANDLE)hProc, nullptr, fullName, MAX_PATH);
			CloseHandle(hProc);

			if (nlen) {
				return fullName;
			}
			// error nlen
		}
		CloseHandle(hProc);
		// errLog hProc
	}
	// errLog pID cThread
	return "";
}
inline size_t getProcInfo(HWND& hw) {
	string hwFullName = getFullName(hw);
	if (hwFullName.empty())
		return (size_t)-1; // MAX

	auto it = chacheProcess.find(hw);
	if (it != chacheProcess.end())
		if ((it->second <= pSize) && (processInfo[it->second].fullName == hwFullName))
			return it->second;
	size_t pos = registryInProccInfo(move(hwFullName));
	chacheProcess[hw] = pos;
	return pos;
}
void watch() {
	size_t currI = -1;
	auto it = procGet.begin();
	auto begin = steady_clock::now(); 
	while (isRunning) { // count
		HWND fgH = GetForegroundWindow();
		if (fgH) { // because `can be NULL in certain circumstances` (docs.microsoft)
			if (currH != fgH) // change currI
				currI = getProcInfo(currH = fgH);
			
			if (currI < pSize)
				processInfo[currI].time += interval;
		}

		// check does it manage to be completed in the allotted time
		if (lastGet > longestGet) longestGet = lastGet;
		(*it) = lastGet;
		if (++it == procGet.end())
			it = procGet.begin();
		lastGet = duration_cast<milliseconds>(steady_clock::now() - begin).count();

		begin += milliseconds(interval);
		this_thread::sleep_until(begin);
	}
	cout << "async stopping" << endl;
	return;
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
void findCollision() {
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
}

string normalizeMS(uint& ms) {
	if (ms) {
		if (ms > 99)
			return to_string(ms) + "ms ";
		else if (ms > 9)
			return " " + to_string(ms) + "ms ";
		else
			return "  " + to_string(ms) + "ms ";
	}
	return "      ";
}
string normalizePart(uint& part, string&& name) {
	if (part) {
		if (part >= 10)
			return to_string(part) + name;
		else
			return " " + to_string(part) + name;
	}
	else
		return "    ";
}
string normalizeMSTime(ull time) {
	uint ms = time % 1000; time /= 1000;
	uint  s = time % 60;

	if (s == 0 && ms == 0)
		return "     >1ms ";
	return normalizePart(s, "s ")
		+ normalizeMS(ms);
}
string normalizeTime(ull time) {
	uint ms = time % 1000; time /= 1000;
	uint  s = time % 60; time /= 60;
	uint  m = time % 60; time /= 60;
	uint  h = time % 24;

	return normalizePart(h, "h ")
		+ normalizePart(m, "m ")
		+ normalizePart(s, "s ");
}

int startWatcher() {
	setlocale(LC_ALL, "ru");

	// run-time stat
	auto it = printInf.begin();
	ull summPrint = 0;
	ull summGet = 0;
	ull lastDiff = 0;

	// unchangeable vars
	HWND myhw = GetConsoleWindow();
	HANDLE outH = GetStdHandle(STD_OUTPUT_HANDLE);

	// control vars
	CONSOLE_SELECTION_INFO selectionInf;
	WINDOWPLACEMENT wplace;
	wplace.length = sizeof(WINDOWPLACEMENT);

	cout << endl << "press to start watch";
	//cin.get();
	auto startTime = steady_clock::now();
	auto workingTime = startTime;

	// start watcher
	isRunning = true;
	auto watcher = async(launch::async, watch);

	// printing
	SetConsoleCursorPosition(outH, { 0, 4 });
	cout << "       average    maximum     last" << endl;
	while (true) { // draw
		// check window is minimize
		if (GetWindowPlacement(myhw, &wplace)
			&& ((wplace.showCmd == SW_HIDE) ||
				(wplace.showCmd == SW_SHOWMINIMIZED) ||
				(wplace.showCmd == SW_MINIMIZE) ||
				(wplace.showCmd == SW_SHOWMINNOACTIVE) ||
				(wplace.showCmd == SW_FORCEMINIMIZE))) {
			workingTime += milliseconds(2000);
			this_thread::sleep_until(workingTime);
		}
		else {
			// ignore console input (because all cout && now() freezes and return invalid values)
			if (GetConsoleSelectionInfo(&selectionInf) && (selectionInf.dwFlags == 0)) {
				summPrint = accumulate(printInf.begin(), printInf.end(), 1);
				summGet = accumulate(procGet.begin(), procGet.end(), 1);
				SetConsoleCursorPosition(outH, { 0, 5 });

				cout << "cout  "
					<< normalizeMSTime(summPrint / ARR_CNT) << " "
					<< normalizeMSTime(longestPrint) << " "
					<< normalizeMSTime(lastDiff) << endl;
				cout << " get  "
					<< normalizeMSTime(summGet / ARR_CNT) << " "
					<< normalizeMSTime(longestGet) << " "
					<< normalizeMSTime(lastGet) << endl;

				cout << "Window: " << currH
					<< "  workTime: " << normalizeTime(
						duration_cast<milliseconds>(workingTime - startTime).count())
					<< endl;
				for (const auto& it : processInfo)
					cout << normalizeTime(it.time) << "  " << it.name << endl;

				// run-time stat
				if (lastDiff > longestPrint) longestPrint = lastDiff;
				(*it) = lastDiff;
				if (++it == printInf.end())
					it = printInf.begin();

				lastDiff = duration_cast<milliseconds>(steady_clock::now() - workingTime).count();
			}

			if ((currH == myhw))
				workingTime += milliseconds(250);
			else
				workingTime += milliseconds(1000);
			this_thread::sleep_until(workingTime);
		}
	}
	CloseHandle(outH);
	return 0;
}
