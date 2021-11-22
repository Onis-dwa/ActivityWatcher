#include <iostream>
#include <filesystem>
#include <string_view>
using namespace std::literals::string_view_literals;
using std::string_view;

#include "exitCodes.hpp"
#include "utility.hpp"
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
	
	constexpr string_view CMD_HELP    = "--help"sv;
	constexpr string_view CMD_DATADIR = "--datadir"sv;
	constexpr string_view CMD_PROFILE = "--profile"sv;
	constexpr std::array commands {
		CMD_HELP, CMD_DATADIR, CMD_PROFILE
	};
	
	constexpr size_t CMD_CNT      = commands.size();
	constexpr size_t CMDI_HELP    = getIndxByValue(commands, CMD_HELP);
	constexpr size_t CMDI_DATADIR = getIndxByValue(commands, CMD_DATADIR);
	constexpr size_t CMDI_PROFILE = getIndxByValue(commands, CMD_PROFILE);

	auto collisions = std::array<bool, CMD_CNT> { false };
	for (unsigned int i = 1; i < argc; ++i) {
		auto it = std::find(commands.cbegin(), commands.cend(), argv[i]);

		if (it != commands.cend()) {
			auto indx = it - commands.cbegin();
			if (indx < 0 || indx >= CMD_CNT) {
				cout << "Error: arg parse command indx out of range: "
					<< indx << endl;
				return base::exitCodes::ARG_INDX_FAIL;
			}
			if (collisions.at(indx)) {
				cout << "Error: arg parse duplicate command: "
					<< argv[i] << endl;
				return base::exitCodes::ARG_COLLISION;
			}
			collisions[indx] = true;

			switch (indx) {
			case CMDI_HELP:
				cout << "Help!"; // TODO draw help
				return base::exitCodes::ARG_HELP;
			case CMDI_DATADIR:
				dataDir = move(argv[++i]);
				break;
			case CMDI_PROFILE:
				profileName = move(argv[++i]);
				break;
			}
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

