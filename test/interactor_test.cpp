#include <cstring>

#include <vector>
#include <string>
#include <sstream>

#include <gtest/gtest.h>

#include "include/crate.h"
#include "include/error.h"
#include "include/interactor.h"
#include "include/parser.h"
#include "test/test_crate.h"

using namespace std;
using namespace rxdaq;


// error inputs
vector<string> kErrorArgs = {
	"help help help",
	"help boot help",
	"help nothing",
	"boot 20",
	"boot 0 2"
};


struct CommandData {
	string input;
	string command;
	unsigned short module;
	unsigned short channel;
};
// input commands and options
vector<CommandData> kHelpCommands = {
	{"help", "help", 0, 0},
	{"help help", "help", 0, 0},
	{"help boot", "help", 0, 0}
};
vector<CommandData> kBootCommands = {
	{"boot", "boot", 13, 0},
	{"boot 13", "boot", 13, 0},
	{"boot 4", "boot", 4, 0},
	{"boot -m 2", "boot", 2, 0},
	{"boot --module 10", "boot", 10, 0},
	{"boot -m 13", "boot", 13, 0},
	{"boot -m 0 2", "boot", 0, 0}
};

const size_t arg_size = 8;
void SeperateArguments(const string &args, int &argc, char **argv) {
	argc = 1;
	strcpy(argv[0], "test");
	char str[256];
	strcpy(str, args.c_str());
	char *token = strtok(str, " ");
	while (token) {
		strcpy(argv[argc], token);
		++argc;
		token = strtok(nullptr, " ");
	}
	return;
}
void FreeArgs(char **argv) {
	for (size_t i = 0; i < arg_size; ++i) {
		delete[] argv[i];
	}
	delete[] argv;
}
 

TEST(InteractorTest, HandleError) {
	int argc;
	char **argv;
	argv = new char*[arg_size];
	for (size_t i = 0; i < arg_size; ++i) {
		argv[i] = new char[32];
	}

	for (size_t i = 0; i < kErrorArgs.size(); ++i) {
		Parser parser;

		SeperateArguments(kErrorArgs[i], argc, argv);
		
		EXPECT_THROW(parser.Parse(argc, argv), UserError)
			<< "Error: not throw or unexpected throw type at " << i;
	}
		
	FreeArgs(argv);
}


TEST(InteractorTest, ParseHelpCommand) {
	int argc;
	char **argv;
	argv = new char*[arg_size];
	for (size_t i = 0; i < arg_size; ++i) {
		argv[i] = new char[32];
	}

	for (size_t i = 0; i < kHelpCommands.size(); ++i) {
		Parser parser;

		SeperateArguments(kHelpCommands[i].input, argc, argv);
		auto interactor = parser.Parse(argc, argv);
		
		EXPECT_EQ(interactor->CommandName(), kHelpCommands[i].command)
			<< "Error: command name at " << i;
	
		EXPECT_NO_THROW(interactor->Run());
	}
		
	FreeArgs(argv);
}


TEST(InteractorTest, ParseBootCommand) {
	int argc;
	char **argv;
	argv = new char*[arg_size];
	for (size_t i = 0; i < arg_size; ++i) {
		argv[i] = new char[32];
	}

	for (size_t i = 0; i < kBootCommands.size(); ++i) {
		std::cout << "case " << i << std::endl;

		Parser parser;
		auto crate = std::make_shared<TestCrate>();

		SeperateArguments(kBootCommands[i].input, argc, argv);
		auto interactor = parser.Parse(argc, argv);

		EXPECT_EQ(interactor->CommandName(), kBootCommands[i].command)
			<< "Error: command name at " << i;


		EXPECT_NO_THROW(interactor->Run(crate));
		
		if (kBootCommands[i].module == kModuleNum) {
			for (unsigned short j = 0; j < kModuleNum; ++j) {
				EXPECT_EQ(crate->modules_[j].status, TestCrate::ModuleStatus::kBooted)
					<< "Error: module didn't boot " << j;
				EXPECT_EQ(crate->modules_[j].boot_pattern, kEntireBoot)
					<< "Error: module didn't boot entirely " << j;
			}
		} else {
			unsigned short index = kBootCommands[i].module;
			EXPECT_EQ(crate->modules_[index].status, TestCrate::ModuleStatus::kBooted)
				<< "Error: module didn't boot " << index;
			EXPECT_EQ(crate->modules_[index].boot_pattern, kEntireBoot)
				<< "Error: module didn't boot entirely " << index;
		}
	}
}