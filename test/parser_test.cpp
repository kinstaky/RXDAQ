#include <cstring>

#include <vector>
#include <string>
#include <sstream>
#include <map>

#include <gtest/gtest.h>

#include "include/parser.h"
#include "include/error.h"

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


struct CommandData {
	string input;
	Interactor::InteractorType type;
};
// input commands and options
vector<CommandData> kCommandArgs = {
	{"help", Interactor::InteractorType::kHelpCommandParser},
	{"boot", Interactor::InteractorType::kBootCommandParser}
	// {"info", Interactor::InteractorType::kInfoCommandParser}
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
		
		EXPECT_THROW(parser.Parse(argc, argv), UserError);
	}
	
	// free memory
	for (size_t i = 0; i < arg_size; ++i) {
		delete[] argv[i];
	}
	delete[] argv;
}


TEST(ParserTest, ParseCommand) {
	// prepare parameters to parse
	int argc;
	char **argv;
	const size_t arg_size = 4;
	argv = new char*[arg_size];
	for (size_t i = 0; i < arg_size; ++i) {
		argv[i] = new char[32];
	}

	for (size_t i = 0; i < kCommandArgs.size(); ++i) {
		Parser parser;

		SeperateArguments(kCommandArgs[i].input, argc, argv);
		
		auto interactor = parser.Parse(argc, argv);
		EXPECT_EQ(kCommandArgs[i].type, interactor->Type())
			<< "Error: type error in case " << i;
	}
	
	// free memory
	for (size_t i = 0; i < arg_size; ++i) {
		delete[] argv[i];
	}
	delete[] argv;
}