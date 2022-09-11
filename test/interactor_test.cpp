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


struct HelpData {
	string input;
	string command;
};
// input commands and options
vector<HelpData> kHelpCommands = {
	{"help", "help"},
	{"help help", "help"},
	{"help boot", "help"},
	{"help read", "help"},
	{"help write", "help"}
};
struct BootData {
	string input;
	unsigned short module;
};
vector<BootData> kBootCommands = {
	{"boot", 13},
	{"boot 13", 13},
	{"boot 4", 4},
	{"boot -m 2", 2},
	{"boot --module 10", 10},
	{"boot -m 13", 13},
	{"boot -m 0 2", 0}
};
struct WriteData {
	string input;
	unsigned short module;
	unsigned short channel;
	string name;
	ParameterType type;
	unsigned int module_value;
	double channel_value;
};
vector<WriteData> kWriteCommands = {
	// list parameters
	{"write", 0, 0, "", ParameterType::kAll, 0, 0.0},
	{"write --list", 0, 0, "", ParameterType::kAll, 0, 0.0},
	{"write -l MODULE_CSRA 20", 0, 0, "", ParameterType::kAll, 0, 0.0},
	// write module parameters
	{"write MODULE_CSRA 20", 13, 0, "MODULE_CSRA", ParameterType::kModule, 20, 0.0},
	{"write MODULE_CSRA 20 2", 2, 0, "MODULE_CSRA", ParameterType::kModule, 20, 0.0},
	{"write -m 3 MODULE_CSRB 20", 3, 0, "MODULE_CSRB", ParameterType::kModule, 20, 0.0},
	{"write --module 5 -v 4 MODULE_CSRB 10 7", 5, 0, "MODULE_CSRB", ParameterType::kModule, 4, 0.0},
	{"write -m 8 --value 9 TrigConfig0", 8, 0, "TrigConfig0", ParameterType::kModule, 9, 0.0},
	{"write --name TrigConfig1 --value 28 --module 1", 1, 0, "TrigConfig1", ParameterType::kModule, 28, 0.0},
	{"write -n TrigConfig0 -m 10 -v 2 TrigConfig0 3 4", 10, 0, "TrigConfig0", ParameterType::kModule, 2, 0.0},
	// write channel parameters
	{"write ENERGY_FLATTOP 2.3", 13, 16, "ENERGY_FLATTOP", ParameterType::kChannel, 0, 2.3},
	{"write ENERGY_FLATTOP 1.9 10", 10, 16, "ENERGY_FLATTOP", ParameterType::kChannel, 0, 1.9},
	{"write TAU 25 0 8", 0, 8, "TAU", ParameterType::kChannel, 0, 25.0},
	{"write -m 6 TAU 40 4 12", 6, 12, "TAU", ParameterType::kChannel, 0, 40.0},
	{"write -c 10 TAU 20", 13, 10, "TAU", ParameterType::kChannel, 0, 20.0},
	{"write --module 4 --channel 2 TRACE_DELAY 10.0", 4, 2, "TRACE_DELAY", ParameterType::kChannel, 0, 10.0},
	{"write -c 16 TRACE_DELAY 20.0 12", 12, 16, "TRACE_DELAY", ParameterType::kChannel, 0, 20.0},
	{"write -v 27.8 TAU", 13, 16, "TAU", ParameterType::kChannel, 0, 27.8},
	{"write --value 16.0 --name BLCUT --module 0", 0, 16, "BLCUT", ParameterType::kChannel, 0, 16.0},
	{"write -n BLCUT -v 3.0 -m 8 -c 16 TAU 20.0 10 3", 8, 16, "BLCUT", ParameterType::kChannel, 0, 3.0}
};
struct ImportExportData {
	string input;
	string command;
	string path;
	TestCrate::ModuleStatus status;
};
vector<ImportExportData> kImportExportCommands = {
	{"import params.json", "import", "params.json", TestCrate::ModuleStatus::kImported},
	{"export params.json", "export", "params.json", TestCrate::ModuleStatus::kExported}
};


const size_t kArgSize = 32;
const size_t kArgMaxLength = 32;
void SeperateArguments(const string &args, int &argc, char **argv) {
	argc = 1;
	strcpy(argv[0], "test");
	char str[kArgSize * kArgMaxLength];
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
	for (size_t i = 0; i < kArgSize; ++i) {
		delete[] argv[i];
	}
	delete[] argv;
}
 

