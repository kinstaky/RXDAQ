#ifndef __CONFIG_H__
#define __CONFIG_H__

// #include <unistd.h>
#include <string>
#include <map>

#include "nlohmann/json.hpp"

#include "include/error.h"

const unsigned short kModuleNum = 13;

namespace rxdaq {

/// This class reads the json format config file for a crate. 
class Config {
public:

	/// @brief default constructor
	///
	Config() = default;


	/// @brief default destructor
	///
	~Config() = default;


	/// @brief read config from file
	///
	/// @param[in] file_name name of the config file
	///
	void ReadFromFile(const std::string &file_name);


	/// @brief write config to file
	///
	/// @param[in] file_name name of the config file
	///
	void WriteToFile(const std::string &file_name);


	//-------------------------------------------------------------------------
	// 							global configuration
	//-------------------------------------------------------------------------

	/// @brief get message level
	///
	/// @returns message level
	///
	inline std::string MessageLevel() const {
		return json_["messageLevel"];
	}


	/// @brief get xia log level
	///
	/// @returns xia log level in string
	///
	inline std::string XiaLogLevel() const {
		return json_["xiaLogLevel"];
	}

	//-------------------------------------------------------------------------
	// 							crate configuration
	//-------------------------------------------------------------------------

	///	@brief get module number
	///
	/// @returns module number
	///
	inline unsigned short ModuleNum() const {
		return json_["modules"].size();
	}


	/// @brief get the physical slot by module index
	///
	/// @returns physical slot number
	///
	inline unsigned short Slot(size_t index) const {
		return json_["modules"][index]["slot"];
	}


	/// @brief get crate id
	///
	/// @returns crate id
	///
	inline unsigned short CrateId() const {
		return json_["crateId"];
	}


	/// @brief get path of parameter setting file
	///
	/// @returns path of parameter setting file
	///
	inline std::string ParameterFile() const {
		return json_["parameterFile"];
	}


	//-------------------------------------------------------------------------
	// 					firmware version information
	//-------------------------------------------------------------------------


	/// @brief get revision of module by index
	///
	/// @param[in] index index of module
	/// @returns revision of module
	///
	inline unsigned short Revision(size_t index) const {
		return GetBootInfo<unsigned short>(index, "rev");
	}


	/// @brief get sampling rate of module by index
	///
	/// @param[in] index index of module
	/// @returns sampling rate of module
	///
	inline unsigned short Rate(size_t index) const {
		return GetBootInfo<unsigned short>(index, "rate");
	}


	/// @brief get adc bits of module by index
	///
	/// @param[in] index index of module
	/// @returns adc bits of module
	///
	inline unsigned short Bits(size_t index) const {
		return GetBootInfo<unsigned short>(index, "bits");
	}
	

	//-------------------------------------------------------------------------
	// 								boot files
	//-------------------------------------------------------------------------

	/// @brief get boot information of module by index
	///
	/// @tparam 
	/// @param[in] index index of module
	/// @returns path of boot file
	///
	template <typename ReturnType>
	ReturnType GetBootInfo(size_t index, const std::string &name) const {
		if (json_["modules"][index].contains(name)) {
			return json_["modules"][index][name];
		} else {
			for (size_t i = 0; i < json_["templates"].size(); ++i) {
				if (
					json_["templates"][i]["name"] == 
					json_["modules"][index]["template"]
				) {
					return json_["templates"][i][name];
				}
			}
			throw RXError("Should not be here in GetBootFile.");
		}
	}


	/// @brief get the boot file version
	///
	/// @param[in] index index of module
	/// @returns version of boot file
	///
	inline std::string Version(size_t index) const {
		return GetBootInfo<std::string>(index, "version");
	}

	
	/// @brief get boot file ldr path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Ldr(size_t index) const {
		return GetBootInfo<std::string>(index, "ldr");
	}


	/// @brief get boot file var path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Var(size_t index) const {
		return GetBootInfo<std::string>(index, "var");
	}


	/// @brief get boot file fippi path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Fippi(size_t index) const {
		return GetBootInfo<std::string>(index, "fippi");
	}


	/// @brief get boot file sys path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Sys(size_t index) const {
		return GetBootInfo<std::string>(index, "sys");
	}


	//-------------------------------------------------------------------------
	// 								run config
	//-------------------------------------------------------------------------

	/// @brief run number
	///
	/// @returns run number
	///
	inline unsigned int RunNumber() const noexcept {
		return json_["run"]["number"];
	}


	/// @brief set run number
	///
	/// @param[in] run_number run number for next run  
	///
	inline void SetRunNumber(unsigned int run_number) noexcept {
		json_["run"]["number"] = run_number;
	}


	/// @brief get run data path
	///
	/// @returns run data path
	/// 
	inline std::string RunDataPath() const noexcept {
		return json_["run"]["dataPath"];
	}


	/// @brief set run data path
	///
	/// @param[in] path run data path
	///
	inline void SetRunDataPath(const std::string &path) noexcept {
		json_["run"]["dataPath"] = path;
	}


	/// @brief get run data file name prefix
	///
	/// @returns run data file name prefix
	/// 
	inline std::string RunDataFile() const noexcept {
		return json_["run"]["dataFile"];
	}


	/// @brief set run data file name prefix
	///
	/// @param[in] prefix run data file name prefix
	///
	inline void SetRunDataFile(const std::string &prefix) noexcept {
		json_["run"]["dataFile"] = prefix;
	}
	

	// /// @brief get the crate information in string
	// ///
	// /// @returns crate information
	// ///
	// std::string CrateInformationString() const;

private:

	/// @brief check top level parameters
	///
	/// @throws runtime_error if lack of parameter or invalid value
	///
	void CheckTopLevelParameters();


	/// @brief check parameters of template
	///
	/// @throws runtime_error if lack of parameter or invalid value
	///
	void CheckTemplatesParameters();


	/// @brief check parameters of module
	///
	/// @throws runtime_error if lack of parameter or invalid value
	///
	void CheckModulesParameters();


	/// @brief check parameters for run
	///
	/// @throws runtime_error if lack of parameter or invalid value
	///
	void CheckRunParameters();

	// json data
	nlohmann::json json_;
};


}		// namespace rxdaq

#endif 		// __CONFIG_H__