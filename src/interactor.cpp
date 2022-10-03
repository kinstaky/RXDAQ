#include "include/interactor.h"

#include <csignal>
#include <iostream>
#include <thread>

#include "grpcpp/grpcpp.h"
#include "pixie/error.hpp"

#include "include/error.h"
#include "include/view.h"
#include "include/control_crate_service.h"

typedef xia::pixie::error::error XiaError;

namespace rxdaq {

//-----------------------------------------------------------------------------
//								Interactor
//-----------------------------------------------------------------------------

Interactor::Interactor(const std::string &name, const std::string &help) noexcept
: type_(InteractorType::kUndefined)
, options_("./rxdaq " + name, help) {

}

std::unique_ptr<Interactor> Interactor::CreateInteractor(const char* name) noexcept {
	std::unique_ptr<Interactor> result = nullptr;
	if (!strlen(name)) {
		result = nullptr;
	} else if (!strcmp(name, "help")) {
		result = std::make_unique<HelpCommandParser>();
	} else if (!strcmp(name, "rpc")) {
		result = std::make_unique<RpcCommandParser>();
	} else if (!strcmp(name, "boot")) {
		result = std::make_unique<BootCommandParser>();
	} else if (!strcmp(name, "task")) {
		result = std::make_unique<TaskCommandParser>();
	// } else if (!strcmp(name, "info")) {
	// 	result = std::make_unique<InfoCommandParser>();
	} else if (!strcmp(name, "read")) {
		result = std::make_unique<ReadCommandParser>();
	} else if (!strcmp(name, "write")) {
		result = std::make_unique<WriteCommandParser>();
	} else if (!strcmp(name, "import")) {
		result = std::make_unique<ImportCommandParser>();
	} else if (!strcmp(name, "export")) {
		result = std::make_unique<ExportCommandParser>();
	} else if (!strcmp(name, "run")) {
		result = std::make_unique<RunCommandParser>();
	}
	return result;
}


//-----------------------------------------------------------------------------
// 								HelpCommandParser
//-----------------------------------------------------------------------------

HelpCommandParser::HelpCommandParser() noexcept
: Interactor(CommandName(), "")
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
		"  rpc                   Run as rpc server.\n"
		"  boot                  Boot modules.\n"
		"  task                  Process task\n"
		"  read                  Read parameters.\n"
		"  write                 Write parameters.\n"
		"  import                Import parameters.\n"
		"  export                Export parameters.\n"
		"  run                   Run in list mode.\n";
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
// 								RpcCommandParser
//-----------------------------------------------------------------------------

std::unique_ptr<grpc::Server> RpcCommandParser::server_ = nullptr;

RpcCommandParser::RpcCommandParser() noexcept
: Interactor(CommandName(), "launch rpc server") {

	type_ = InteractorType::kRpcCommandParser;
	options_.add_options()
		(
			"h,host", "Set the host of server.",
			cxxopts::value<std::string>()->default_value("0.0.0.0"),
			"<address>"
		)
		(
			"p,port", "Set the port of server.",
			cxxopts::value<std::string>()->default_value("12300"),
			"<port>"
		)
		(
			"config", "Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"host_pos", "Set the host of server",
			cxxopts::value<std::string>()->default_value("0.0.0.0")
		)
		(
			"port_pos", "Set the port of server.",
			cxxopts::value<std::string>()->default_value("12300")
		);
	options_.parse_positional({"host_pos", "port_pos"});
	options_.positional_help("[host] [port]");
}


RpcCommandParser::~RpcCommandParser() {
	server_ = nullptr;
	std::cout << "delete rpc command parser" << std::endl;
}


std::string RpcCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n"
		"Examples:\n"
		"  './rxdaq rpc' to launch the rpc server.\n"
		"  './rxdaq rpc 0.0.0.0 12300' to launch the rpc server and listen\n"
		"    on 0.0.0.0:12300\n"
		"  './rxdaq rpc -h 0.0.0.0 -p 12300' to do the same thing as above.\n"
		"Remember that --config can be used to choose path of json config file.\n";
	
	return result;
}


void RpcCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	host_ = parse_result["host"].count() ?
		parse_result["host"].as<std::string>() :
		parse_result["host_pos"].as<std::string>();

	port_ = parse_result["port"].count() ?
		parse_result["port"].as<std::string>() :
		parse_result["port_pos"].as<std::string>();

	return;
}


void RpcCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize(config_path_);
	
	// service
	ControlCrateService service(crate);
	grpc::ServerBuilder builder;
	builder.AddListeningPort(
		host_ + ":" + port_,
		grpc::InsecureServerCredentials()
	);
	builder.RegisterService(&service);

	server_ = builder.BuildAndStart();
	std::cout << "Rpc server listening on " << host_ << ":" << port_ << "\n";

	// signal(SIGINT, SigIntHandler);
	server_->Wait();
}



//-----------------------------------------------------------------------------
// 								BootCommandParser
//-----------------------------------------------------------------------------

BootCommandParser::BootCommandParser() noexcept
: Interactor(CommandName(), "boot firmwares")
, config_path_("")
, module_(kModuleNum) {

	type_ = InteractorType::kBootCommandParser;
	options_.add_options()
		(
			"m,module", "Choose moudle to boot, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"<id>"
		)
		(
			"config", "Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"module_pos", "Choose module to boot, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"id"
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

	// check config
	config_path_ = parse_result["config"].as<std::string>();

	// check module
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();
	CheckModuleNumber(module_);

	return;
}


void BootCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize(config_path_);
	crate->Boot(module_, false);

	// if (module_ == kModuleNum) {
	// 	std::vector<std::thread> boot_threads;
	// 	for (unsigned short i = 0; i < crate->ModuleNum(); ++i) {
	// 		boot_threads.emplace_back(&Crate::Boot, crate, i, kEntireBoot);
	// 	}
	// 	for (auto &t : boot_threads) {
	// 		t.join();
	// 	}
	// } else {
	// 	crate->Boot(module_, kEntireBoot);
	// }
	return;
}


// //-----------------------------------------------------------------------------
// // 								TaskCommandParser
// //-----------------------------------------------------------------------------

TaskCommandParser::TaskCommandParser() noexcept
: Interactor(CommandName(), "process auto tasks")
, config_path_("config.json")
, task_name_("")
, module_(kModuleNum)
{
	type_ = InteractorType::kTaskCommandParser;
	options_.add_options()
		(
			"l,list",
			"list available tasks to process automatically.",
			cxxopts::value<bool>()
		)
		(
			"n,name",
			"Choose name of task to process.",
			cxxopts::value<std::string>()->default_value(""),
			"<task>"
		)
		(
			"m,module",
			"Choose module to process task.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"<id>"
		)
		(
			"config",
			"Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"name_pos",
			"Choose name of task to process.",
			cxxopts::value<std::string>()->default_value("")
		)
		(
			"module_pos",
			"Choose module to process task.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum))
		);
	options_.parse_positional({"name_pos", "module_pos"});
	options_.positional_help("[name] [module]");
}


std::string TaskCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n" 
		"Examples:\n"
		"  './rxdaq task' or './rxdaq task --list' to list available tasks.\n"
		"  './rxdaq task offset' to adjust DC offset in all modules.\n"
		"  './rxdaq task offset 0 to adjust DC offset in module 0.\n"
		"  or use name arguments './rxdaq task --name offset --module 0' to\n"
		"  do the same thing.\n";
	
	return result;
}


void TaskCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	task_name_ = parse_result["name"].count() ?
		parse_result["name"].as<std::string>() :
		parse_result["name_pos"].as<std::string>();
	task_name_ = parse_result["list"].count() ? "" : task_name_;
	
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();
	CheckModuleNumber(module_);
}


void TaskCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();
	if (task_name_.empty()) {
		std::cout << crate->ListTasks();
		return;
	}
	
	std::vector<unsigned short> modules =
		CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);

	for (unsigned short m : modules) {
		crate->Task(task_name_, m);
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
: Interactor(CommandName(), "read parameters")
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
			"<parameter>"
		)
		(
			"m,module",
			"Choose module to read.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"<id>"
		)
		(
			"c,channel",
			"Choose channel to read.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum)),
			"<id>"
		)
		(
			"v,verbose",
			"Read some parameter verbosely.",
			cxxopts::value<bool>()
		)
		(
			"config",
			"Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
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
	result += "\n"
		"Examples:\n"
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
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	name_ = parse_result["name"].count() ?
		parse_result["name"].as<std::string>() :
		parse_result["name_pos"].as<std::string>();
	name_ = parse_result["list"].count() ? "" : name_;
	
	module_ = parse_result["module"].count() ?
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();
	CheckModuleNumber(module_);

	channel_ = parse_result["channel"].count() ?
		parse_result["channel"].as<int>() :
		parse_result["channel_pos"].as<int>();
	CheckChannelNumber(channel_);

	verbose_ = parse_result["verbose"].count() ? true : false;
}

