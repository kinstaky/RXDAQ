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


Crate::Crate() noexcept
: message_(Message::Level::kWarning), config_path_("config.json") {
}


void Crate::Initialize(const std::string &config_path) {
	if (!config_path.empty()) {
		config_path_ = config_path;
	}
	// read config from json file
	config_.ReadFromFile(config_path_);
	message_.SetLevel(Message::ToLevel(config_.MessageLevel()));

	std::cout << message_(MsgLevel::kDebug) << "Crate::Init().\n";

	// xia crate init
	xia_crate_.initialize();

	// check module size
	if (!xia_crate_.modules.size()) {
		xia_crate_.shutdown();
		throw XiaError(
			xia::pixie::error::code::module_total_invalid,
			"Crate did not initialize with any modules."
		);
	}
	if (config_.ModuleNum() > xia_crate_.num_modules) {
		xia_crate_.shutdown();
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

	// // set the FIFO realtime settings
	// for (auto &module : xia_crate_.modules) {
	// 	module->fifo_buffers = kFifoBuffers;
	// 	module->fifo_run_wait_usecs = kFifoRunWaitUsecs;
	// 	module->fifo_hold_usecs = kFifoHoldUsecs;
	// }

	std::cout << message_(MsgLevel::kDebug) << "Crate::Init() finish.\n";
}


void Crate::LoadFirmware(unsigned short module_id) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::LoadFirmware(" << module_id << ")\n";

	// add crate user
	xia::pixie::crate::crate::user user(xia_crate_);

	// check module firmware information before boot
	xia::pixie::module::module &module = *(xia_crate_.modules[module_id]);
	// xia::pixie::crate::module_handle module(xia_crate_, module_id, xia::pixie::crate::module_handle::present);
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

	std::cout << message_(MsgLevel::kDebug) << "Crate::LoadFirmwares: loading firmwares...\n";

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
		firmwares[i]->slot.push_back(config_.Slot(module_id));
	}
	firmwares_lock_.unlock();

	for (auto &firmware : firmwares) {
		module.firmware.push_back(firmware);
	}
}


void Crate::BootModule(unsigned short module_id, bool fast) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::BootModule(" << module_id << ", " << fast << ").\n";

	LoadFirmware(module_id);
	
	xia::pixie::crate::module_handle module(
		xia_crate_, module_id, xia::pixie::crate::module_handle::present
	);
	
	if (!fast || !module->online()) {
		module->boot();
	}
}

void Crate::Boot(unsigned short module_id, bool fast) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::Boot(" << module_id << ", " << fast << ").\n";

	// load firmware
	if (module_id == kModuleNum) {
		std::vector<std::thread> threads;
		for (unsigned short i = 0; i < ModuleNum(); ++i) {
			threads.emplace_back(&Crate::BootModule, this, i, fast);
		}
		for (auto &t : threads) {
			t.join();
		}
	} else {
		BootModule(module_id, fast);
	}
	if (!fast) {
		ImportParameters(config_.Par(0));
	}
}


// void Crate::BootCrate(bool fast) {
// 	std::cout << message_(MsgLevel::kDebug)
// 		<< "Crate::BootCrate(" << fast << ")\n";

// 	xia_crate_.ready();
	
// 	if (fast) {
// 		xia_crate_.boot(false);
// 	} else {
// 		xia_crate_.boot(true);
// 		ImportParameters(config_.Par(0));
// 	}
// }


// // The function is edited from PixieRegisterFirmware, but the origin function
// // create the firmware for twice. So I edit it and create only once.
// void Crate::Boot(unsigned short module_id, unsigned short boot_pattern) {
// 	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot(" << module_id
// 		<< ", 0x" << std::hex << boot_pattern << std::dec << ")\n"; 


// 	LoadFirmware(module_id);

// 	xia::pixie::crate::crate::user user(xia_crate_);
// 	// check module firmware information before boot
// 	xia::pixie::module::module &module = *(xia_crate_.modules[module_id]);

// 	const int kBOOT_COMFPGA = 0x1;
// 	const int kBOOT_SPFPGA = 0x2;
// 	const int kBOOT_DSPCODE = 0x4;

// 	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot: booting module...\n";

// 	// now boot module
// 	module.probe();
// 	module.boot(
// 		boot_pattern & kBOOT_COMFPGA,
// 		boot_pattern & kBOOT_SPFPGA,
// 		boot_pattern & kBOOT_DSPCODE
// 	);

	// if (boot_pattern & kBOOT_DSPPAR) {
	// 	ImportParameters(config_.Par(module_id));
	// }

// 	std::cout << message_(MsgLevel::kDebug) << "Crate::Boot() finish.\n";
// 	return;
// }


//-----------------------------------------------------------------------------
//	 				method to read and write parameters
//-----------------------------------------------------------------------------

