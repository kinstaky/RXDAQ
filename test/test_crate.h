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
	};


	/// @brief constructor
	///
	TestCrate();
	
	
	/// @brief default destructor
	///
	virtual ~TestCrate() = default;

	
	//-------------------------------------------------------------------------
	// 						main function interface
	//-------------------------------------------------------------------------

	/// @brief initialize crate
	///
	virtual void Initialize(const std::string &config_path) override;


	/// @brief boot modules
	///
	/// @param module module to boot, 16 for all modules
	/// @param boot_pattern boot pattern, 0 for fast boot
	///
	virtual void Boot(
		unsigned short module,
		unsigned short boot_pattern = kFastBoot
	) override;


	// virtual void ReadModuleParameter(const std::string &name, unsigned int *par, unsigned short module_) override;
	// virtual void ReadChannelParameter(const std::string &name, double *par, unsigned short module_, unsigned short channel_) override;
	// virtual void WriteModuleParameter(const std::string &name, unsigned int par, unsigned short module_) override;
	// virtual void WriteChannelParameter(const std::string &name, double par, unsigned short module_, unsigned short channel_) override;
	// virtual void Run(unsigned short module_, unsigned int time_) override;
	// virtual void EndRun(unsigned short module_) override;
	// virtual void ImportParameters(const std::string &path_) override;
	// virtual void ExportParameters(const std::string &path_) override;

	TestModule modules_[kModuleNum];
};

}	// namespace rxdaq


#endif 		// __TEST_CRATE_H__