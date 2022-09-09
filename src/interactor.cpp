#include "include/interactor.h"

#include <iostream>
#include <thread>

#include "pixie/error.hpp"

#include "include/error.h"

typedef xia::pixie::error::error XiaError;

namespace rxdaq {

//-----------------------------------------------------------------------------
//								Interactor
//-----------------------------------------------------------------------------

Interactor::Interactor() noexcept
: type_(InteractorType::kUndefined) {

}

std::unique_ptr<Interactor> Interactor::CreateInteractor(const char* name) noexcept {
	std::unique_ptr<Interactor> result = nullptr;
	if (!strlen(name)) {
		result = nullptr;
	} else if (!strcmp(name, "help")) {
		result = std::make_unique<HelpCommandParser>();
	} else if (!strcmp(name, "boot")) {
		result = std::make_unique<BootCommandParser>();
	// } else if (!strcmp(name, "info")) {
	// 	result = std::make_unique<InfoCommandParser>();
	} else if (!strcmp(name, "read")) {
		result = std::make_unique<ReadCommandParser>();
	} else if (!strcmp(name, "write")) {
		result = std::make_unique<WriteCommandParser>();
	}
	return result;
}


//-----------------------------------------------------------------------------
// 								CommandParser
//-----------------------------------------------------------------------------

CommandParser::CommandParser(const std::string &name, const std::string &help) noexcept
: options_("./rxdaq " + name, help) {

}


//-----------------------------------------------------------------------------
// 								HelpCommandParser
//-----------------------------------------------------------------------------

HelpCommandParser::HelpCommandParser() noexcept
: CommandParser(CommandName(), "")
, help_information_("") {

	type_ = InteractorType::kHelpCommandParser;
	options_.add_options()
		("command", "Print help for [command].", cxxopts::value<std::string>(), "pos_help");
	options_.parse_positional({"command"});
}


std::string HelpCommandParser::Help() const noexcept {
	std::string result = "Usage: rxdaq command [options]\n"
		"\n"
		"Available commands:\n"
		"  help                  Display help information.\n"
		"  boot                  Boot modules.\n"
		"  read                  Read parameters.\n"
		"  write                 Write parameters.\n";
	return result;
}


void HelpCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	
	// check arguments number
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get command
	std::string command = "";
	if (parse_result.count("command")) {
		command = parse_result["command"].as<std::string>();
	}
	if (command.empty()) {
		// usage of program
		help_information_ = Help();
	} else {
		// usage of command
		auto command_parser = CreateInteractor(command.c_str());
		if (!command_parser) {
			// invalid command
			throw UserError("invalid command " + command);
		}
		help_information_ = command_parser->Help();
	}
	return;
}


void HelpCommandParser::Run(std::shared_ptr<Crate> crate) {
	if (crate) {
		throw RXError("HelpCommandParser doesn't need crate");
	}
	// display help information
	std::cout << help_information_ << std::endl;
	return;
}


//-----------------------------------------------------------------------------
// 								BootCommandParser
//-----------------------------------------------------------------------------

BootCommandParser::BootCommandParser() noexcept
: CommandParser(CommandName()
, "boot firmwares")
, config_path_("")
, module_(kModuleNum) {

	type_ = InteractorType::kBootCommandParser;
	options_.add_options()
		(
			"module_pos", "Choose module to boot, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"id"
		)
		(
			"m,module", "Choose moudle to boot, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"id"
		)
		(
			"config", "Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"path"
		);
	options_.parse_positional({"module_pos"});
	options_.positional_help("[module]");
}


std::string BootCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n"
		"Examples:\n"
		"  './rxdaq boot' to boot all modules.\n"
		"  './rxdaq boot 0' to boot only module 0.(same with below command)\n"
		"  './rxdaq boot -m 0' to use options to choose module to boot.\n"
		"Remember that --config can be used to choose path of json config file.\n";
	
	return result;
}


void BootCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// check module
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();
	if (module_ > kModuleNum) {
		throw UserError("module should smaller than or equal to 13");
	} else if (module_ < 0) {
		throw UserError("module should be positive");
	}

	// check config
	config_path_ = parse_result["config"].as<std::string>();
	return;
}


void BootCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize(config_path_);
	if (module_ == kModuleNum) {
		std::vector<std::thread> boot_threads;
		// for (unsigned short i = 0; i < config.ModuleNum(); ++i) {
		for (unsigned short i = 0; i < kModuleNum; ++i) {
			boot_threads.emplace_back(&Crate::Boot, crate, i, kEntireBoot);
		}
		for (auto &t : boot_threads) {
			t.join();
		}
	} else {
		crate->Boot(module_, kEntireBoot);
	}
	return;
}