std::string Crate::ListParameters(ParameterType type) noexcept {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::ListParameters(" << kParameterTypeNames.at(type) << ")\n";

	std::string result = "";
	
	// module parameters
	if (type == ParameterType::kAll || type == ParameterType::kModule) {
		result += "Module parameters\n";
		const auto module_parameters = xia::pixie::param::get_module_param_map();
		for (const auto &parameter : module_parameters) {
			result += "  " + parameter.first + "\n";
		}
	}

	// channel parameters
	if (type == ParameterType::kAll || type == ParameterType::kChannel) {
			result += "\nChannel parameters\n";
		const auto channel_parameters = xia::pixie::param::get_channel_param_map();
		for (const auto &parameter : channel_parameters) {
			result += "  " + parameter.first + "\n";
		}
	}

	return result;
}


ParameterType Crate::CheckParameter(const std::string &name) noexcept {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::CheckParameterName(" << name << ")\n";

	if (xia::pixie::param::is_module_param(name)) {
		return ParameterType::kModule;
	}
	if (xia::pixie::param::is_channel_param(name)) {
		return ParameterType::kChannel;
	}
	return ParameterType::kInvalid;
	// if (XiaParam::is_system_param(name)) return TypeSystemParameter;
	// if (XiaParam::is_module_var(name)) return TypeModuleVariable;
	// if (XiaParam::is_channel_var(name)) return TypeChannelVariable;
}




unsigned int Crate::ReadParameter(
	const std::string &name,
	unsigned short module
) {
	
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::ReadModuleParameter(" << name << ", " << module << ")\n";

	xia_crate_.ready();
	xia::pixie::crate::module_handle module_handler(xia_crate_, module);
	return module_handler->read(name);
}


double Crate::ReadParameter(
	const std::string &name,
	unsigned short module,
	unsigned short channel
) {

	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::ReadChannelParameter(" << name << ", " << module
		<< ", " << channel << ")\n";
	xia_crate_.ready();
	xia::pixie::crate::module_handle module_handler(xia_crate_, module);
	return module_handler->read(name, channel);
}



void Crate::WriteParameter(
	const std::string &name,
	unsigned int value,
	unsigned short module
) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::WriteModuleParameter("  << name << ", " << value
		<< ", " << module <<  ")\n";

	xia_crate_.ready();
	bool bcast;
	if (module == kModuleNum) {
		bcast = true;
	} else {
		xia::pixie::crate::module_handle module_handler(xia_crate_, module);
		bcast = module_handler->write(name, value);
	}
	// some parameters should be written to all modules
	if (bcast) {
		xia::pixie::crate::crate::user user(xia_crate_);
		for (auto &m : xia_crate_.modules) {
			if (module != m->number && m->online()) {
				m->write(name, value);
			}
		}
	}
}


void Crate::WriteParameter(
	const std::string &name,
	double value,
	unsigned short module,
	unsigned short channel
) {

	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::WriteChannelParameter(" << name << ", " << value << ", "
		<< module << ", " << channel << ")\n";
	xia_crate_.ready();
	xia::pixie::crate::module_handle module_handler(xia_crate_, module);
	module_handler->write(name, channel, value);
}



void Crate::ImportParameters(const std::string &path) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::ImportParameters(" << path << ").\n";

	xia::pixie::module::number_slots loaded;
	xia_crate_.import_config(path, loaded);

	// try {
	// 	std::cout << message_(MsgLevel::kDebug)
	// 		<< "Crate::ImportParameters: load parameters settings\n";

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


}



void Crate::ExportParameters(const std::string &path) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::ExportParameters(" <<  path << ").\n";
	
	xia_crate_.export_config(path);
}

inline unsigned int ClockDuration(
	std::chrono::steady_clock::time_point start,
	std::chrono::steady_clock::time_point stop
) {
	return std::chrono::duration_cast<std::chrono::seconds>(
		stop - start
	).count();
}

