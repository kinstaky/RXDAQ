#ifndef __INTERACTOR_H__
#define __INTERACTOR_H__

#include <string>

#include "include/crate.h"
#include "cxxopts.hpp"

namespace rxdaq {

/// This class is base class of interactors. Interactors get commands or
/// requests from higher level, such as command line or rpc client, and
/// control the crate in Run() method.
class Interactor {
public:

	enum class InteractorType {
		kUndefined = 0,
		kRpcServer,
		kHelpCommandParser,
		kBootCommandParser,
		kInfoCommandParser,
		kReadCommandParser,
		kWriteCommandParser,
		kImportCommandParser,
		kExportCommandParser,
		kRunCommandParser
	};


	/// @brief create an interactor by name
	///
	/// @param[in] name type of the interactor
	/// @returns pointer to the interactor
	///
	static std::unique_ptr<Interactor> CreateInteractor(const char *name);
	
	
	/// @brief constructor
	///
	Interactor();


	/// @brief default destructor
	///
	virtual ~Interactor() = default;


	/// @brief get the type of the interactor
	///
	/// @returns interactor type
	///
	inline InteractorType Type() const {
		return type_;
	}


	/// @brief get the command name matches the Interactor
	///
	/// @returns name of the command 
	///
	inline virtual std::string CommandName() const {
		return "";
	}


	/// @brief abstract method to parse the parameters
	///
	/// @param[in] argc the number of arguments
	/// @param[in] argv the arguments
	///
	virtual void Parse(int argc, char **argv) = 0;


	/// @brief abstract method to run the interactor
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate = nullptr) = 0;


	/// @brief get the help information of this interactor
	///
	inline virtual std::string Help() const {
		return "";
	}

protected:
	InteractorType type_;
};


/// This is the base class of command parser. All subcommands from the
/// command line macthes a kind of command parser.
class CommandParser : public Interactor {
public:

	/// @brief constructor
	///
	/// @param[in] name name of the program
	/// @param[in] help description of the help information
	///
	CommandParser(const std::string &name, const std::string &help);


	/// @brief default destructor
	///
	virtual ~CommandParser() = default;


	/// @brief get the help information of this interactor
	///
	inline virtual std::string Help() const override {
		return options_.help();
	}


protected:
	cxxopts::Options options_;
};


/// This class process the subcommand "help". It parse the arguments and
/// crate a command parser matches the argument to print subcommand's help
/// information. 
class HelpCommandParser final : public CommandParser {
public:
	
	/// @brief default constructor
	///
	HelpCommandParser();


	/// @brief default destructor
	///
	virtual ~HelpCommandParser() = default;


	/// @brief parse the arguments and generate help commands
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv the arguments
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief  get the command name
	///
	/// @returns command name `help`
	///
	inline virtual std::string CommandName() const {
		return std::string("help");
	}


	/// @brief get help information
	///
	/// @return help information
	virtual std::string Help() const override;


	/// @brief run the interactor and print help information
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate = nullptr) override;

private:
	std::string help_information_;

};


/// This class processes the subcommand "boot". It boots the modules
/// in the crate.
class BootCommandParser : public CommandParser {
public:

	/// @brief constructor
	///
	BootCommandParser();


	/// @brief default destructor
	///
	~BootCommandParser() = default;


	/// @brief parse the arguments and generate help commands
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv the arguments
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief  get the command name
	///
	/// @returns command name `boot`
	///
	inline virtual std::string CommandName() const {
		return std::string("boot");
	}


	/// @brief run the interactor and boot modules
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	int module_;
	std::string config_path_;
};


// class InfoCommandParser : public CommandParser {
// public:

// 	/// @brief constructor
// 	///
// 	InfoCommandParser();

// 	/// @brief default destructor
// 	///
// 	virtual ~InfoCommandParser() = default;


// 	/// @brief parse the arguments and generate help commands
// 	///
// 	/// @param[in] argc number of arguments
// 	/// @param[in] argv the arguments
// 	///
// 	virtual void Parse(int argc, char **argv) override;
// };


}		// namespace rxdaq

#endif				// __INTERACTOR_H__