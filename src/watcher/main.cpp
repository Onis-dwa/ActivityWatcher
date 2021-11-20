/*
*/

#include <iostream>
#include "watcherApp.h"

int main(int argc, char* argv[]) {
	auto app = watcherApp(std::move(argc), std::move(argv));


	return 0;
}