// //-----------------------------------------------------------------------------
// // 								InfoCommandParser
// //-----------------------------------------------------------------------------

// InfoCommandParser::InfoCommandParser()
// : CommandParser() {
// 	type_ = InteractorType::kInfoCommandParser;
// }



//-----------------------------------------------------------------------------
// 								ReadCommandParser
//-----------------------------------------------------------------------------

ReadCommandParser::ReadCommandParser() noexcept
: CommandParser(CommandName()
, "read parameters")
, config_path_("config.json")
, name_("")
, module_(kModuleNum)
, channel_(kChannelNum) {

	type_ = InteractorType::kReadCommandParser;
	options_.add_options()
		(
			"l,list",
			"List available parameters for reading.",
			cxxopts::value<bool>()
		)
		(
			"n,name",
			"Choose parameter name.",
			cxxopts::value<std::string>()->default_value(""),
			"parameter"
		)
		(
			"m,module",
			"Choose module to read.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"id"
		)
		(
			"c,channel",
			"Choose channel to read.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum)),
			"id"
		)
		(
			"config",
			"Set config path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"path"
		)
		(
			"name_pos",
			"Choose parameter name.",
			cxxopts::value<std::string>()->default_value("")
		)
		(
			"module_pos",
			"Choose module to read, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum))
		)
		(
			"channel_pos",
			"Choose channel to read, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum))
		);
	options_.parse_positional({"name_pos", "module_pos", "channel_pos"});
	options_.positional_help("[name] [module] [channel]");
}


std::string ReadCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "Examples:\n"
		"  './rxdaq read' or './rxdaq read --list' to list available parameters.\n"
		"  './rxdaq read MODULE_CSRA' to read parameter MODULE_CSRA from all modules.\n"
		"  './rxdaq read MODULE_CSRA 0' to read parameter MODULE_CSRA from module 0.\n"
		"    or use name arguments in command './rxdaq read -m 0 --name MODULE_CSRA'\n"
		"    to do the same thing.\n"
		"  './rxdaq read ENERGY_FLATTOP 0 16' to read channel parameter ENERGY_FLATTOP\n"
		"    from all channels in module 0.\n"
		"  './rxdaq read -n ENERGY_FLATTOP -m 0 -c 16' is the same with above command.\n"
		"Remember that --config can be used to choose path of json config file.\n";
	
	return result;
}


void ReadCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	
	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	name_ = parse_result["name"].count() ?
		parse_result["name"].as<std::string>() :
		parse_result["name_pos"].as<std::string>();
	name_ = parse_result["list"].count() ? "" : name_;
	
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();

	channel_ = parse_result["channel"].count() ?
		parse_result["channel"].as<int>() :
		parse_result["channel_pos"].as<int>();	
}



void ReadCommandParser::Run(std::shared_ptr<Crate> crate) {
	if (name_.empty()) {
		std::cout << crate->ListParameters();
		return;
	}
	auto type = crate->CheckParameter(name_);
	if (type == ParameterType::kInvalid) {
		throw UserError("Invalid paraemter" + name_ + ".\n");
	} else if (type == ParameterType::kModule) {

		// prepare for modules to read
		auto modules = CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);

		// read parameters
		std::vector<unsigned int> values;
		for (const auto &m : modules) {
			values.push_back(crate->ReadParameter(name_, m));
		}

		// output parameters
		std::cout << "module   " << name_ << "\n";		// title
		for (const auto &m : modules) {
			std::cout << std::setw(6) << m;
			std::cout << std::setw(name_.size()+3) << values[m];
			std::cout << "\n";
		}

	} else if (type == ParameterType::kChannel) {

		// prepare for modules and channels to read
		auto modules = CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);
		auto channels = CreateRequestIndexes(kChannelNum, kChannelNum, channel_);

		// read parameters
		std::vector<double> values;
		for (const auto &m : modules) {
			for (const auto &c : channels) {
				values.push_back(crate->ReadParameter(name_, m, c));
			}
		}

		// output parameters
		std::cout << "module";
		for (const auto &c : channels) {
			std::cout << std::setw(8) << "ch" + std::to_string(c);
		}
		std::cout << "\n";
		auto value_iter = values.begin();
		for (const auto &m : modules) {
			std::cout << std::setw(6) << m;
			for (size_t c = 0; c < channels.size(); ++c, ++value_iter) {
				std::cout << std::setw(8) << *value_iter;
			}
			std::cout << "\n";
		}
	}
}



