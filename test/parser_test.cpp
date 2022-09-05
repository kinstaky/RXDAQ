#include <cstring>

#include <vector>
#include <string>
#include <sstream>
#include <map>

#include <gtest/gtest.h>

#include "include/parser.h"

using namespace std;
using namespace rxdaq;

// error inputs
vector<string> kErrorArgs = {
	"",
	"hello",
	"c",
	"-h",
	"-c",
	"-c -h"
};

// input commands and options
vector<string> kCommandArgs = {
	"help",
	"read -l",
	"write",
	"info"
};

void SeperateArguments(const string &args, int &argc, char **argv) {
	argc = 1;
	strcpy(argv[0], "test");
	char str[128];
	strcpy(str, args.c_str());
	char *token = strtok(str, " ");
	while (token) {
		strcpy(argv[argc], token);
		++argc;
		token = strtok(nullptr, " ");
	}
	std::cout << "seperate `" << args << "` into " << argc << " parts" << std::endl;
	for (int i = 0; i < argc; ++i) {
		std::cout << argv[i] << " ";
	}
	cout << endl;
	return;
}

TEST(ParserTest, ErrorArguments) {
	Parser parser;
	// prepare parameters to parse
	int argc;
	char **argv;
	const size_t arg_size = 4;
	argv = new char*[arg_size];
	for (size_t i = 0; i < arg_size; ++i) {
		argv[i] = new char[32];
	}

	for (size_t i = 0; i < kErrorArgs.size(); ++i) {
		SeperateArguments(kErrorArgs[i], argc, argv);
		
		for (int i = 0; i < argc; ++i) {
			std::cout << argv[i] << std::endl;
		}
		
		EXPECT_THROW(parser.Parse(argc, argv), std::runtime_error);
		
	}
	
	// free memory
	for (size_t i = 0; i < arg_size; ++i) {
		delete[] argv[i];
	}
	delete[] argv;
}