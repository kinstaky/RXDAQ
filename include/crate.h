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
	kModule,
	kChannel
};


/// This class represents a physical XIA crate, and provides some interface to
/// control it. So the interactors can call this methods include boot, read and
/// write parameters, get firmware information, import or export the parameters
/// of firmware and run in list mode to accquire data.
class Crate {
public:

	/// @brief default constructor
	///
	Crate();


	/// @brief default destructor
	virtual ~Crate() = default;


	// public method

	/// @brief initialize the crate
	///
	/// @param[in] config_path path of config file, "" for default
	///
	virtual void Initialize(const std::string &config_path = "");

	
	/// @brief boot modules in crate
	///
	/// @param[in] module_id module to boot
	/// @param[in] boot_pattern boot pattern, kFastBoot or kEntireBoot
	///
	virtual void Boot(unsigned short module_id, unsigned short boot_pattern = kFastBoot);
	

	// /// @brief read module parameter
	// ///
	// /// @param[in] name name of the parameter
	// /// @param[in] module modules to read
	// /// @param[out] value read value of parameter
	// ///
	// virtual void ReadModuleParameter(
	// 	const std::string &name,
	// 	unsigned short module,
	// 	unsigned int &value
	// );


	// /// @brief read channel parameters
	// ///
	// /// @param[in] name name of the parameter
	// /// @param[in] module the module to read 
	// /// @param[in] channel the channel to read
	// /// @param[out] value read value of parameter
	// ///
	// virtual void ReadChannelParameter(
	// 	const std::string &name,
	// 	unsigned short module,
	// 	unsigned short channel,
	// 	double &value
	// );


	// /// @brief write module parameter
	// ///
	// /// @param name name of the parameter
	// /// @param module module to write
	// /// @param value value to write
	// ///
	// virtual void WriteModuleParameter(
	// 	const std::string &name,
	// 	unsigned short module,
	// 	unsigned int value
	// );


	// /// @brief write channel parameter
	// ///
	// /// @param name name of the parameter
	// /// @param module module to write
	// /// @param channel channel to write
	// /// @param value value to write
	// ///
	// virtual void WriteChannelParameter(
	// 	const std::string &name,
	// 	unsigned short module,
	// 	unsigned short channel,
	// 	double value
	// );




	// virtual void StartRun(unsigned short module, unsigned int time_);
	// virtual void EndRun(unsigned short module);
	// virtual void ImportParameters(const std::string &path);
	// virtual void ExportParameters(const std::string &path);
	// virtual void PrintInfo() const;
	// virtual void SetRunConfig(const std::string &path);
	// virtual void SetRunConfig(const std::string &path, const std::string &name, unsigned int run);

	// virtual int CheckParameterName(const std::string &name);
	// virtual unsigned short ModuleNum() const;

	// virtual std::string ListParameters();

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