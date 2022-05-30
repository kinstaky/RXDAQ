#include "main/Manager.h"
#include "lib/pixie_sdk/include/pixie16/pixie16.h"
#include "main/dstring.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

typedef dstring::Level dlevel;


std::ostream & operator<<(std::ostream &os, const Task &task) {
	// print type
	switch (task.Type) {
		case Task::TType::Init:
			os << "Init ";
			break;
		case Task::TType::Read:
			os << "Read ";
			break;
		case Task::TType::Write:
			os << "Write";
			break;
		case Task::TType::Boot:
			os << "Boot ";
			break;
		default:
			os << "Error";
	}
	// // print crate
	// if (task.Crate >= 0) os << std::setw(7) << task.Crate;
	// else os << "    all";
	// print module
	if (task.Module >= 0) os << std::setw(8) << task.Module;
	else os << "     all";
	// print channel
	if (task.Channel >= 0) os << std::setw(10) << task.Channel;
	else os << "       all";
	// print list
	os << (task.List ? "      T" : "      F");
	// print name
	os << "   " << task.Name;
	return os;
}


// cmdReadWriteFunc(Subparser &parser, bool read)
// implememtation of read and write command
// arguments:
//		parser -- args parser
//		read   -- ture for read and false for write
void Manager::cmdReadWriteFunc(args::Subparser &parser, bool read) {
	args::HelpFlag help(parser, "help", "display this read/write help menu", {'h', "help"});
	// args::ValueFlag<unsigned int> crate(parser, "id", "the crate id", {'C', "crate"});
	args::ValueFlag<int> module(parser, "id", "the module id", {'m', "module"});
	args::ValueFlag<int> channel(parser, "ch", "channel, require module id", {'c', "channel"});
	// list or name is necassary
	args::Group group(parser, "exclusive group of name or list", args::Group::Validators::Xor);
	args::ValueFlag<std::string> parameter(group, "name", "name of parameter", {'n', "name"});
	args::Flag list(group, "list", "list all parameters", {'l', "list"});

	parser.Parse();

	// check names
	if (!list) {
		if (!parameters.checkName(parameter.Get(), read)) {
			throw std::runtime_error("parameter \"" + parameter.Get() + "\" isn't available. Use --list to see the available parameters.\n");
		}
	}


	Task task;
	task.Type = read ? Task::TType::Read : Task::TType::Write;
	// task.Crate = crate ? crate.Get() : -1;
	task.Module = module ? module.Get() : -1;
	task.Channel = channel && task.Module!=-1 ? channel.Get() : -1;
	task.List = list;
	task.Name = parameter.Get();
	tasks.insert(std::make_pair(read ? Task::TPriority::Read : Task::TPriority::Write, task));

	return;
}



Manager::Manager() {
	ds = dstring();
	ds.SetColorPrefix();
}



Manager::~Manager() {

}


int Manager::PrintTasks() const {
	std::cout << ds(dlevel::Debug, "Manager::PrintTasks\n");
	std::stringstream ss;
	// print task list
	ss << "--------------------Tasks list--------------------" << std::endl;
	// std::cout << dstring(dType::Verbose, "Tasks list") << std::endl;
	ss << "Type   Module   Channel   List   Name" << std::endl;
	for (auto t : tasks) {
		ss << t.second << std::endl;
	}

	std::cout << ds(dlevel::Info, ss.str());

	return 0;
}


int Manager::PrintCrateInfo() const {
	std::cout << ds(dlevel::Debug, "Manager::PrintCrateInfo\n");
	std::cout << ds(dlevel::Info, config.CrateInfoStr());
	return 0;
}