//-----------------------------------------------------------------------------
// 								WriteCommandParser
//-----------------------------------------------------------------------------

WriteCommandParser::WriteCommandParser() noexcept
: CommandParser(CommandName()
, "write parameters")
, config_path_("config.json")
, name_("")
, value_("")
, module_(kModuleNum)
, channel_(kChannelNum) {

	type_ = InteractorType::kWriteCommandParser;
	options_.add_options()
		(
			"l,list",
			"List available parameters for writing.",
			cxxopts::value<bool>()
		)
		(
			"n,name",
			"Choose parameter name.",
			cxxopts::value<std::string>()->default_value(""),
			"parameter"
		)
		(
			"v,value",
			"Set parameter value.",
			cxxopts::value<std::string>()->default_value(""),
			"value"
		)
		(
			"m,module",
			"Choose module to write.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"id"
		)
		(
			"c,channel",
			"Choose channel to write.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum)),
			"id"
		)
		(
			"config",
			"Set config path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"path"
		)
		(
			"name_pos",
			"Choose parameter name.",
			cxxopts::value<std::string>()->default_value("")
		)
		(
			"value_pos",
			"Set parameter value.",
			cxxopts::value<std::string>()->default_value("")
		)
		(
			"module_pos",
			"Choose module to write, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum))
		)
		(
			"channel_pos",
			"Choose channel to write, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum))
		);
	options_.parse_positional({"name_pos", "value_pos", "module_pos", "channel_pos"});
	options_.positional_help("[name] [value] [module] [channel]");
}



std::string WriteCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "Examples:\n"
		"  './rxdaq write' or './rxdaq write --list' to list available parameters.\n"
		"  './rxdaq write MODULE_CSRA 10' to write MODULE_CSRA as 10 to all modules.\n"
		"  './rxdaq write MODULE_CSRA 10 0' to write MODULE_CSRA as 10 to module 0.\n"
		"    or use name arguments in command './rxdaq write -m 0 -n MODULE_CSRA -v 10'\n"
		"    to do the same thing.\n"
		"  './rxdaq write ENERGY_FLATTOP 0.8 0 16' to write 0.8 to channel parameter\n"
		"    ENERGY_FLATTOP to all channels in module 0.\n"
		"  './rxdaq write -n ENERGY_FLATTOP -m 0 -c 16 -v 0.8' is the same with above.\n"
		"Remember that --config can be used to choose path of json config file.\n";
	
	return result;
}


void WriteCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	
	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	name_ = parse_result["name"].count() ?
		parse_result["name"].as<std::string>() :
		parse_result["name_pos"].as<std::string>();
	name_ = parse_result["list"].count() ? "" : name_;

	if (!name_.empty() && !parse_result["value"].count()
		&& !parse_result["value_pos"].count()) {
		throw UserError("Parameter value is necessary.");
	}
	value_ = parse_result["value"].count() ? 
		parse_result["value"].as<std::string>() :
		parse_result["value_pos"].as<std::string>();
	
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();

	channel_ = parse_result["channel"].count() ?
		parse_result["channel"].as<int>() :
		parse_result["channel_pos"].as<int>();
}


void WriteCommandParser::Run(std::shared_ptr<Crate> crate) {
	if (name_.empty()) {
		std::cout << crate->ListParameters();
		return;
	}
	auto type = crate->CheckParameter(name_);
	if (type == ParameterType::kInvalid) {
		throw UserError("Invalid paraemter" + name_ + ".\n");
	} else if (type == ParameterType::kModule) {

		// prepare for modules to read
		auto modules = CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);

		// write module parameters
		for (const auto &m : modules) {
			crate->WriteParameter(name_, stoul(value_), m);
		}

	} else if (type == ParameterType::kChannel) {

		// prepare for modules and channels to read
		auto modules = CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);
		auto channels = CreateRequestIndexes(kChannelNum, kChannelNum, channel_);

		// write parameters
		for (const auto &m : modules) {
			for (const auto &c : channels) {
				crate->WriteParameter(name_, stod(value_), m, c);
			}
		}
	}
}

std::vector<unsigned short> CreateRequestIndexes(unsigned short max_index, unsigned short reality_limit, unsigned short index) {
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


};	// namespace rxdaq