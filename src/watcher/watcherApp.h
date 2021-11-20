/*
*/

#include <iostream>
using namespace std;

class watcherApp {
public:
	explicit watcherApp(int&& argc, char**&& argv) {

		cout << argc << endl;
		for (int i = 0; i < argc; ++i)
			cout << "\t" << argv[i] << endl;
	}

};