int Manager::RunManager() {
	std::cout << ds(dlevel::Debug, "Manager::RunManager\n");
	PrintTasks();
	PrintCrateInfo();

	// run tasks
	for (auto &[key, t] : tasks) {
		if (t.Type == Task::TType::Init) {
			// init system
			if (Pixie16InitSystem(config.ModuleNum(), config.PXISlotMap(), 0)) {
				std::cout << ds(dlevel::Error, "Pixie16InitSystem Failed.\n");
				return -1;
			}
			// check the moduels before runing other tasks
			if (checkModuleInfo()) return -2;
		} else if (t.Type == Task::TType::Boot) {
			// boot modules
			if (t.Module == -1) {			// boot all modules in crate
				std::cout << ds(dlevel::Debug, "Boot all modules.\n");
				for (unsigned short i = 0; i != config.ModuleNum(); ++i) {
					if (checkBootRet(Pixie16BootModule(config.Sys(i).c_str(), config.Fippi(i).c_str(),
													   config.Trig(i).c_str(), config.Ldr(i).c_str(),
													   config.Par(i).c_str(), config.Var(i).c_str(),
													   i, 0x7f)))
						return -3;
				}
			} else {						// boot only one module
				std::cout << ds(dlevel::Debug, "Boot module " + std::to_string(t.Module) + ".\n");
				if (checkBootRet(Pixie16BootModule(config.Sys(t.Module).c_str(), config.Fippi(t.Module).c_str(),
												   config.Trig(t.Module).c_str(), config.Ldr(t.Module).c_str(),
												   config.Par(t.Module).c_str(), config.Var(t.Module).c_str(),
												   t.Module, 0x7f)))
					return -3;
			}
		} else if (t.Type == Task::TType::Read) {
			if (t.List) {
std::cout << "\033[1;31mNOT FINISHED\033[0m print readable parameters list" << std::endl;
				return 0;
			}
			// read values
			unsigned int *modPar;
			double *chanPar;
			// read module parameter
			if (parameters.InModule(t.Name)) {
				// read all modules
				if (t.Module == -1) {
					std::cout << ds(dlevel::Debug, "Read module parameter " + t.Name + " from all modules.\n");
					modPar = new unsigned int[config.ModuleNum()];
					std::vector<unsigned short> readModules(config.ModuleNum());
					for (unsigned short i = 0; i != config.ModuleNum(); i++) {
						readModuels[i] = i;
					}

std::cout << "\033[1;31mSHOULD BE COMMENT\033[0m read from modules";
for (auto iter : readModules) std::cout << iter << "  ";
std::cout << std::endl;

					// read parameters from firmware
					try {
						parameters.ModuleRead(t.Name, modPar, readModules);
					} catch (const std::exception &e) {
						std::cout << ds(dlevel::Error, e.what());
						return -4;
					}
					// print them
					std::stringstream ss;
					ss << "--------------------Read Parameters--------------------" << std::endl;
					ss << "Module   " << t.Name << std::endl;
					for (int i = 0; i != config.ModuleNum(); ++i) {
						ss << std::setw(6) << i;				// module number
						ss << std::setw(3+t.Name.size()) << std::hex << modPar[i];
						ss << std::endl;
					}
					std::cout << ss.str();

				} else {								// read just one module
					std::cout << ds(dlevel::Debug, "Read module parameter \"" + t.Name + "\" from module " + std::to_string(t.Module) + ".\n");
					modPar = new unsigned int;
					// read parameters from firmware
					try {
						parameters.ModuleRead(t.Name, modPar, t.Module);
					} catch (const std::exception &e) {
						std::cout << ds(dlevel::Error, e.what());
						return -4;
					}
					// print it
					td::stringstream ss;
					ss << "--------------------Read Parameters--------------------" << std::endl;
					ss << "Module   " << t.Name << std::endl;
					ss << std::setw(6) << t.Module;
					ss << std::setw(3+t.Name.size()) << std::hex << *modPar;
					ss << std::endl;
					std::cout << ss.str();
				}
			} else if (parameters.InChannel()) {

			}
		} else if (t.Type == Task::TType::Write) {
			std::cout << "write parameters" << std::endl;
			return 0;
		}
	}
	return 0;
}


