/*
*/

#include <iostream>

#include "application.h"

using std::move;

int main(int argc, char* argv[]) {
	Application app;
	if (auto rc = app.load(move(argc), move(argv))) return rc;

	return app.exec();
}