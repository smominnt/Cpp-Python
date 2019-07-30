#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include "h1.h"
using namespace std;
list <string> programLines;


void loadfile(ifstream& myfile) {
	string line;
	while (getline(myfile, line)) { //load lines into list
		programLines.push_back(line);
	}
	programLines.push_back("#EOF");
	passPrg(programLines);
}


void openMyFile(int argc, char **argv) {
	//read file
	ifstream myFile;
	string filename = argv[1];
	myFile.open(filename);

	if (!myFile) {
		cout << "File not found\n";
		exit(1);
	}
	else {
		loadfile(myFile);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		cout << "Syntax ./Source <file_name>\n";
		exit(1);
	}
	openMyFile(argc, argv);
    return 0;
}