int Manager::ParseCLI(int argc, char **argv) {
	std::cout << ds(dlevel::Debug, "Manager::ParseCLI\n");

	args::ArgumentParser parser("This is a test program for RXDAQ.", "");						// parser
	args::HelpFlag help(parser, "help", "Display this help infomation.", {'h', "help"});		// help

	// init
	// args::Flag init(parser, "init", "initialize the system", {'i', "init"});
	// crate id
	args::ValueFlag<int> crate(parser, "id", "crate id ", {"crate", 'c'}, args::Options::Required);
	// config file
	args::ValueFlag<std::string> configFile(parser, "path", "path of the config file, required", {"config"}, args::Options::Required);


	// output level
	args::ValueFlag<std::string> outLevel(parser, "output level", "levels: error, warning, info, debug", {"level", 'l'});

	// // completion flag
	// args::CompletionFlag completion(parser, {"complete"});

	// commands
	args::Group commands(parser, "commands, at most one", args::Group::Validators::AtMostOne);
	args::Command cmdBoot(commands, "boot", "boot modules", [&](args::Subparser &parser) {
		cmdBootFunc(parser);
	});
	args::Command cmdRead(commands, "read", "read parameters", [&](args::Subparser &parser) {
		cmdReadWriteFunc(parser, true);
	});
	args::Command cmdWrite(commands, "write", "write parameters", [&](args::Subparser &parser) {
		cmdReadWriteFunc(parser, false);
	});



	try {
		parser.ParseCLI(argc, argv);
	}
	catch (const args::Help &h) {
		std::cout << parser;
		return 0;
	}
	catch (const args::ParseError &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return -1;
	}
	catch (const args::ValidationError &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return -1;
	}


	ds.SetLevel(dlevel::Error);
	if (outLevel) {
		if (outLevel.Get() == "error") {
		} else if (outLevel.Get() == "warning") {
			ds.SetLevel(dlevel::Warning);
		} else if (outLevel.Get() == "info") {
			ds.SetLevel(dlevel::Info);
		} else if (outLevel.Get() == "debug") {
			ds.SetLevel(dlevel::Debug);
		} else {
			std::stringstream ss;
			ss << "outLevel undefined " << outLevel.Get() << std::endl;
			std::cout << ds(dlevel::Error, ss.str());
			return -2;
		}
	}

	if (crate.Get() < 0) {
		std::cout << ds(dlevel::Error, "parameter crate is negative " + std::to_string(crate.Get()) + ".\n");
		return -3;
	}
	try {
		config.ReadFromFile(configFile.Get(), crate.Get());
	} catch (const std::exception &e) {
		std::cout << ds(dlevel::Error, e.what());
		return -3;
	}

	// check the module number and channel number
	for (auto &[key, t] : tasks) {
		// skip list command, this doesn't care about the module or channel
		if (t.List) continue;
		if (t.Module > config.ModuleNum()) {
			std::cout << ds(dlevel::Error, "args module id " + std::to_string(t.Module) + " over module size " + std::to_string(config.ModuleNum()) + ".\n");
			return -4;
		}
		if (t.Channel >= 16) {
			std::cout << ds(dlevel::Error, "args channle id " + std::to_string(t.Channel) + " over 15.\n");
		}
	}

	// add init system task, maybe the xia system need this init first?
	Task task{Task::TType::Init, -1, -1, "", false};
	tasks.insert(std::make_pair(Task::TPriority::Init, task));


	return 0;
}


void Manager::cmdBootFunc(args::Subparser &parser) {
	args::HelpFlag help(parser, "help", "display this boot help menu", {'h', "help"});
	args::ValueFlag<unsigned int> module(parser, "id", "the module id, default for all modules(-1)", {'m', "module"});

	parser.Parse();

	Task task;
	task.Type = Task::TType::Boot;
	task.Module = module ? module.Get() : -1;
	task.Channel = -1;
	task.List = false;
	task.Name = "";
	tasks.insert(std::make_pair(Task::TPriority::Boot, task));

}

