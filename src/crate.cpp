#include <csignal>

#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "pixie/error.hpp"

#include "include/crate.h"
#include "include/error.h"

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

std::atomic<bool> Crate::keep_running_ = false;

Crate::Crate() noexcept
: message_(Message::Level::kWarning), booted_(false), config_path_("config.json") {
	message_.SetColorfulPrefix();
}


Crate::~Crate() {
	std::cout << message_(MsgLevel::kDebug) << "Crate::~Crate()\n";
	if (booted_) {
		std::cout << message_(MsgLevel::kInfo) << "Closing modules...\n";
		
		xia_crate_.ready();
		for (unsigned short i = 0; i < ModuleNum(); ++i) {
			xia::pixie::crate::module_handle module(xia_crate_, i,
                xia::pixie::crate::module_handle::present);
			module->close();
		}
	}
}


xia::log::level GetXiaLogLevel(const std::string &level) {
	if (level == "error") {
		return xia::log::error;
	} else if (level == "warning") {
		return xia::log::warning;
	} else if (level == "info") {
		return xia::log::info;
	} else if (level == "debug") {
		return xia::log::debug;
	} else {
		return xia::log::warning;
	}
}


void Crate::Initialize(const std::string &config_path) {
	if (!config_path.empty()) {
		config_path_ = config_path;
	}
	// read config from json file
	config_.ReadFromFile(config_path_);
	message_.SetLevel(Message::ToLevel(config_.MessageLevel()));

	std::cout << message_(MsgLevel::kDebug) << "Crate::Init().\n";

	xia::logging::start("log", "Pixie16Msg.log", true);
	xia::logging::set_level(GetXiaLogLevel(config_.XiaLogLevel()));

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
				config_.Version(module_id),
				config_.Revision(module_id),
				config_.Rate(module_id),
				config_.Bits(module_id),
				device_name[i]
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


void Crate::Boot(unsigned short module_id, bool fast) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::Boot(" << module_id << ", " << fast << ").\n";

	// load firmware
	if (module_id == kModuleNum) {
		std::vector<std::thread> threads;
		for (unsigned short i = 0; i < ModuleNum(); ++i) {
			threads.emplace_back(&Crate::LoadFirmware, this, i);
		}
		for (auto &t : threads) {
			t.join();
		}
	} else {
		LoadFirmware(module_id);
	}

	// std::vector<unsigned short> modules = CreateRequestIndexes(kModuleNum, ModuleNum(), module_id);
	// for (const auto &m : modules) {
	// 	LoadFirmware(m);
	// }

	xia_crate_.ready();
	xia_crate_.set_firmware();
	xia_crate_.boot(!fast);

	ImportParameters(config_.ParameterFile());
	booted_ = true;
}



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
	xia_crate_.initialize_afe();

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


std::ofstream CreateRunDataStream(
	std::string path,
	std::string name,
	unsigned short run,
	unsigned short module
) {
	std::stringstream file_name;
	file_name << path << name << "_R" << std::setfill('0') << std::setw(4)
		<< run << "_M" << std::setfill('0') << std::setw(2) << module
		<< ".bin";

	return 
		std::ofstream(file_name.str(), std::ios::binary | std::ios::trunc);
}


std::string RunDataDirectory(
	std::string path,
	std::string name,
	unsigned int run
) {
	std::stringstream dir_name;
	dir_name << path << name << std::setfill('0') << std::setw(4) << run << "/";
	return dir_name.str();
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
			" for " + std::to_string(seconds) + " seconds" :
			""
		)
		<< ".\n"; 

	
	xia_crate_.ready();
	
	std::vector<unsigned short> modules =
		CreateRequestIndexes(kModuleNum, ModuleNum(), module_id);


	WriteParameter("SYNCH_WAIT", module_id == kModuleNum ? 1 : 0, 0);
	WriteParameter("IN_SYNCH", 0, 0);

	// create directory for data files
	std::string dir_name =
		RunDataDirectory(config_.RunDataPath(), config_.RunDataFile(), run);
	std::filesystem::create_directories(dir_name);
	// create output stream
	run_output_streams_.clear();
	for (const auto &m : modules) {
		run_output_streams_.push_back(CreateRunDataStream(
			dir_name, config_.RunDataFile(), run, m
		));
	}

	// start list mode
	for (const auto &m : modules) {
		xia_crate_.modules[m]->start_listmode(
			xia::pixie::hw::run::run_mode::new_run
		);
	}

	// get data
	keep_running_ = true;
	run_start_time_ = std::chrono::steady_clock::now();
	auto stop_time = run_start_time_;
	signal(SIGINT, SigIntHandler);
	while (
		keep_running_ && 
		(!seconds || ClockDuration(run_start_time_, stop_time) < seconds)
	) {
		for (const auto &m : modules) {
			if (xia_crate_.modules[m]->run_active()) {
				ReadListModeData(m, 131072.0*0.2);
			} else {
				std::cout << message_(MsgLevel::kInfo)
					<< "Module " << m << " has not active run.\n";
			}
		}
		stop_time = std::chrono::steady_clock::now();
	}
	signal(SIGINT, SIG_DFL);

	FinishRun(module_id);
}