template <typename Value, typename VerboseValue>
std::vector<VerboseValue> GenerateVerboseValues(
	std::string parameter,
	std::vector<Value> values,
	std::vector<unsigned short> modules,
	std::vector<unsigned short> channels = std::vector<unsigned short>(1)
) {
	std::vector<VerboseValue> result;
	for (size_t i = 0; i < modules.size(); ++i) {
		for (size_t j = 0; j < channels.size(); ++j) {
			for (const auto &value :
				vparam::VerboseValues(
					parameter,
					values[i * channels.size() + j]
				)
			) {
				
				result.push_back(static_cast<VerboseValue>(value));
			}
		}
	}
	return result;
};


void ReadCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();
	// crate->Boot(module_, true);

	if (name_.empty()) {
		std::cout << crate->ListParameters(ParameterType::kAll, verbose_);
		return;
	}

	auto type = crate->CheckParameter(name_);
	if (type == ParameterType::kModule) {

		// prepare for modules to read
		auto modules = CreateRequestIndexes(kModuleNum, crate->ModuleNum(), module_);

		// read parameters
		std::vector<unsigned int> values;
		for (const auto &m : modules) {
			values.push_back(crate->ReadParameter(name_, m));
		}


		// output parameters
		if (verbose_ && vparam::Expand(name_)) {
			std::vector<std::string> names = vparam::VerboseNames(name_);
			
			if (name_ == "MODULE_CSRB") {
				std::cout << View(
					GenerateVerboseValues<unsigned int, bool>(
						name_,
						values,
						modules
					),
					names,
					modules
				);
			} else {
				std::cout << View(
					GenerateVerboseValues<unsigned int, unsigned int>(
						name_,
						values,
						modules
					),
					names,
					modules
				);
			}
		} else {
			std::cout << View(values, {name_}, modules);
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
		if (verbose_ && vparam::Expand(name_)) {
			std::vector<std::string> names = vparam::VerboseNames(name_);

			if (name_ == "CHANNEL_CSRA") {
				std::cout << View(
					GenerateVerboseValues<double, bool>(
						name_,
						values,
						modules,
						channels
					),
					names,
					modules,
					channels
				);
			} else {
				std::cout << View(
					GenerateVerboseValues<double, double>(
						name_,
						values,
						modules,
						channels
					),
					names,
					modules,
					channels
				);
			}

		} else {
			std::cout << View(values, {name_}, modules, channels);
		}
	
	} else {
		throw UserError("Invalid paraemter" + name_ + ".\n");
	}
}



//-----------------------------------------------------------------------------
// 								WriteCommandParser
//-----------------------------------------------------------------------------

WriteCommandParser::WriteCommandParser() noexcept
: Interactor(CommandName(), "write parameters")
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
			"<parameter>"
		)
		(
			"v,value",
			"Set parameter value.",
			cxxopts::value<std::string>()->default_value(""),
			"<value>"
		)
		(
			"m,module",
			"Choose module to write.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"<id>"
		)
		(
			"c,channel",
			"Choose channel to write.",
			cxxopts::value<int>()->default_value(std::to_string(kChannelNum)),
			"<id>"
		)
		(
			"config",
			"Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
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
	result += "\n"
		"Examples:\n"
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
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

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
	CheckModuleNumber(module_);

	channel_ = parse_result["channel"].count() ?
		parse_result["channel"].as<int>() :
		parse_result["channel_pos"].as<int>();
	CheckChannelNumber(channel_);
}


void WriteCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();

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


//-----------------------------------------------------------------------------
// 								ImportCommandParser
//-----------------------------------------------------------------------------

ImportCommandParser::ImportCommandParser() noexcept
: Interactor(CommandName(), "import parameters from json file")
, config_path_("config.json")
, parameter_config_path_("") {

	type_ = InteractorType::kImportCommandParser;
	options_.add_options()
		(
			"p,path",
			"Set parameters config file path.",
			cxxopts::value<std::string>()->default_value(""),
			"<file>"
		)
		(
			"config",
			"Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"path_pos",
			"Set parameters config file path.",
			cxxopts::value<std::string>()->default_value("")
		);
	options_.parse_positional({"path_pos"});
	options_.positional_help("[path]");
}


std::string ImportCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n"
		"Examples:\n"
		"  './rxdaq import params.json' to read parameters from params.json\n"
		"Remember that --config can be used to choose path of json config file.\n";

	return result;
}


void ImportCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	parameter_config_path_ = parse_result["path"].count() ?
		parse_result["path"].as<std::string>() :
		parse_result["path_pos"].as<std::string>();
}


void ImportCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();
	crate->ImportParameters(parameter_config_path_);
}


//-----------------------------------------------------------------------------
// 								ExportCommandParser
//-----------------------------------------------------------------------------

