#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <map>

#include "nlohmann/json.hpp"

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


	/// @brief get message level
	///
	/// @returns message level
	///
	inline std::string MessageLevel() const {
		return message_level_;
	}


	/// @brief get crate id
	///
	/// @returns crate id
	///
	inline unsigned short CrateId() const {
		return crate_id_;
	}


	///	@brief get module number
	///
	/// @returns module number
	///
	inline unsigned short ModuleNum() const {
		return module_num_;
	}


	/// @brief get the physical slot by module index
	///
	/// @returns physical slot number
	///
	inline unsigned short Slot(size_t index) const {
		return module_information_[index].slot;
	}


	//-------------------------------------------------------------------------
	// 				firmware version information
	//-------------------------------------------------------------------------

	/// @brief get revision of module by index
	///
	/// @param[in] index index of module
	/// @returns revision of module
	///
	inline unsigned short Revision(size_t index) const {
		return module_information_[index].revision;
	}


	/// @brief get sampling rate of module by index
	///
	/// @param[in] index index of module
	/// @returns sampling rate of module
	///
	inline unsigned short Rate(size_t index) const {
		return module_information_[index].rate;
	}


	/// @brief get adc bits of module by index
	///
	/// @param[in] index index of module
	/// @returns adc bits of module
	///
	inline unsigned short Bits(size_t index) const {
		return module_information_[index].bits;
	}
	

	//-------------------------------------------------------------------------
	// 								boot files
	//-------------------------------------------------------------------------
	
	/// @brief get boot file ldr path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Ldr(size_t index) const {
		return GetBootFile(index, "ldr");
	}


	/// @brief get boot file par path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Par(size_t index) const {
		return GetBootFile(index, "par");
	}


	/// @brief get boot file var path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Var(size_t index) const {
		return GetBootFile(index, "var");
	}


	/// @brief get boot file fippi path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Fippi(size_t index) const {
		return GetBootFile(index, "fippi");
	}


	/// @brief get boot file sys path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Sys(size_t index) const {
		return GetBootFile(index, "sys");
	}


	/// @brief get boot file trig path by index
	///
	/// @param[in] index 
	/// @returns path of boot file
	///
	inline std::string Trig(size_t index) const {
		return GetBootFile(index, "trig");
	}


	//-------------------------------------------------------------------------
	// 								run config
	//-------------------------------------------------------------------------

	/// @brief run number
	///
	/// @returns run number
	///
	inline unsigned int RunNumber() const noexcept {
		return run_information_.run_number;
	}


	/// @brief set run number
	///
	/// @param[in] run_number run number for next run  
	///
	inline void SetRunNumber(unsigned int run_number) noexcept {
		run_information_.run_number = run_number;
	}


	/// @brief get run data path
	///
	/// @returns run data path
	/// 
	inline std::string RunDataPath() const noexcept {
		return run_information_.data_path;
	}


	/// @brief set run data path
	///
	/// @param[in] path run data path
	///
	inline void SetRunDataPath(const std::string &path) noexcept {
		run_information_.data_path = path;
	}


	/// @brief get run data file name prefix
	///
	/// @returns run data file name prefix
	/// 
	inline std::string RunDataFile() const noexcept {
		return run_information_.data_file;
	}


	/// @brief set run data file name prefix
	///
	/// @param[in] prefix run data file name prefix
	///
	inline void SetRunDataFile(const std::string &prefix) noexcept {
		run_information_.data_file = prefix;
	}
	

	// /// @brief get the crate information in string
	// ///
	// /// @returns crate information
	// ///
	// std::string CrateInformationString() const;

private:

	// module information
	struct ModuleInfo {
		unsigned short slot;								// physical slot
		nlohmann::json *boot_template;						// boot template
		std::map<std::string, std::string> boot_files;		// boot files
		unsigned short revision;							// module revision
		unsigned short rate;								// module sampling rate
		unsigned short bits;								// module ADC bits
		// unsigned int ModSerNum;							// module serial number
	};
	
	// run information
	struct RunInfo {
		unsigned int run_number;
		std::string data_path;
		std::string data_file;
	};

	/// @brief get boot file path of module by index and boot file kind
	///
	/// @param[in] index index of module
	/// @returns path of boot file
	///
	std::string GetBootFile(size_t index, const std::string &name) const;


	// parameter names to check
	// parameter names in top level
	static const std::string top_level_parameters_[];
	// parameter names in template
	static const std::string boot_files_[];
	static const std::string firmware_versions_[];
	// parameter names in crate
	static const std::string crate_parameters_[];
	// parameter names in run
	static const std::string run_parameters_[];
	
	// crate information
	std::string message_level_;
	unsigned short crate_id_;
	unsigned short module_num_;

	// module information
	ModuleInfo module_information_[kModuleNum];
	// module boot templates
	std::map<std::string, nlohmann::json> boot_templates_;

	// run information
	RunInfo run_information_;
};


}		// namespace rxdaq

#endif 		// __CONFIG_H__