// Manager::checkModuleInfo()
//  This function prints the information read from hardware and
//  compares them with the information read from the config json
//  file. If they're different, prints the error with dstring and
//  return negative value.
int Manager::checkModuleInfo() const {
	std::stringstream ss, es;								// normal stringstream and error stringstream
	bool diff = false;
	ss << "----------Module Firmware Information----------\n";
	ss << "id   Rev   Serial   Bits   ADC_Rate\n";
	for (unsigned short i = 0; i != config.ModuleNum(); ++i) {
		unsigned short tRev, tBit, tRate;
		unsigned int tSer;
		// read from hardware
		Pixie16ReadModuleInfo(i, &tRev, &tSer, &tBit, &tRate);
		// print the information read from hardware
		ss << std::setw(2) << i;							// module id
		ss << std::setw(6) << tRev;							// revision
		ss << std::setw(9) << tSer;							// serial number
		ss << std::setw(7) << tBit;							// ADC bits
		ss << std::setw(11) << tRate;						// ADC sampling rate
		ss << "\n";

		bool moduleDiff = false;
		// compare them with information read from config file
		if (tRev != config.Rev(i)) {
			es << "revision(" << config.Rev(i) << ") ";
			moduleDiff = true;
		}
		if (tRate != config.Rate(i)) {
			es << "rate(" << config.Rate(i) << ") ";
			moduleDiff = true;
		}
		if (tBit != config.Bits(i)) {
			es << "Bits(" << config.Bits(i) << ") ";
			moduleDiff = true;
		}
		if (moduleDiff) {
			es << " different in module " << i << ".\n";
			diff = true;
		}
	}
	std::cout << ds(dlevel::Info, ss.str());
	if (diff) {
		// The serveral lines below try to print the each error information
		// with colorful prefix respectively. So I use a loop and call ds()
		// for several times.
		std::string estr;
		std::getline(es, estr);
		while (!estr.empty()) {
			std::cout << ds(dlevel::Error, estr) << std::endl;
			std::getline(es, estr);
		}
		return -1;
	}
	return 0;
}


// Manager::checkBootRet
//	Check the return value of the function Pixie16BootModule and
//  output errors to explain the return value.
//	The switch part is copy from wuhongyi's PKUXIADAQ.
int Manager::checkBootRet(int retval) const {
	if (retval == 0) return 0;
	std::stringstream es;
	switch (retval) {
		case -1:
			es << "Invalid Pixie-16 module number.\n";
			break;
		case -2:
			es << "Size of ComFPGAConfigFile is invalid.\n";
			break;
		case -3:
			es << "Failed to boot Communication FPGA, Check log file.\n";
			break;
		case -4:
			es << "Failed to allocate memory to store data in ComFPGAConfigFile.\n";
			break;
		case -5:
			es << "Failed to open ComFPGAConfigFile.\n";
		break;
		case -10:
			es << "Size of SPFPGAConfigFile is invalid.\n";
			break;
		case -11:
			es << "Failed to boot signal processing FPGA, Check log file.\n";
			break;
		case -12:
			es << "Failed to allocate memory to store data in SPFPGAConfigFile.\n";
			break;
		case -13:
			es << "Failed to open SPFPGAConfigFile.\n";
			break;
		case -14:
			es << "Failed to boot DSP, Check log file.\n";
			break;
		case -15:
			es << "Failed to allocate memory to store DSP executable code.\n";
			break;
		case -16:
			es << "Failed to open DSPCodeFile.\n";
			break;
		case -17:
			es << "Size of DSPParFile is invalid.\n";
			break;
		case -18:
			es << "Failed to open DSPParFile.\n";
			break;
		case -19:
			es << "Can’t initialize DSP variable indices.\n";
			break;
		case -20:
			es << "Can’t copy DSP variable indices, Check log file.\n";
			break;
		case -21:
			es << "Failed to program Fippi in a module, Check log file.\n";
			break;
		case -22:
			es << "Failed to set DACs in a module, Check log file.\n";
			break;
		case -23:
			es << "Failed to start RESET_ADC run in a module, Check log file.\n";
			break;
		case -24:
			es << "RESET_ADC run timed out in a module, Check log file.\n";
			break;
		default:
			es << "Undefined error(" << retval << ").\n";
	}
	std::cout << ds(dlevel::Error, es.str());
	return -1;
}