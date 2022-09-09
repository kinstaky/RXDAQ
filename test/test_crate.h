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
		kRunning
	};


	/// virtual module to recrod the process on modules 
	struct TestModule {
		ModuleStatus status;
		unsigned short boot_pattern;
		std::map<std::string, unsigned int> module_parameters;
		std::map<std::string, double> channel_parameters[kChannelNum];
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
		unsigned short boot_pattern = kFastBoot
	) noexcept override;


	//-------------------------------------------------------------------------
	//	 				method to read and write parameters
	//-------------------------------------------------------------------------

	/// @brief list parameters
	///
	/// @returns list of parameters in string format
	///
	virtual std::string ListParameters(
		ParameterType type = ParameterType::kAll
	) noexcept override;


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



	// virtual void Run(unsigned short module_, unsigned int time_) override;
	// virtual void EndRun(unsigned short module_) override;
	// virtual void ImportParameters(const std::string &path_) override;
	// virtual void ExportParameters(const std::string &path_) override;

	TestModule modules_[kModuleNum];
	bool list_;
};

}	// namespace rxdaq


#endif 		// __TEST_CRATE_H__