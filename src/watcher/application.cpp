#include <iostream>
#include <filesystem>

#include <string_view>
using namespace std::literals::string_view_literals;
using std::string_view;

#include <exitCodes.hpp>
#include "application.h"

using std::cout;
using std::endl;
using std::move;

int Application::load(int&& argc, char**&& argv) {
	if(auto rc = processArguments(move(argc), move(argv))) return rc;

	std::filesystem::current_path(dataDir);
	cout << "cwd: " << std::filesystem::current_path() << endl;
	cout << "argc: " << argc << endl;
	for (int i = 1; i < argc; ++i)
		cout << "\t" << argv[i] << endl;

	// whitch to datadir
	// create config by cfgname || default name
	// create db by dbname || default name

	// load registred app by path? or name?
	// store app by id

	// create win32Wathcer
	// or	  unixWatcher
	return base::exitCodes::SUCCESS;
}

int Application::exec() {
	//int i = 0;
	//std::cin >> i;

	// run watcher

	return base::exitCodes::SUCCESS;
}

int Application::processArguments(int&& argc, char**&& argv) {
	if (!parseFirstArg(argv[0])) return base::exitCodes::ARG_FIRST_FAIL;

	enum class KeyFormat {
		NoValues,
		OneValue,
	};

	auto commands = std::array<string_view, 3> {
		"--help"sv, "--datadir"sv, "--profile"sv
	};

	cout << "pn: " << profileName << endl;
	for (unsigned int i = 1; i < argc; ++i) {
		auto it = std::find(commands.cbegin(), commands.cend(), argv[i]);

		if (it != commands.cend()) {
			auto indx = it - commands.cbegin();

			switch (indx) {
			case 0:
				cout << "Help!"; // TODO draw help
				return base::exitCodes::ARG_HELP;
			case 1:
				dataDir = move(argv[++i]);
				break;
			case 2:
				profileName = move(argv[++i]);
				break;
			default:
				cout << "Error: arg parse command indx out of range.\n";
				return base::exitCodes::ARG_SWITCH_FAIL;
			}
			continue;
		}
		else {
			cout << endl << "Wrong parametr at " << i << " <" << argv[i] << ">\n";
			return base::exitCodes::ARG_FIND_FAIL;
		}
	}

	if (dataDir.empty())
		dataDir = appDir;

	return base::exitCodes::SUCCESS;
}

bool Application::parseFirstArg(char*& arg) {
	appDir = arg;
	auto pos = appDir.rfind(std::filesystem::path::preferred_separator);
	appName = move(appDir.substr(pos + 1));
	appDir.erase(pos);

	if (appDir.empty() || appName.empty())
		return false;
	return true;
}

