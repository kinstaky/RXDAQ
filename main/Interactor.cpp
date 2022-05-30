#include "lib/args.hxx"
#include "Interactor.h"
#include "defs.h"
#include "Error.h"

typedef dstring::Level dlv;

namespace RXDAQ {

//--------------------------------------------------
//					Interactor
//--------------------------------------------------

// constructor
Interactor::Interactor() {
}


// deconstructor
Interactor::~Interactor() {
}



// set config file
void Interactor::SetConfigFile(const char *path) {
	if (path) configFileName = path;
	else configFileName = "";
}


// get config file
const char *Interactor::ConfigFile() {
	if (configFileName.size()) return configFileName.c_str();
	else return nullptr;
}


dlv Interactor::GetOutputLevel() const {
	return dlv::Error;
}


//--------------------------------------------------
//					Parser
//--------------------------------------------------

// constructor
Parser::Parser(int argc, char **argv): Interactor() {
	boot = false;
	read = false;
	write = false;
	rpc = false;
	run = false;
	crate = -1;
	module = -1;
	channel = -1;
	list = false;
	parse(argc, argv);
}


// deconstructor
Parser::~Parser() {
}


// pick this part from the constructor to be comfortable
void Parser::parse(int argc, char **argv) {
	args::ArgumentParser parser("This is a test program for RXDAQ.", "");						// parser
	args::HelpFlag help(parser, "help", "Display this help infomation.", {'h', "help"});		// help

	// crate id
	args::ValueFlag<unsigned short> optCrate(parser, "id", "Set the crate id of this program, default is 0", {"crate", 'c'}, 0);
	// config file
	args::ValueFlag<std::string> configFile(parser, "file", "Set the path of the config file", {"config"});


	// output level
	args::ValueFlag<std::string> outLevel(parser, "level", "Set the Output levels: error, warning, info, debug, default is error", {"level", 'l'}, "error");

	// // completion flag
	// args::CompletionFlag completion(parser, {"complete"});

	// commands
	args::Group commands(parser, "Commands, at most one.", args::Group::Validators::AtMostOne);
	// args::Command cmdBoot(commands, "boot", "boot modules", [&](args::Subparser &parser) {
	// 	cmdBootFunc(parser);
	// });
	// args::Command cmdRead(commands, "read", "read parameters", [&](args::Subparser &parser) {
	// 	cmdReadWriteFunc(parser, true);
	// });
	// args::Command cmdWrite(commands, "write", "write parameters", [&](args::Subparser &parser) {
	// 	cmdReadWriteFunc(parser, false);
	// });
	args::Command cmdBoot(commands, "boot", "Boot modules.");
	args::Command cmdRead(commands, "read", "Read parameters from firmware.");
	args::Command cmdWrite(commands, "write", "Write parameters to firmware.");
	args::Command cmdRPC(commands, "rpc", "Launch the gRPC server.");
	args::Command cmdRun(commands, "run", "Start list-mode run.");
	args::Command cmdExport(commands, "export", "Export parameters to json file.");
	args::Command cmdImport(commands, "import", "Import parameters from json file.");
	args::Command cmdInfo(commands, "info", "Display moduels firware information.");

	// options of commands
	args::ValueFlag<unsigned short> optModule(cmdBoot, "module", "The module, default is all.", {'m', "module"});
	cmdRead.Add(optModule);
	cmdWrite.Add(optModule);
	cmdRun.Add(optModule);

	args::ValueFlag<unsigned short> optChannel(cmdRead, "channel", "The channel, default is all.", {"ch", "channel"}, 16);
	cmdWrite.Add(optChannel);

	args::Flag optList(cmdRead, "list", "List the parameters.", {'l', "list"});
	cmdWrite.Add(optList);

	args::ValueFlag<std::string> optName(cmdRead, "name", "Choose the parameter to read/write.", {'n', "name"});
	cmdWrite.Add(optName);

	args::ValueFlag<std::string> optValue(cmdWrite, "value", "Write the parameter value.", {'v', "value"}, args::Options::Required);

	args::ValueFlag<int> optRunTime(cmdRun, "time", "Set run time in seconds.", {'t', "time"}, args::Options::Required);

	args::ValueFlag<std::string> optPath(cmdExport, "file", "Set the path of the parameters file.", {'p', "path"}, args::Options::Required);
	cmdImport.Add(optPath);

	args::ValueFlag<std::string> optRunConfig(cmdRun, "file", "Set the run config file.", {'p', "path"});
	args::ValueFlag<unsigned int> optRun(cmdRun, "number", "Set the run number.", {'r', "run"});
	args::ValueFlag<std::string> optDataPath(cmdRun, "path", "Set the run data path.", {'d', "data"});
	args::ValueFlag<std::string> optDataFile(cmdRun, "name", "Set then run data file name.", {'f', "file"});

	try {
		parser.ParseCLI(argc, argv);
	} catch (const args::Help &h) {
		std::cout << parser;
		exit(0);
	} catch (const args::ParseError &e) {
		std::cout << "\033[1;31mERROR: \033[0m" << e.what() << std::endl;
		std::cout << parser;
		exit(-1);
	} catch (const args::ValidationError &e) {
		std::cout << "\033[1;31mERROR: \033[0m" << e.what() << std::endl;
		std::cout << parser;
		exit(-1);
	}

	if (configFile) {
		SetConfigFile(configFile.Get().c_str());
	}
	if (outLevel) {
		if (outLevel.Get() == "error") {
			level = dlv::Error;
		} else if (outLevel.Get() == "warning") {
			level = dlv::Warning;
		} else if (outLevel.Get() == "info") {
			level = dlv::Info;
		} else if (outLevel.Get() == "debug") {
			level = dlv::Debug;
		} else {
			std::cout << "\033[1;31mERROR: \033[0mInvalid output level: " << outLevel.Get() << std::endl;
			std::cout << "Output level should be one of error, warning, info and debug." << std::endl;
			exit(-2);
		}
	} else {
		level = dlv::Error;
	}


	boot = cmdBoot;
	read = cmdRead;
	write = cmdWrite;
	rpc = cmdRPC;
	run = cmdRun;
	exportParam = cmdExport;
	importParam = cmdImport;
	info = cmdInfo;

	crate = optCrate ? optCrate.Get() : 0;
	module = optModule ? optModule.Get() : AllModules;
	channel = optChannel ? optChannel.Get() : AllChannels;
	list = optList;
	name = optName.Get();
	value = optValue.Get();
	time = optRunTime.Get();
	path = optRunConfig ? optRunConfig.Get() : optPath.Get();
	runConfig = optRunConfig;
	runNumber = optRun ? optRun.Get() : 0;
	runDataPath = optDataPath ? optDataPath.Get() : "./";
	runDataFile = optDataFile ? optDataFile.Get() : "";

	// args manual check
	if (boot || read || write || run || exportParam || importParam || info) {
		if (!configFile) {
			std::cout << "\033[1;31mERROR: \033[0mRequire config file (--config) in this command mode." << std::endl;
			exit(-2);
		}
		if (!optCrate) {
			std::cout << "\033[1;31mERROR: \033[0mRequire crate id (--crate or -c) in this command mode." << std::endl;
			exit(-2);
		}
	}
	if (read || write) {
		if (!list && name=="") {
			std::cout << "\033[1;31mERROR: \033[0mRequire option --list(-l) or --name(-n) in this command mode." << std::endl;
			exit(-2);
		}
	}
}


// whether use RPC as the Interactor
bool Parser::RPC() {
	return rpc;
}

dlv Parser::GetOutputLevel() const {
	return level;
}

unsigned short Parser::CrateID() const {
	return crate;
}

// The Parser run the commands from the shell and call the method in Crate.
void Parser::Run(Crate *crate) {
	try {
		crate->Init();
		if (!boot) {
			crate->Boot(module, 0x4);
		}

		if (boot) {
			crate->Boot(module, 0xf);
		} else if (list) {
			std::cout << crate->ListParameters();
		} else if (read) {
			int paramType = crate->CheckParameterName(name);
			if (paramType == 0) {
				throw std::runtime_error("Invlid parameter " + name + ".\n");
			} else if (paramType == TypeModuleParameter) {

				// prepare for index
				std::vector<unsigned short> modules;
				if (module == AllModules) {
					int moduleNum = crate->ModuleNum();
					for (unsigned short m = 0; m != moduleNum; ++m) {
						modules.push_back(m);
					}
				} else {
					modules.push_back(module);
				}

				// read from crate
				unsigned int par;
				std::vector<unsigned int> pars;
				for (auto &m : modules) {
					crate->ReadModuleParameter(name, &par, m);
					pars.push_back(par);
				}
				// output parameters
				std::cout << "module   " << name << std::endl;		// title
				for (auto &m : modules) {
					std::cout << std::setw(6) << m;
					std::cout << std::setw(name.size()+3) << pars[m];
					std::cout << std::endl;
				}


			} else if (paramType == TypeChannelParameter) {

				// prepare for index
				std::vector<unsigned short> modules;
				std::vector<unsigned short> channels;
				if (module == AllModules) {
					int moduleNum = crate->ModuleNum();
					for (unsigned short m = 0; m != moduleNum; ++m) {
						modules.push_back(m);
					}
				} else {
					modules.push_back(module);
				}
				if (channel == AllChannels) {
					for (unsigned short c = 0; c != 16; ++c) {
						channels.push_back(c);
					}
				} else {
					channels.push_back(channel);
				}

				// read from crate
				double par;
				std::vector<double> pars;
				for (auto &m : modules) {
					for (auto &c : channels) {
						crate->ReadChannelParameter(name, &par, m, c);
						pars.push_back(par);
					}
				}

				// output parameters
				std::cout << std::setw(14) << "channel" << std::endl;
				std::cout << "module";
				for (auto & c : channels) {
					std::cout << std::setw(8) << c;
				}
				std::cout << std::endl;						// title

				auto iter = pars.begin();
				for (auto &m : modules) {
					std::cout << std::setw(6) << m;
					for (unsigned int c = 0; c != channels.size(); ++c) {
						std::cout << std::setw(8) << *iter;
						++iter;
					}
					std::cout << std::endl;
				}

			} else {
				throw std::runtime_error("Unknown return value from Crate::CheckParameterName.\n");
			}

		} else if (write) {
			int paramType = crate->CheckParameterName(name);
			if (paramType == 0) {
				throw std::runtime_error("Invlid parameter " + name + ".\n");
			} else if (paramType == TypeModuleParameter) {

				// prepare for index
				std::vector<unsigned short> modules;
				if (module == AllModules) {
					int moduleNum = crate->ModuleNum();
					for (unsigned short m = 0; m != moduleNum; ++m) {
						modules.push_back(m);
					}
				} else {
					modules.push_back(module);
				}

				// write to crate
				unsigned int val = static_cast<unsigned int>(stoi(value, nullptr, 0));
				for (auto &m : modules) {
					crate->WriteModuleParameter(name, val, m);
				}

			} else if (paramType == TypeChannelParameter) {

				// prepare for index
				std::vector<unsigned short> modules;
				std::vector<unsigned short> channels;
				if (module == AllModules) {
					int moduleNum = crate->ModuleNum();
					for (unsigned short m = 0; m != moduleNum; ++m) {
						modules.push_back(m);
					}
				} else {
					modules.push_back(module);
				}
				if (channel == AllChannels) {
					for (unsigned short c = 0; c != 16; ++c) {
						channels.push_back(c);
					}
				} else {
					channels.push_back(channel);
				}

				// write to crate
				double val = stod(value);
				for (auto &m : modules) {
					for (auto &c : channels) {
						crate->WriteChannelParameter(name, val, m, c);
					}
				}

			} else {
				throw std::runtime_error("Unknown return value from Crate::CheckParameterName.\n");
			}

		} else if (run) {
			if (runConfig) crate->SetRunConfig(path);
			else crate->SetRunConfig(runDataPath, runDataFile, runNumber);
			crate->Run(module, time);
		} else if (info) {
			crate->PrintInfo();
		} else if (exportParam) {
			crate->ExportParameters(path);
		} else if (importParam) {
			crate->ImportParameters(path);
		}

	} catch (const UserError &e) {						// user operation error
		std::cout << ds(dlv::Warning, e.what());
	} catch (const xia::pixie::error::error &e) {						// fatal xia error
		std::cout << ds(dlv::Error, std::string("Error from xia pixie sdk: ") + e.result_text() + ".\n");
	} catch (const RXError &e) {						// fatal RXDAQ error
		std::cout << ds(dlv::Error, std::string("RXDAQ Error, ") + e.what());
	} catch (const std::exception &e) {					// fatal error
		std::cout << ds(dlv::Error, e.what());
	}
	return;
}




}			// namespace RXDAQ