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

Interactor::Interactor()
: type_(InteractorType::kUndefined) {

}

std::unique_ptr<Interactor> Interactor::CreateInteractor(const char* name) {
	std::unique_ptr<Interactor> result = nullptr;
	if (!strlen(name)) {
		result = nullptr;
	} else if (!strcmp(name, "help")) {
		result = std::make_unique<HelpCommandParser>();
	} else if (!strcmp(name, "boot")) {
		result = std::make_unique<BootCommandParser>();
	// } else if (!strcmp(argv[1], "info")) {
	// 	result = std::make_unique<InfoCommandParser>();
	}
	return result;
}


//-----------------------------------------------------------------------------
// 								CommandParser
//-----------------------------------------------------------------------------

CommandParser::CommandParser(const std::string &name, const std::string &help)
: options_("./rxdaq " + name, help) {

}


//-----------------------------------------------------------------------------
// 								HelpCommandParser
//-----------------------------------------------------------------------------

HelpCommandParser::HelpCommandParser()
: CommandParser(CommandName(), ""), help_information_("") {
	type_ = InteractorType::kHelpCommandParser;
	options_.add_options()
		("command", "Print help for [command].", cxxopts::value<std::string>(), "pos_help");
	options_.parse_positional({"command"});
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


std::string HelpCommandParser::Help() const {
	std::string result = "Usage: rxdaq command [options]\n"
		"\n"
		"Available commands:\n"
		"  help                  Display help information.\n"
		"  boot                  Boot modules.\n";
	return result;
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

BootCommandParser::BootCommandParser()
: CommandParser(CommandName(), "boot firmwares"),
	module_(0),
	config_path_("") 
{
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
			"c, config", "Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"path"
		);
	options_.parse_positional({"module_pos"});
	options_.positional_help("[module]");
}


void BootCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// check module
	module_ = parse_result["module_pos"].as<int>();
	if (parse_result["m"].count()) {
		module_ = parse_result["m"].as<int>();
	}
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


};	// namespace rxdaq