void Crate::StartRun(unsigned short module_id, unsigned int seconds, int run) {
	if (run != -1) {
		config_.SetRunNumber(run);
	} else {
		run = config_.RunNumber();
	}

	std::cout << message_(MsgLevel::kDebug)
		<<  "Crate::Run(" << module_id << ", " << seconds << " s, " << run << ")\n";
	std::cout << message_(MsgLevel::kInfo)
		<< "Starting list mode run" << (
			seconds ?
			" " + std::to_string(seconds) + " seconds" :
			""
		)
		<< ".\n"; 

	xia::pixie::hw::run::run_mode run_mode =
		xia::pixie::hw::run::run_mode::new_run;
	xia_crate_.ready();
	auto start_time = std::chrono::steady_clock::now();

	if (module_id == kModuleNum) {
		WriteParameter("SYNCH_WAIT", 1, 0);
		WriteParameter("IN_SYNCH", 0, 0);

		for (const auto &module : xia_crate_.modules) {
			module->start_listmode(run_mode);
		}
		
		std::vector<std::ofstream> output_streams = [&]() {
			std::vector<std::ofstream> result;
			for (unsigned short i = 0; i < ModuleNum(); ++i) {

				std::stringstream file_name;
				file_name << config_.RunDataPath() << config_.RunDataFile()
					<< "_R" << std::setfill('0') << std::setw(4) << run
					<< "_M" << std::setfill('0') << std::setw(2) << i << ".bin";
				result.push_back(std::ofstream(file_name.str()));
			}
			return result;
		}();

		auto stop_time = std::chrono::steady_clock::now();
		while (ClockDuration(start_time, stop_time) < seconds) {
			for (unsigned short i = 0; i < ModuleNum(); ++i) {
				xia::pixie::crate::module_handle module(xia_crate_, i);
				if (module->run_active()) {
					ReadListModeData(module, output_streams[i], 131072.0*0.2);
				} else {
					std::cout << message_(MsgLevel::kInfo)
						<< "Module " << i << " has no active run.\n";
				}
			}
		}

		StopRun(module_id);

		for (unsigned short i = 0; i < ModuleNum(); ++i) {
			xia::pixie::crate::module_handle module(xia_crate_, i);
			ReadListModeData(module, output_streams[i], 0);
		}

		for (auto &file : output_streams) {
			file.close();
		}
	} else {
		WriteParameter("IN_SYNCH", 0, module_id);
		xia::pixie::crate::module_handle module(xia_crate_, module_id);
		module->start_listmode(run_mode);

		// output stream
		std::stringstream file_name;
		file_name << config_.RunDataPath() << config_.RunDataFile() << "_R"
			<< std::setfill('0') << std::setw(4) << run << "_M"
			<< std::setfill('0') << std::setw(2) << module_id << ".bin";
		std::ofstream fout(file_name.str());

		auto stop_time = std::chrono::steady_clock::now();
		while (ClockDuration(start_time, stop_time) <= seconds) {
			if (module->run_active()) {
				ReadListModeData(module, fout, 131072.0*0.2);
			} else {
				std::cout << message_(MsgLevel::kInfo)
					<< "Module " << module_id << "has no active run.\n";
			}
		}

		StopRun(module_id);

		ReadListModeData(module, fout, 0);
		fout.close();
	}

	// display run time
	auto stop_time = std::chrono::steady_clock::now();
	unsigned int run_seconds = ClockDuration(start_time, stop_time);
	unsigned int run_minutes = run_seconds / 60;
	unsigned int run_hours = run_minutes / 60;
	run_minutes %= 60;
	run_seconds %= 3600;
	std::cout << message_(MsgLevel::kInfo)
		<< "List mode run finished in "
		<< (run_hours ? std::to_string(run_hours) + ":" : "")
		<< (run_minutes ? std::to_string(run_minutes) + ":" : "")
		<< run_seconds << ".\n";

}



void Crate::StopRun(unsigned short module_id) {
	std::cout << message_(MsgLevel::kDebug)
		<< "StopRun(" << module_id << ")\n";
	std::cout << message_(MsgLevel::kInfo)
		<< "Stopping list mode run.\n";

	const unsigned int kMaxAttempts = 30;
	xia_crate_.ready();
	if (module_id == kModuleNum) {
		for (auto &module : xia_crate_.modules) {
			module->run_end();
		}

		bool finished = false;
		for (unsigned int count = 0; count < kMaxAttempts; ++count) {
			finished = true;
			for (auto &module : xia_crate_.modules) {
				if (module->run_active()) {
					finished = false;
				}
			}
			if (finished) break;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (!finished) {
			throw UserError("Not all modules stop run properly!");
		}
	} else {
		// (xia_crate_.modules[module_])->run_end();
		xia::pixie::crate::module_handle module(xia_crate_, module_id);
		module->run_end();
		bool finished = false;
		for (unsigned int count = 0; count != kMaxAttempts; ++count) {
			if (!module->run_active()) {
				finished = true;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if (!finished) {
			throw UserError("Module " + std::to_string(module_id) + "did not stop run properly!\n");
		}
	}
	ExportParameters(config_path_);
}



void Crate::ReadListModeData(
	xia::pixie::crate::module_handle &module,
	std::ofstream &fout,
	unsigned int threshold
) {
	unsigned int fifo_words =
		static_cast<unsigned int>(module->read_list_mode_level());

	if (fifo_words > threshold) {
		xia::pixie::hw::words data(fifo_words);
		module->read_list_mode(data);
		fout.write(
			reinterpret_cast<char*>(data.data()),
			fifo_words*sizeof(uint32_t)
		);
	}
}

}	 // namespace rxdaq



// void Crate::PrintInfo() const {
// 	std::cout << ds(dlv::Debug, "Crate::PrintInfo()\n");
// 	std::cout << ds(dlv::Info, config_.CrateInfoStr());
// 	return;
// }



// }		// RXDAQ