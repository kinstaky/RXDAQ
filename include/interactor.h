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
		kHelpCommandParser,
		kRpcCommandParser,
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
	static std::unique_ptr<Interactor> CreateInteractor(const char *name) noexcept;
	
	
	/// @brief constructor
	///
	/// @param[in] name name of the program
	/// @param[in] help description of the help information
	///
	Interactor(const std::string &name, const std::string &help) noexcept;


	/// @brief default destructor
	///
	virtual ~Interactor() = default;


	/// @brief get the type of the interactor
	///
	/// @returns interactor type
	///
	inline InteractorType Type() const noexcept {
		return type_;
	}


	/// @brief get the command name matches the Interactor
	///
	/// @returns name of the command 
	///
	inline virtual std::string CommandName() const noexcept {
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
	/// @returns empty string
	///
	inline virtual std::string Help() const noexcept {
		return options_.help();
	}

protected:
	InteractorType type_;
	cxxopts::Options options_;
};



/// This class process the subcommand "help". It parse the arguments and
/// crate a command parser matches the argument to print subcommand's help
/// information. 
class HelpCommandParser final : public Interactor {
public:
	
	/// @brief default constructor
	///
	HelpCommandParser() noexcept;


	/// @brief default destructor
	///
	virtual ~HelpCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'help'
	///
	inline virtual std::string CommandName() const noexcept override {
		return std::string("help");
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and generate help commands
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv the arguments
	///
	/// @throws UserError: invalid arguments number or invalid command
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and print help information
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate = nullptr) override;

private:
	std::string help_information_;

};


class RpcCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	RpcCommandParser() noexcept;


	/// @brief default destructor
	///
	~RpcCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'rpc'
	///
	inline virtual std::string CommandName() const noexcept override {
		return std::string("rpc");
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get boot information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv the arguments
	///
	/// @throws UserError: unmatched arguments or invalid module id
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and boot modules
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	std::string host_;
	std::string port_;
};


/// This class processes the subcommand "boot". It boots the modules
/// in the crate.
class BootCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	BootCommandParser() noexcept;


	/// @brief default destructor
	///
	~BootCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'boot'
	///
	inline virtual std::string CommandName() const noexcept override {
		return std::string("boot");
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get boot information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv the arguments
	///
	/// @throws UserError: unmatched arguments or invalid module id
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and boot modules
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	int module_;
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


/// This class parse the options of subcommand read and try to read parameters.
class ReadCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	ReadCommandParser() noexcept;
	
	
	/// @brief default destructor
	///
	~ReadCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'boot'
	///
	inline virtual std::string CommandName() const noexcept override {
		return "read";
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get read information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv arguments list
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and read parameters
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	std::string name_;
	int module_;
	int channel_;
};



/// This class parse the options of subcommand write and try to write parameters.
class WriteCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	WriteCommandParser() noexcept;
	
	
	/// @brief default destructor
	///
	~WriteCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'boot'
	///
	inline virtual std::string CommandName() const noexcept override {
		return "write";
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get read information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv arguments list
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and read parameters
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	std::string name_;
	std::string value_;
	int module_;
	int channel_;
};


/// This class parse the options of subcommand import and import the module
/// and channel parameters from file.
class ImportCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	ImportCommandParser() noexcept;


	/// @brief default destructor
	///
	virtual ~ImportCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'boot'
	///
	inline virtual std::string CommandName() const noexcept override {
		return "import";
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get read information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv arguments list
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and read parameters
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	std::string parameter_config_path_;
};


/// This class parse the options of subcommand export and export the module
/// and channel parameters to file.
class ExportCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	ExportCommandParser() noexcept;


	/// @brief default destructor
	///
	virtual ~ExportCommandParser() = default;


	/// @brief  get the command name
	///
	/// @returns command name 'boot'
	///
	inline virtual std::string CommandName() const noexcept override {
		return "export";
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get read information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv arguments list
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and read parameters
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	std::string parameter_config_path_;

};



/// This class parse the options of subcommand run and run in list mode.
class RunCommandParser : public Interactor {
public:

	/// @brief constructor
	///
	RunCommandParser() noexcept;


	/// @brief default destructor
	///
	virtual ~RunCommandParser() = default;


	/// @brief get command name
	///
	/// @returns command name 'run'
	///
	inline virtual std::string CommandName() const noexcept override {
		return "run";
	}


	/// @brief get help information
	///
	/// @returns help information
	///
	virtual std::string Help() const noexcept override;


	/// @brief parse the arguments and get read information
	///
	/// @param[in] argc number of arguments
	/// @param[in] argv arguments list
	///
	virtual void Parse(int argc, char **argv) override;


	/// @brief run the interactor and read parameters
	///
	/// @param[in] crate pointer to crate object
	///
	virtual void Run(std::shared_ptr<Crate> crate) override;

private:
	std::string config_path_;
	int module_;
	int seconds_;
	int run_;
};


/// @brief create vector of indexes for modules or channels
///
/// @param[in] max_index maximum index of the vector 
/// @param[in] reality_limit reality limit of the vector size
/// @param[in] index if index is equal to max_index, create the list from 0
///		to reality limit, otherwise create a vector with only this index 
/// @returns vector of the indexes
///
/// @relates CommandParser
///
std::vector<unsigned short> CreateRequestIndexes(unsigned short max_index, unsigned short reality_limit, unsigned short index);


}		// namespace rxdaq

#endif				// __INTERACTOR_H__