void Crate::WaitFinished(const std::vector<unsigned short> &modules) {
	std::cout << message_(MsgLevel::kDebug)
		<< "Crate::WaitFinished(...),  Waiting all modules to finish...\n";

	const unsigned int max_attempts = 30;
	// check finished
	bool finished = false;
	for (unsigned int count = 0; count < max_attempts; ++count) {
		std::cout << message_(MsgLevel::kDebug)
			<< "Finish attempt " << count << "\n";

		finished = true;
		for (const auto &m : modules) {
			if (xia_crate_.modules[m]->run_active()) {
				finished = false;
			}
		}
		if (finished) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (!finished) {
		throw RXError("Not all modules stop run properly!");
	}
}



void Crate::FinishRun(unsigned short module_id) {
	std::cout << message_(MsgLevel::kDebug)
		<< "FinishRun(" << module_id << ")\n";
	std::cout << message_(MsgLevel::kInfo)
		<< "Finishing list mode run.\n";

	xia_crate_.ready();

	// stop run
	unsigned short director_module = module_id == kModuleNum ? 0 : module_id;
	xia_crate_.modules[director_module]->run_end();


	std::vector<unsigned short> modules =
		CreateRequestIndexes(kModuleNum, ModuleNum(), module_id);


	WaitFinished(modules);

	// read residual data
	for (const auto &m : modules) {
		ReadListModeData(m, 0);
	}

	// close streams
	for (auto &stream : run_output_streams_) {
		stream.close();
	}

	// display run time information
	auto stop_time = std::chrono::steady_clock::now();
	std::cout << message_(MsgLevel::kInfo)
		<< RunTimeInfo(ClockDuration(run_start_time_, stop_time));


	// export settings of this run
	ExportParameters(RunDataDirectory(
		config_.RunDataPath(), config_.RunDataFile(), config_.RunNumber()
	) + "parameters.json");

	// update run number and save
	config_.SetRunNumber(config_.RunNumber() + 1);
	config_.WriteToFile(config_path_);
}



void Crate::ReadListModeData(
	unsigned short module_id,
	unsigned int threshold
) {
	xia::pixie::crate::module_handle module(xia_crate_, module_id);
	unsigned int fifo_words =
		static_cast<unsigned int>(module->read_list_mode_level());

	if (fifo_words > threshold) {
		std::cout << message_(MsgLevel::kDebug)
			<< "ReadListModeData(" << module_id << ", " << threshold << ")\n";

		xia::pixie::hw::words data(fifo_words);
		module->read_list_mode(data);
		run_output_streams_[module_id].write(
			reinterpret_cast<char*>(data.data()),
			fifo_words*sizeof(uint32_t)
		);
	}
}


//-----------------------------------------------------------------------------
// 								related functions
//-----------------------------------------------------------------------------


std::string RunTimeInfo(unsigned int duration, int run) {
	std::string result;
	unsigned int seconds = duration;
	unsigned int minutes = seconds / 60;
	unsigned int hours = minutes / 60;
	minutes %= 60;
	seconds %= 3600;
	result = "List mode run "
		+ (run == -1 ? "\b" : std::to_string(run))
		+ " finished in "
		+ (hours ? std::to_string(hours) + ":" : "")
		+ (minutes ? std::to_string(minutes) + ":" : "")
		+ std::to_string(seconds) + "s.\n";
	return result;
}




std::vector<unsigned short> CreateRequestIndexes(
	unsigned short max_index,
	unsigned short reality_limit,
	unsigned short index
) {
	std::vector<unsigned short> result;
	if (index == max_index) {
		for (unsigned short i = 0; i < reality_limit; ++i) {
			result.push_back(i);
		}
	} else {
		result.push_back(index);
	}
	return result;
};


}	 // namespace rxdaq



// void Crate::PrintInfo() const {
// 	std::cout << ds(dlv::Debug, "Crate::PrintInfo()\n");
// 	std::cout << ds(dlv::Info, config_.CrateInfoStr());
// 	return;
// }
