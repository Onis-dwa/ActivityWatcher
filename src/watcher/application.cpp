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
namespace fs = std::filesystem;

// temp
#include <pugixml.hpp>
#include <sqlite3.h>
#include <boost/asio.hpp>
using namespace boost::asio;
using ip::tcp;
using boost::system::error_code;
#include "watcher.hpp"


int Application::load(int&& argc, char**&& argv) {
	if(auto rc = processArguments(move(argc), move(argv))) return rc;
	cout << "dataDir: " << dataDir << endl;
	cout << "profileName: " << profileName << endl;
	
	fs::current_path(dataDir);
	if (!fs::exists(profileName))
		fs::create_directory(profileName);
	fs::current_path(profileName);

	cout << "cwd: " << fs::current_path() << endl;
	return base::exitCodes::SUCCESS;

	pugi::xml_document doc;
	if (fs::exists("config.xml")) {
		pugi::xml_parse_result result = doc.load_file("config.xml");

		if (result)
			cout << "xml load" << endl;
		else
			cout << "xml not load" << endl;
	}
	else {
		auto declarationNode = doc.append_child(pugi::node_declaration);
		declarationNode.append_attribute("version") = WATCHER_VERSION;
		declarationNode.append_attribute("encoding") = "UTF-8";

		auto root = doc.append_child("root");

		bool saveSucceeded = doc.save_file("config.xml", PUGIXML_TEXT("  "));
		cout << "saveSucceeded: " << saveSucceeded << endl;
	}

	sqlite3* db;
	auto rc = sqlite3_open("db.sqlite3", &db);
	cout << "Open db: " << (rc ? "FAIL" : "SUCCESS");
	sqlite3_close(db);

	io_service service;
	tcp::socket socket(service);
	tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), 32094));
	cout << "socket created" << endl;

	return base::exitCodes::SUCCESS;
}

int Application::exec() {
	auto rc = startWatcher();
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
	if (profileName.empty())
		profileName = "profile-default";

	return base::exitCodes::SUCCESS;
}

bool Application::parseFirstArg(char*& arg) {
	appDir = arg;
	appDir.erase(nameFromFullName(appDir, appName));

	if (appDir.empty() || appName.empty())
		return false;
	return true;
}