TEST(InteractorTest, HandleError) {
	int argc;
	char **argv;
	argv = new char*[kArgSize];
	for (size_t i = 0; i < kArgSize; ++i) {
		argv[i] = new char[kArgMaxLength];
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
	argv = new char*[kArgSize];
	for (size_t i = 0; i < kArgSize; ++i) {
		argv[i] = new char[kArgMaxLength];
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
	argv = new char*[kArgSize];
	for (size_t i = 0; i < kArgSize; ++i) {
		argv[i] = new char[kArgMaxLength];
	}

	for (size_t i = 0; i < kBootCommands.size(); ++i) {
		std::cout << "case " << i << std::endl;

		Parser parser;
		auto crate = std::make_shared<TestCrate>();

		SeperateArguments(kBootCommands[i].input, argc, argv);
		auto interactor = parser.Parse(argc, argv);

		EXPECT_EQ(interactor->CommandName(), std::string("boot"))
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

	FreeArgs(argv);
}


TEST(InteractorTest, ParseWriteCommand) {
	int argc;
	char **argv;
	argv = new char*[kArgSize];
	for (size_t i = 0; i < kArgSize; ++i) {
		argv[i] = new char[kArgMaxLength];
	}

	for (size_t i = 0; i < kWriteCommands.size(); ++i) {
		std::cout << "case " << i << std::endl;

		Parser parser;
		auto crate = std::make_shared<TestCrate>();

		SeperateArguments(kWriteCommands[i].input, argc, argv);
		auto interactor = parser.Parse(argc, argv);

		EXPECT_EQ(interactor->CommandName(), "write")
			<< "Error: command name at " << i;


		EXPECT_NO_THROW(interactor->Run(crate));

		if (kWriteCommands[i].name.empty()) {
			EXPECT_TRUE(crate->list_)
				<< "Error: module didn't list " << i;
		} else {
			EXPECT_EQ(
				kWriteCommands[i].type,
				crate->CheckParameter(kWriteCommands[i].name)
			)
				<< "Error: parameter type " << i;


			std::vector<unsigned short> modules;
			if (kWriteCommands[i].module == kModuleNum) {
				for (unsigned short j = 0; j < crate->ModuleNum(); ++j) {
					modules.push_back(j);
				}
			} else {
				modules.push_back(kWriteCommands[i].module);
			}

			if (kWriteCommands[i].type == ParameterType::kModule) {
				for (const auto &m : modules) {
					auto &parameters = crate->modules_[m].module_parameters;
					auto search = parameters.find(kWriteCommands[i].name);

					EXPECT_NE(search, parameters.end())
						<< "Error: module parameter not found case "
						<< i << " module " << m;

					EXPECT_EQ(search->second, kWriteCommands[i].module_value)
						<< "Error: module parameter value case "
						<< i << " module " << m;						
				}
			
			} else if (kWriteCommands[i].type == ParameterType::kChannel) {
				std::vector<unsigned short> channels;
				if (kWriteCommands[i].channel == kChannelNum) {
					for (unsigned short j = 0; j < kChannelNum; ++j) {
						channels.push_back(j);
					}
				} else {
					channels.push_back(kWriteCommands[i].channel);
				}

				for (const auto &m : modules) {
					for (const auto &c : channels) {
						auto &parameters = crate->modules_[m].channel_parameters[c];
						auto search = parameters.find(kWriteCommands[i].name);

						EXPECT_NE(search, parameters.end())
							<< "Error: channel parameter not found case "
							<< i << " module " << m << " channel " << c;

						EXPECT_EQ(search->second, kWriteCommands[i].channel_value)
							<< "Error: channel parameter value case "
							<< i << " module " << m << " channel " << c;
					}
				}
			} else {
				FAIL() << "Error: invalid parameter type of case "
					<< i << " " << kWriteCommands[i].input;
			}
		}
	}

	FreeArgs(argv);
}



TEST(InteractorTest, ImportExportCommand) {
	int argc;
	char **argv;
	argv = new char*[kArgSize];
	for (size_t i = 0; i < kArgSize; ++i) {
		argv[i] = new char[kArgMaxLength];
	}

	for (size_t i = 0; i < kImportExportCommands.size(); ++i) {
		std::cout << "case " << i << std::endl;

		Parser parser;
		auto crate = std::make_shared<TestCrate>();

		SeperateArguments(kImportExportCommands[i].input, argc, argv);
		auto interactor = parser.Parse(argc, argv);

		EXPECT_EQ(interactor->CommandName(), kImportExportCommands[i].command)
			<< "Error: command name at " << i;

		EXPECT_NO_THROW(interactor->Run(crate));

		for (size_t m = 0; m < crate->ModuleNum(); ++m) {
			EXPECT_EQ(crate->modules_[m].config_file, kImportExportCommands[i].path)
				<< "Error: path at " << i << " module " << m;

			EXPECT_EQ(crate->modules_[m].status, kImportExportCommands[i].status)
				<< "Error: status at " << i << " module " << m;
		}
	}

	FreeArgs(argv);
}