ExportCommandParser::ExportCommandParser() noexcept
: Interactor(CommandName(), "export parameters from json file")
, config_path_("config.json")
, parameter_config_path_("") {

	type_ = InteractorType::kExportCommandParser;
	options_.add_options()
		(
			"p,path",
			"Set parameters config file path.",
			cxxopts::value<std::string>()->default_value(""),
			"<file>"
		)
		(
			"config",
			"Set config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"path_pos",
			"Set parameters config file path.",
			cxxopts::value<std::string>()->default_value("")
		);
	options_.parse_positional({"path_pos"});
	options_.positional_help("[path]");
}



std::string ExportCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n"
		"Examples:\n"
		"  `./rxdaq export param.json' to export parameters to params.json\n"
		"Remember that --config can be used to choose path of json config file.\n";

	return result;
}


void ExportCommandParser::Parse(int argc, char** argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	parameter_config_path_ = parse_result["path"].count() ?
		parse_result["path"].as<std::string>() :
		parse_result["path_pos"].as<std::string>();
}


void ExportCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();	
	crate->ExportParameters(parameter_config_path_);
}


//-----------------------------------------------------------------------------
// 								RunCommandParser
//-----------------------------------------------------------------------------

RunCommandParser::RunCommandParser() noexcept
: Interactor(CommandName(), "run in list mode")
, config_path_("config.json")
, module_(kModuleNum)
, seconds_(0)
, run_(0) {

	type_ = InteractorType::kRunCommandParser;
	options_.add_options()
		(
			"m,module",
			"Set the module to run, default is all.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum)),
			"<id>"
		)
		(
			"t,time",
			"Set the run time, default is 0(infinite).",
			cxxopts::value<int>()->default_value("0"),
			"<seconds>"
		)
		(
			"r,run",
			"Set the run number, default is read from config file.",
			cxxopts::value<int>()->default_value("-1"),
			"<number>"
		)
		(
			"config",
			"Set the config file path.",
			cxxopts::value<std::string>()->default_value("config.json"),
			"<file>"
		)
		(
			"module_pos",
			"Set the module to run.",
			cxxopts::value<int>()->default_value(std::to_string(kModuleNum))
		)
		(
			"time_pos",
			"Set the run time.",
			cxxopts::value<int>()->default_value("0")
		)
		(
			"run_pos",
			"Set the run number.",
			cxxopts::value<int>()->default_value("-1")
		)
		;
	options_.parse_positional({"module_pos", "time_pos", "run_pos"});
	options_.positional_help("[module] [time] [run]");
}

std::string RunCommandParser::Help() const noexcept {
	std::string result = options_.help();
	result += "\n"
		"Examples:\n"
		"  'run' to run all modules in list mode.\n"
		"  'run 0' to run module 0 in list mode.\n"
		"  'run 13 10' to run all modules in list mode for 10 seconds.\n"
		"  'run 0 60 3' to run module 0 in list mode for 60 seconds as run 3.\n"
		"  'run -m 0 -t 60 -r 3' to do the same thing with name arguments as above.\n"
		"  'run -r 4' to run all modules in list mode as run 4.\n"
		"Press Ctrl+C to stop before reaching the finish time.\n"
		"Remember that --config can be used to choose path of json config file.\n";
	return result;
}


void RunCommandParser::Parse(int argc, char **argv) {
	auto parse_result = options_.parse(argc, argv);
	if (!parse_result.unmatched().empty()) {
		throw UserError("too many arguments");
	}

	// get parameters
	config_path_ = parse_result["config"].as<std::string>();

	module_ = parse_result["module"].count() ? 
		parse_result["module"].as<int>() :
		parse_result["module_pos"].as<int>();
	CheckModuleNumber(module_);

	seconds_ = parse_result["time"].count() ?
		parse_result["time"].as<int>() :
		parse_result["time_pos"].as<int>();

	run_ = parse_result["run"].count() ?
		parse_result["run"].as<int>() :
		parse_result["run_pos"].as<int>();
}

void RunCommandParser::Run(std::shared_ptr<Crate> crate) {
	crate->Initialize();
	crate->StartRun(module_, seconds_, run_);
}


void CheckModuleNumber(int module) {
	if (module > kModuleNum) {
		throw UserError("module should smaller than or equal to 13");
	} else if (module < 0) {
		throw UserError("module should be positive");
	}
}


void CheckChannelNumber(int channel) {
	if (channel > kChannelNum) {
		throw UserError("channel should smaller than or equal to 16");
	} else if (channel < 0) {
		throw UserError("channel should be positive");
	}
}



};	// namespace rxdaq