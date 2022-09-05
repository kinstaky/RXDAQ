// #include <fstream>
// #include <iomanip>
// #include <thread>
// #include <vector>
#include <iostream>

#include "pixie/error.hpp"

#include "include/crate.h"
#include "include/error.h"
// #include "pixie/pixie16/legacy.hpp"
// #include "pixie/param.hpp"

namespace rxdaq {

typedef Message::Level MsgLevel;
typedef xia::pixie::error::error XiaError;


/*
 * Module FIFO realtime default settings.
 */
const size_t kFifoBuffers = 100;
const size_t kFifoRunWaitUsecs = 2000;
const size_t kFifoIdleWaitUsecs = 150000;
const size_t kFifoHoldUsecs = 50000;


Crate::Crate()
: message_(Message::Level::kWarning), config_path_("config.json") {
}


void Crate::Initialize(const std::string &config_path) {
	// read config from json file
	config_.ReadFromFile(config_path_);
	message_.SetLevel(Message::ToLevel(config_.MessageLevel()));

	std::cout << message_(MsgLevel::kDebug) << "Crate::Init().\n";

	// xia crate init
	xia_crate_.initialize();

	// check module size
	if (!xia_crate_.modules.size()) {
		throw XiaError(
			xia::pixie::error::code::module_total_invalid,
			"Crate did not initialize with any modules."
		);
	}
	if (config_.ModuleNum() > xia_crate_.num_modules) {
		throw XiaError(
			XiaError::code::module_total_invalid,
			"module count does not match user supplied number of modules"
		);
	}

	// bind modules to the physical slots
	xia::pixie::module::number_slots numbers;
	for (unsigned short i = 0; i != config_.ModuleNum(); ++i) {
		typedef xia::pixie::module::number_slot number_slot;
		numbers.push_back(number_slot(i, config_.Slot(i)));
	}
	xia_crate_.assign(numbers);

	// set the FIFO realtime settings
	for (auto &module : xia_crate_.modules) {
		module->fifo_buffers = kFifoBuffers;
		module->fifo_run_wait_usecs = kFifoRunWaitUsecs;
		module->fifo_hold_usecs = kFifoHoldUsecs;
	}

	std::cout << message_(MsgLevel::kDebug) << "Crate::Init() finish.\n";
}


void Crate::Boot(unsigned short module_id, unsigned short boot_pattern) {
	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot(" << module_id
		<< ", 0x" << std::hex << boot_pattern << ")\n"; 

	// add crate user
	xia::pixie::crate::crate::user user(xia_crate_);

	// check module firmware information before boot
	xia::pixie::module::module &module = *(xia_crate_.modules[module_id]);
	if (module.eeprom.revision != config_.Revision(module_id)) {
		std::stringstream ss;
		ss << "Revision of module " << module_id << " is " << module.revision
			<< " NOT equal to " << config_.Revision(module_id)
			<< " in config file.\n";
		throw UserError(ss.str());
	}
	if (module.eeprom.configs[0].adc_msps != config_.Rate(module_id)) {
		std::stringstream ss;
		ss << "ADC sampling rate of module " << module_id << " is "
			<< module.eeprom.configs[0].adc_msps << " NOT equal to "
			<< config_.Rate(module_id) << " in config file.\n";
		throw UserError(ss.str());
	}
	if (module.eeprom.configs[0].adc_bits != config_.Bits(module_id)) {
		std::stringstream ss;
		ss << "ADC bits of module " << module_id << " is "
			<< module.eeprom.configs[0].adc_bits << " NOT equal to "
			<< config_.Bits(module_id) << " in config file.\n";
		throw UserError(ss.str());
	}

	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot: loading firmwares...\n";

	// check firmwares
	xia::pixie::firmware::firmware_ref firmwares[4];			// firmwares
	std::string firmware_files[4] = {
		config_.Sys(module_id),
		config_.Fippi(module_id),
		config_.Ldr(module_id),
		config_.Var(module_id)
	};
	std::string device_name[4] = {"sys", "fippi", "dsp", "var"};

	// create or get firmware
	firmwares_lock_.lock();
	for (size_t i = 0; i != 4; ++i) {
		auto search = firmwares_.find(firmware_files[i]);
		if (search == firmwares_.end()) {
			
			// firmware not exist, add new one
			firmwares[i] = std::make_shared<xia::pixie::firmware::firmware>(
				"n/a", module.revision, module.eeprom.configs[0].adc_msps,
				module.eeprom.configs[0].adc_bits, device_name[i]
			);
			
			// load firmware from file
			firmwares[i]->filename = firmware_files[i];
			firmwares[i]->load();

			// insert new one to map
			firmwares_.insert(std::make_pair(firmware_files[i], firmwares[i]));
		
		} else {
			firmwares[i] = search->second;
		}
		// record module in firmware
		firmwares[i]->slot.push_back(module.slot);
	}
	firmwares_lock_.unlock();


	// std::string tag = fw[0]->tag;
	for (auto &fp : firmwares) {
		module.firmware.push_back(fp);
	}

	const int kBOOT_COMFPGA = 0x1;
	const int kBOOT_SPFPGA = 0x2;
	const int kBOOT_DSPCODE = 0x4;
	// const int kBOOT_DSPPAR = 0x8;

	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot: booting module...\n";

	// now boot module
	module.probe();
	module.boot(
		boot_pattern & kBOOT_COMFPGA,
		boot_pattern & kBOOT_SPFPGA,
		boot_pattern & kBOOT_DSPCODE
	);

	// Import(module_id, config_.Par(module_id));


	// try {
	// 	std::cout << ds(dlv::Debug, "Crate::bootOneModule: load parameters settings\n");

	// 	xia::pixie::legacy::settings settings(module);
	// 	settings.load(config_.Par(module_));
	// 	settings.import(module);
	// 	settings.write(module);
	// 	module.sync_vars();
	// 	if ((boot_pattern & kBOOT_DSPPAR) != 0) {
	// 		module.sync_hw();
	// 	}

	// }  catch (const xia::pixie::error::error& e) {
	// 	if (e.type == xia::pixie::error::code::module_total_invalid) {
	// 		xia::pixie::module::number_slots modules;
	// 		xia_crate_.import_config(config_.Par(module_id), modules);
	// 	} else {
	// 		throw xia::pixie::error::error(e.type, e.what());
	// 	}
	// }


	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot() finish.\n";
	return;
}

}	 // namespace rxdaq






// Crate::Crate() {
// 	configFile = "";
// }

// Crate::~Crate() {
// }



// void Crate::SetConfigFile(const char *path) {
// 	configFile = path;
// }


// void Crate::SetRunConfig(const std::string &path_) {
// 	runConfig.configFile = path_;
// 	std::ifstream fin(path_, std::ios::in);
// 	if (fin.good()) {
// 		// throw UserError("Could not open file " + path_ + ".\n");
// 		nlohmann::json &js = runConfig.js;
// 		fin >> js;
// 	// to do, add file checking
// 		runConfig.dataPath = js["path"];
// 		runConfig.dataFileName = js["fileName"];
// 		runConfig.run = js["run"];
// 	}
// 	return;
// }

// void Crate::SetRunConfig(const std::string &path_, const std::string &name_, unsigned int run_) {
// 	runConfig.dataPath = path_;
// 	runConfig.dataFileName = name_;
// 	runConfig.run = run_;

// 	nlohmann::json &js = runConfig.js;
// 	js["path"] = path_;
// 	js["fileName"] = name_;
// 	js["run"] = run_;
// 	if (runConfig.configFile == "") {
// 		runConfig.configFile = "./run.json";
// 	}
// 	std::ofstream fout(runConfig.configFile);
// 	if (!fout.good()) {
// 		throw UserError("Could not open file " + runConfig.configFile + ".\n");
// 	}
// 	fout << js.dump(2) << std::endl;
// 	return;
// }






// void Crate::bootOneModule(unsigned short module_, unsigned short bootPattern_) {
// 	std::stringstream ss;
// 	ss << "Crate::bootOneModule(" << module_ << ", 0x" << std::hex << bootPattern_ << ")\n";
// 	std::cout << ds(dlv::Debug, ss.str());

// 	// check module firmware information before boot
// 	xia::pixie::module::module &module = *(xia_crate_.modules[module_]);
// 	if (module.revision != config_.Rev(module_)) {
// 		std::stringstream ss;
// 		ss << "Revision of module " << module_ << " is " << module.revision;
// 		ss << " NOT equal to " << config_.Rev(module_) << " in config file.\n";
// 		throw UserError(ss.str());
// 	}
// 	if (module.configs[0].adc_msps != config_.Rate(module_)) {
// 		std::stringstream ss;
// 		ss << "ADC sampling rate of module " << module_ << " is " << module.configs[0].adc_msps;
// 		ss << " NOT equal to " << config_.Rate(module_) << " in config file.\n";
// 		throw UserError(ss.str());
// 	}
// 	if (module.configs[0].adc_bits != config_.Bits(module_)) {
// 		std::stringstream ss;
// 		ss << "ADC bits of module " << module_ << " is " << module.configs[0].adc_bits;
// 		ss << " NOT equal to " << config_.Bits(module_) << " in config file.\n";
// 		throw UserError(ss.str());
// 	}

// 	std::cout << ds(dlv::Debug, "Crate::bootOneModule: load firmwares\n");

// 	// check firmwares
// 	typedef std::shared_ptr<Firmware> Firmware_ref;
// 	Firmware_ref fw[4];			// firmwares
// 	std::string fileName[4] = {config_.Sys(module_), config_.Fippi(module_), config_.Ldr(module_), config_.Var(module_)};
// 	std::string deviceName[4] = {"sys", "fippi", "dsp", "var"};

// 	// create or get firmware
// 	lck.lock();
// 	for (size_t i = 0; i != 4; ++i) {
// 		if (firmwares.find(fileName[i]) == firmwares.end()) {
// 			fw[i] = std::make_shared<Firmware>("n/a", module.revision, module.configs[0].adc_msps, module.configs[0].adc_bits, deviceName[i], fileName[i]);
// 			firmwares.insert(std::make_pair(fileName[i], fw[i]));
// 		} else {
// 			fw[i] = firmwares[fileName[i]];
// 		}
// 		fw[i]->slot.push_back(module.slot);
// 	}
// 	lck.unlock();



// 	// firmware comm_fw("n/a", module.revision, module.configs[0].adc_msps, module.configs[0].adc_bits, "sys");
// 	// firmware fippi_fw("n/a", module.revision, module.configs[0].adc_msps, module.configs[0].adc_bits, "fippi");
// 	// firmware dsp_fw("n/a", module.revision, module.configs[0].adc_msps, module.configs[0].adc_bits, "dsp");
// 	// firmware dsp_var("n/a", module.revision, module.configs[0].adc_msps, module.configs[0].adc_bits, "var");


// 	for (size_t i = 0; i != 4; ++i) {
// 		fw[i]->load();
// 	}

// 	// comm_fw->lck.lock();
// 	// comm_fw->filename = config_.Sys(module_);
// 	// comm_fw->readData = false;
// 	// comm_fw->slot.push_back(module.slot);
// 	// comm_fw->lck.unlock();

// 	// fippi_fw->lck.lock();
// 	// fippi_fw->filename = config_.Fippi(module_);
// 	// fippi_fw->readData = false;
// 	// fippi_fw->slot.push_back(module.slot);
// 	// fippi_fw->lck.unlock();

// 	// dsp_fw->lck.lock();
// 	// dsp_fw->filename = config_.Ldr(module_);
// 	// dsp_fw->readData = false;
// 	// dsp_fw->slot.push_back(module.slot);
// 	// dsp_fw->lck.unlock();

// 	// dsp_var->lck.lock();
// 	// dsp_var->filename = config_.Var(module_);
// 	// dsp_var->readData = false;
// 	// dsp_var->slot.push_back(module.slot);
// 	// dsp_var->lck.unlock();

// 	// std::string tag = fw[0]->tag;
// 	for (auto &fp : fw) {
// 		module.firmware.push_back(fp);
// 	}

// 	// xia::pixie::crate::crate::guard *crateGuard;
// 	// crateGuard = new xia::pixie::crate::crate::guard(xia_crate_);
// 	// xia::pixie::firmware::add(xia_crate_.firmware, comm_fw);
// 	// xia::pixie::firmware::add(xia_crate_.firmware, fippi_fw);
// 	// xia::pixie::firmware::add(xia_crate_.firmware, dsp_fw);
// 	// xia::pixie::firmware::add(xia_crate_.firmware, dsp_var);


// 	// // xia_crate_.set_firmware();
// 	// auto tag = comm_fw.tag;
// 	// module.add(xia_crate_.firmware[tag]);


// 	// for (auto &fw : xia_crate_.firmware[tag]) {
// 	// 	fw->load();
// 	// }
// 	// delete crateGuard;

// 	// xia::pixie::firmware::load(xia_crate_.firmware[tag]);


// 	const int BOOTPATTERN_COMFPGA_BIT = 0x1;
// 	const int BOOTPATTERN_SPFPGA_BIT = 0x2;
// 	const int BOOTPATTERN_DSPCODE_BIT = 0x4;
// 	const int BOOTPATTERN_DSPPAR_BIT = 0x8;

// 	bool boot_comm = (bootPattern_ & BOOTPATTERN_COMFPGA_BIT) != 0;
// 	bool boot_fippi = (bootPattern_ & BOOTPATTERN_SPFPGA_BIT) != 0;
// 	bool boot_dsp = (bootPattern_ & BOOTPATTERN_DSPCODE_BIT) != 0;

// 	std::cout << ds(dlv::Debug, "Crate::bootOneModule: module boot\n");

// 	module.probe();
// 	module.boot(boot_comm, boot_fippi, boot_dsp);

// 	bool json_config = false;
// 	xia::pixie::legacy::settings settings(module);

// 	try {
// 		std::cout << ds(dlv::Debug, "Crate::bootOneModule: load parameters settings\n");

// 		settings.load(config_.Par(module_));
// 	}  catch (xia::pixie::error::error& err) {
// 		if (err.type == xia::pixie::error::code::module_total_invalid) {
// 			json_config = true;
// 			xia::pixie::module::number_slots modules;
// 			xia_crate_.import_config(config_.Par(module_), modules);
// 		} else {
// 			throw;
// 		}
// 	}

// 	if (!json_config) {
// 		settings.import(module);
// 		settings.write(module);
// 		module.sync_vars();
// 		if ((bootPattern_ & BOOTPATTERN_DSPPAR_BIT) != 0) {
// 			module.sync_hw();
// 		}
// 	}

// 	std::cout << ds(dlv::Debug, "Crate::bootOneModule() ends\n");
// 	return;
// }





// void Crate::ReadModuleParameter(const std::string &name_, unsigned int *par_, unsigned short module_) {
// 	std::cout << ds(dlv::Debug, "Crate::ReadModuleParameter(" + name_ + ", " + std::to_string(module_) + ")\n");
// 	try {
// 		xia_crate_.ready();
// 		xia::pixie::crate::module_handle module(xia_crate_, module_);
// 		*par_ = module->read(name_);
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::ReadChannelParameter(const std::string &name_, double *par_, unsigned short module_, unsigned short channel_) {
// 	std::cout << ds(dlv::Debug, "Crate::ReadChannelParameter(" + name_ + ", " + std::to_string(module_) + ", " + std::to_string(channel_) + ")\n");
// 	try {
// 		xia_crate_.ready();
// 		xia::pixie::crate::module_handle module(xia_crate_, module_);
// 		*par_ = module->read(name_, channel_);
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::WriteModuleParameter(const std::string &name_, unsigned int val_, unsigned short module_) {
// 	std::cout << ds(dlv::Debug, "Crate::WriteModuleParameter(" + name_ + ", " + std::to_string(val_) + ", " + std::to_string(module_) + ")\n");
// 	try {
// 		xia_crate_.ready();
// 		bool bcast;
// 		if (module_ == AllModules) {
// 			bcast = true;
// 		} else {
// 			xia::pixie::crate::module_handle module(xia_crate_, module_);
// 			bcast = module->write(name_, val_);
// 		}
// 		// some parameters should be written to all modules
// 		if (bcast) {
// 			xia::pixie::crate::crate::user user(xia_crate_);
// 			for (auto &module : xia_crate_.modules) {
// 				if (module_ != module->number && module->online()) {
// 					module->write(name_, val_);
// 				}
// 			}
// 		}
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }

// void Crate::WriteChannelParameter(const std::string &name_, double val_, unsigned short module_, unsigned short channel_) {
// 	std::cout << ds(dlv::Debug, "Crate::WriteChannelParameter(" + name_ + ", " + std::to_string(val_) + ", " + std::to_string(module_) + ", " + std::to_string(channel_) + ")\n");
// 	try {
// 		xia_crate_.ready();
// 		xia::pixie::crate::module_handle module(xia_crate_, module_);
// 		module->write(name_, channel_, val_);
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }

// void Crate::Run(unsigned short module_, unsigned int time_) {
// 	std::cout << ds(dlv::Debug, "Crate::Run(" + std::to_string(module_) + ", " + std::to_string(time_) + ")\n");
// 	std::cout << ds(dlv::Info, "Starting list mode data run for " + std::to_string(time_) + " s.\n");

// 	try {
// 		WriteModuleParameter("SYNCH_WAIT", 1, 0);
// 		WriteModuleParameter("IN_SYNCH", 0, 0);

// 		bool runAll = module_ == AllModules;
// 		xia::pixie::hw::run::run_mode runMode = xia::pixie::hw::run::run_mode::new_run;
// 		xia_crate_.ready();
// 		auto startTime = std::chrono::steady_clock::now();
// 		if (runAll) {					// run all modules
// 			// run in list mode
// 			for (auto &module : xia_crate_.modules) {
// 				module->start_listmode(runMode);
// 			}

// 			// output streams
// 			std::vector<std::ofstream> outputStreams;
// 			for (unsigned short i = 0; i != config_.ModuleNum(); ++i) {
// 				outputStreams.push_back(std::ofstream());
// 				std::stringstream ss;
// 				ss << runConfig.dataPath << runConfig.dataFileName << "_R" << std::setfill('0') << std::setw(4) << runConfig.run << "_M" << std::setfill('0') << std::setw(2) << i << ".bin";
// 				outputStreams[i].open(ss.str());
// 			}
// 			auto stopTime = std::chrono::steady_clock::now();
// 			while (std::chrono::duration_cast<std::chrono::seconds>(stopTime - startTime).count() <= time_) {
// 				for (unsigned short m = 0; m != config_.ModuleNum(); ++m) {
// 					xia::pixie::crate::module_handle module(xia_crate_, m);
// 					if (module->run_active()) {
// 						readListData(module, outputStreams[m], 131072.0*0.2);
// 					} else {
// 						std::cout << ds(dlv::Info, "Module " + std::to_string(m) + " has no active run.\n");
// 					}
// 				}
// 			}

// 			EndRun(module_);	// try several times

// 			for (unsigned short m = 0; m != config_.ModuleNum(); ++m) {
// 				xia::pixie::crate::module_handle module(xia_crate_, m);
// 				readListData(module, outputStreams[m], 0);
// 			}

// 			for (auto &fout : outputStreams) {
// 				fout.close();
// 			}
// 		} else {						// run just one module
// 			xia::pixie::crate::module_handle module(xia_crate_, module_);
// 			module->start_listmode(runMode);

// 			// output stream
// 			std::stringstream ss;
// 			ss << runConfig.dataPath << runConfig.dataFileName << "_R" << std::setfill('0') << std::setw(4) << runConfig.run << "_M" << std::setfill('0') << std::setw(2) << module_ << ".bin";
// 			std::ofstream fout(ss.str());

// 			auto stopTime = std::chrono::steady_clock::now();
// 			while (std::chrono::duration_cast<std::chrono::seconds>(stopTime - startTime).count() <= time_) {
// 				if (module->run_active()) {
// 					readListData(module, fout, 131072.0*0.2);
// 				} else {
// 					std::cout << ds(dlv::Info, "Module " + std::to_string(module_) + "has no active run.\n");
// 				}
// 			}

// 			EndRun(module_);

// 			readListData(module, fout, 0);
// 			fout.close();
// 		}

// 		// display run time
// 		auto stopTime = std::chrono::steady_clock::now();
// 		int runSeconds = std::chrono::duration_cast<std::chrono::seconds>(stopTime - startTime).count();
// 		int runMinutes = runSeconds / 60;
// 		int runHours = runMinutes / 60;
// 		runMinutes %= 60;
// 		runSeconds %= 3600;
// 		std::stringstream ss;
// 		ss << "List mode run finished in ";
// 		if (!runHours) ss << runHours << ":";
// 		if (!runMinutes) ss << runMinutes << ":";
// 		ss << runSeconds << " .\n";
// 		std::cout << ds(dlv::Info, ss.str());
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::EndRun(unsigned short module_) {
// 	std::cout << ds(dlv::Debug, "EndRun(" + (module_ == AllModules ? "All" : std::to_string(module_)) + ")\n");
// 	std::cout << ds(dlv::Info, "Stopping list mode run.\n");

// 	try {
// 		const unsigned int maxAttempts = 30;
// 		xia_crate_.ready();
// 		if (module_ == AllModules) {
// 			for (auto &module : xia_crate_.modules) {
// 				module->run_end();
// 			}

// 			bool finished = false;
// 			for (unsigned int count = 0; count != maxAttempts; ++count) {
// 				finished = true;
// 				for (auto &module : xia_crate_.modules) {
// 					if (module->run_active()) {
// 						finished = false;
// 					}
// 				}
// 				if (finished) break;
// 				std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 			}
// 			if (!finished) {
// 				throw UserError("Not all modules stop run properly!\n");
// 			}
// 		} else {
// 			// (xia_crate_.modules[module_])->run_end();
// 			xia::pixie::crate::module_handle module(xia_crate_, module_);
// 			module->run_end();
// 			bool finished = false;
// 			for (unsigned int count = 0; count != maxAttempts; ++count) {
// 				if (!module->run_active()) {
// 					finished = true;
// 					break;
// 				}
// 				std::this_thread::sleep_for(std::chrono::milliseconds(100));
// 			}
// 			if (!finished) {
// 				throw UserError("Module " + std::to_string(module_) + "did not stop run properly!\n");
// 			}
// 		}
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::ExportParameters(const std::string &path_) {
// 	std::cout << ds(dlv::Debug, "Crate::ExportParameters(" + path_ + ").\n");
// 	try {
// 		xia_crate_.export_config(path_);
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::ImportParameters(const std::string &path_) {
// 	std::cout << ds(dlv::Debug, "Crate::ImportParameters(" + path_ + ").\n");
// 	try {
// 		xia::pixie::module::number_slots loaded;
// 		xia_crate_.import_config(path_, loaded);

// 		std::stringstream ss;
// 		for (auto &slot : loaded) {
// 			ss << slot.first << "->" << slot.second << "  ";
// 		}
// 		ss << "\n";
// 		std::cout << ds(dlv::Debug, ss.str());
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// void Crate::readListData(xia::pixie::crate::module_handle &module_, std::ofstream &fout, unsigned int threshold) {
// 	try {
// 		unsigned int fifoWords = static_cast<unsigned int>(module_->read_list_mode_level());
// 		if (fifoWords > threshold) {
// 			xia::pixie::hw::words data(fifoWords);
// 			module_->read_list_mode(data);
// 			fout.write(reinterpret_cast<char*>(data.data()), fifoWords*sizeof(uint32_t));
// 		}
// 	} catch (const UserError &e) {						// user operation error
// 		throw UserError(e.what());
// 	} catch (const XiaError &e) {						// fatal xia error
// 		throw XiaError(e.type, e.what());
// 	} catch (const RXError &e) {						// fatal RXDAQ error
// 		throw RXError(e.what());
// 	} catch (const std::exception &e) {					// fatal error
// 		throw std::runtime_error(e.what());
// 	}
// }


// int Crate::CheckParameterName(const std::string &name) {
// 	std::cout << ds(dlv::Debug, "Crate::CheckParameterName(" + name + ")\n");
// 	// typedef xia::pixie::param XiaParam;
// 	if (xia::pixie::param::is_module_param(name)) return TypeModuleParameter;
// 	if (xia::pixie::param::is_channel_param(name)) return TypeChannelParameter;
// 	return 0;
// 	// if (XiaP50aram::is_system_param(name)) return TypeSystemParameter;
// 	// if (XiaParam::is_module_var(name)) return TypeModuleVariable;
// 	// if (XiaParam::is_channel_var(name)) return TypeChannelVariable;
// }


// std::string Crate::ListParameters(int type) {
// 	std::cout << ds(dlv::Debug, "Crate::ListParameters(" + std::to_string(type) + ")\n");
// 	typedef xia::pixie::param::module_param_map module_param_map;
// 	typedef xia::pixie::param::channel_param_map channel_param_map;
// 	std::stringstream ss;
// 	ss << "Module parameters" << std::endl;
// 	module_param_map moduleParams = xia::pixie::param::get_module_param_map();
// 	for (auto &par : moduleParams) {
// 		ss << "    " << par.first << std::endl;
// 	}
// 	ss << "\nChannel parameters" << std::endl;
// 	channel_param_map channelParams = xia::pixie::param::get_channel_param_map();
// 	for (auto &par : channelParams) {
// 		ss << "    " << par.first << std::endl;
// 	}
// 	return ss.str();
// }

// unsigned short Crate::ModuleNum() const {
// 	std::cout << ds(dlv::Debug, "Crate::ModuleNum()\n");
// 	return config_.ModuleNum();
// }


// void Crate::PrintInfo() const {
// 	std::cout << ds(dlv::Debug, "Crate::PrintInfo()\n");
// 	std::cout << ds(dlv::Info, config_.CrateInfoStr());
// 	return;
// }



// }		// RXDAQ