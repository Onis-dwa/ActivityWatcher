/*
*/
#pragma once

#include <string>
#include <boost/utility.hpp>

using std::string;

class Application final : boost::noncopyable {
public:
	Application() {};
	int load(int&& argc, char**&& argv);

	int exec();

private:
	int processArguments(int&& argc, char**&& argv);
	bool parseFirstArg(char*& arg);

	string appName;
	string appDir;
	
	string dataDir;
	string profileName;
};
