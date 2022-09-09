#include "test_crate.h"

#include <iostream>

namespace rxdaq {


TestCrate::TestCrate() noexcept {
	// initialize virtual modules
	for (unsigned short i = 0; i < kModuleNum; ++i) {
		modules_[i].status = ModuleStatus::kInitial;
		modules_[i].boot_pattern = 0;
	}
	list_ = false;
}


void TestCrate::Initialize(const std::string&) noexcept {
	return;
}


void TestCrate::Boot(unsigned short module, unsigned short boot_pattern) noexcept {
	modules_[module].status = ModuleStatus::kBooted;
	modules_[module].boot_pattern = boot_pattern;
	return;
}


std::string TestCrate::ListParameters(ParameterType) noexcept {
	list_ = true;
	return "";
}

unsigned int TestCrate::ReadParameter(
	const std::string &name,
	unsigned short module
) noexcept {

	return modules_[module].module_parameters[name];
}


double TestCrate::ReadParameter(
	const std::string &name,
	unsigned short module,
	unsigned short channel
) noexcept {

	return modules_[module].channel_parameters[channel][name];
}


void TestCrate::WriteParameter(
	const std::string &name,
	unsigned int value,
	unsigned short module
) noexcept {

	modules_[module].module_parameters.insert(std::make_pair(name, value));
	return;
}


void TestCrate::WriteParameter(
	const std::string &name,
	double value,
	unsigned short module,
	unsigned short channel
) noexcept {

	modules_[module].channel_parameters[channel].insert(std::make_pair(name, value));
	return;
}



// void TestCrate::Run(unsigned short module_, unsigned int time_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::Run(";
// 	if (module_ == 16) ss << "All Modules";
// 	else ss << "Module " << module_;
// 	ss << ", " << time_ << " s";
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	std::cout << "runconfig: " << runConfig.configFile << std::endl;
// 	std::cout << "data:" << std::endl;
// 	std::cout << "  path " << runConfig.dataPath << std::endl;
// 	std::cout << "  file " << runConfig.dataFileName << std::endl;
// 	std::cout << "  run  " << runConfig.run << std::endl;
// 	std::cout << runConfig.js.dump(2) << std::endl;
// 	return;
// }



// void TestCrate::EndRun(unsigned short module_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::EndRun(";
// 	if (module_ == 16) ss << "All Modules";
// 	else ss << "Module " << module_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::ImportParameters(const std::string &path) {
// 	std::stringstream ss;
// 	ss << "TestCrate::ImportParameters(";
// 	ss << path;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::ExportParameters(const std::string &path) {
// 	std::stringstream ss;
// 	ss << "TestCrate::ExportParameters(";
// 	ss << path;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


}	 	// namespace rxdaq