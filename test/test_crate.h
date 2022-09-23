#ifndef __TEST_CRATE_H__
#define __TEST_CRATE_H__


#include <vector>

#include "include/crate.h"

namespace rxdaq {


/// This class is only used in test. It behaves like a crate except for
/// controling the real physical crate.
class TestCrate final: public Crate {
public:
	/// virtual module status
	enum class ModuleStatus {
		kInitial = 0,
		kBooted,
		kRunning,
		kImported,
		kExported
	};


	/// virtual module to recrod the process on modules 
	struct TestModule {
		ModuleStatus status;
		bool boot_mode;
		std::map<std::string, unsigned int> module_parameters;
		std::map<std::string, double> channel_parameters[kChannelNum];
		std::string config_file;
	};


	/// @brief constructor
	///
	TestCrate() noexcept;
	
	
	/// @brief default destructor
	///
	virtual ~TestCrate() = default;


	//-------------------------------------------------------------------------
	// 					method to get crate information
	//-------------------------------------------------------------------------

	/// @brief get module number
	///
	/// @returns module number
	///
	virtual inline unsigned short ModuleNum() const noexcept override {
		return 10;
	}

	
	//-------------------------------------------------------------------------
	// 					method to initialize and boot
	//-------------------------------------------------------------------------

	/// @brief initialize crate
	///
	virtual void Initialize(const std::string &config_path) noexcept override;


	/// @brief boot modules
	///
	/// @param module module to boot, 16 for all modules
	/// @param boot_pattern boot pattern, 0 for fast boot
	///
	virtual void Boot(
		unsigned short module,
		bool fast = true
	) noexcept override;


	//-------------------------------------------------------------------------
	//	 				method to read and write parameters
	//-------------------------------------------------------------------------

	/// @brief read module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] module modules to read
	/// @returns parameter value
	///
	virtual unsigned int ReadParameter(
		const std::string &name,
		unsigned short module
	) noexcept override;


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
	) noexcept override;


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
	) noexcept override;
	
	
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
	) noexcept override;


	/// @brief import parameters from file
	///
	/// @param[in] path parameter config file path
	/// 
	virtual void ImportParameters(const std::string &path) override;



	/// @brief export parameters to file
	///
	/// @param[in] path parameter config file path
	///
	virtual void ExportParameters(const std::string &path) override;


	/// @brief start list mode run
	///
	/// @param[in] module module to run in list mode 
	/// @param[in] seconds seconds to run, 0 for infinite time
	/// @param[in] run run number, -1 to read from config file
	///
	virtual void StartRun(
		unsigned short module,
		unsigned int seconds,
		int run
	) override;

	// virtual void EndRun(unsigned short module_) override;

	TestModule modules_[kModuleNum];
	bool list_;
	unsigned int run_time_;
	int run_number_;
};

}	// namespace rxdaq


#endif 		// __TEST_CRATE_H__