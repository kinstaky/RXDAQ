#ifndef __CRATE_H__
#define __CRATE_H__

#include <fstream>
#include <string>
#include <mutex>

#include "pixie/pixie16/crate.hpp"
#include "nlohmann/json.hpp"

#include "include/config.h"
#include "include/message.h"

namespace rxdaq {

const unsigned short kChannelNum = 16;


const unsigned short kFastBoot = 0x4;
const unsigned short kEntireBoot = 0xf;

/// parameters type
enum class ParameterType {
	kAll = 0,
	kCrate,
	kModule,
	kChannel,
	kInvalid
};
/// parameters type - parameter type name
const std::map<ParameterType, std::string> kParameterTypeNames = {
	{ParameterType::kAll, "all"},
	{ParameterType::kCrate, "crate"},
	{ParameterType::kModule, "module"},
	{ParameterType::kChannel, "channel"},
	{ParameterType::kInvalid, "invalid"}
};


/// This class represents a physical XIA crate, and provides some interface to
/// control it. So the interactors can call this methods include boot, read and
/// write parameters, get firmware information, import or export the parameters
/// of firmware and run in list mode to accquire data.
class Crate {
public:

	/// @brief default constructor
	///
	Crate() noexcept;


	/// @brief default destructor
	virtual ~Crate() = default;


	//-------------------------------------------------------------------------
	// 					method to get crate information
	//-------------------------------------------------------------------------

	/// @brief get module number
	///
	/// @returns module number
	///
	virtual inline unsigned short ModuleNum() const noexcept {
		return config_.ModuleNum();
	}


	//-------------------------------------------------------------------------
	//	 				method to initialize and boot
	//-------------------------------------------------------------------------


	/// @brief initialize the crate
	///
	/// @param[in] config_path path of config file, "" for default
	///
	/// @throws XiaError if initialize failed
	///
	virtual void Initialize(const std::string &config_path = "");

	
	/// @brief boot modules in crate
	///
	/// @param[in] module_id module to boot
	/// @param[in] boot_pattern boot pattern, kFastBoot or kEntireBoot
	///
	/// @throws UserError if firmware version of config error
	/// @throws XiaError if boot failed
	///
	virtual void Boot(unsigned short module_id, unsigned short boot_pattern = kFastBoot);


	//-------------------------------------------------------------------------
	//	 				method to read and write parameters
	//-------------------------------------------------------------------------

	/// @brief list parameters
	///
	/// @returns list of parameters in string format
	///
	virtual std::string ListParameters(
		ParameterType type = ParameterType::kAll
	) noexcept;


	/// @brief check parameter type by name
	///
	/// @param[in] name parameter name
	/// @returns parameter type enum class
	///
	virtual ParameterType CheckParameter(const std::string &name) noexcept;
	

	/// @brief read module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] module modules to read
	/// @returns parameter value
	///
	virtual unsigned int ReadParameter(
		const std::string &name,
		unsigned short module
	);


	/// @brief read channel parameters
	///
	/// @param[in] name name of the parameter
	/// @param[in] module the module to read 
	/// @param[in] channel the channel to read
	/// @returns parameter value
	///
	virtual double ReadParameter(
		const std::string &name,
		unsigned short module,
		unsigned short channel
	);


	/// @brief write module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] value value to write
	/// @param[in] module module to write
	///
	virtual void WriteParameter(
		const std::string &name,
		unsigned int value,
		unsigned short module
	);


	/// @brief write channel parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] value value to write
	/// @param[in] module module to write
	/// @param[in] channel channel to write
	///
	virtual void WriteParameter(
		const std::string &name,
		double value,
		unsigned short module,
		unsigned short channel
	);




	// virtual void StartRun(unsigned short module, unsigned int time_);
	// virtual void EndRun(unsigned short module);
	// virtual void ImportParameters(const std::string &path);
	// virtual void ExportParameters(const std::string &path);
	// virtual void PrintInfo() const;
	// virtual void SetRunConfig(const std::string &path);
	// virtual void SetRunConfig(const std::string &path, const std::string &name, unsigned int run);



	

	// std::string configFile;
	// int crateID;

	// struct RunConfig {
	// 	std::string dataPath;
	// 	std::string dataFileName;
	// 	unsigned int run;
	// 	nlohmann::json js;
	// 	std::string configFile;
	// } runConfig;

private:
	


	// xia crate, the lower level compoment
	xia::pixie::crate::crate xia_crate_;

	// message output control
	Message message_;

	// firmwares and lock
	std::map<std::string, xia::pixie::firmware::firmware_ref> firmwares_;
	std::mutex firmwares_lock_;

	// config
	std::string config_path_;
	Config config_;

	// void readListData(xia::pixie::crate::module_handle &module_, std::ofstream &fout, unsigned int threshold);
};

}	// namespace rxdaq



#endif		// __CRATE_H__