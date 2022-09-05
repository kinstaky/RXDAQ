#include "test_crate.h"

#include <iostream>

namespace rxdaq {


TestCrate::TestCrate() {
	// initialize virtual modules
	for (unsigned short i = 0; i < kModuleNum; ++i) {
		modules_[i].status = ModuleStatus::kInitial;
		modules_[i].boot_pattern = 0;
	}
}


void TestCrate::Initialize(const std::string&) {
	return;
}


void TestCrate::Boot(unsigned short module, unsigned short boot_pattern) {
	std::cout << "boot " << module << " pattern " << boot_pattern << std::endl;
	modules_[module].status = ModuleStatus::kBooted;
	modules_[module].boot_pattern = boot_pattern;
	return;
}



// void TestCrate::Init() {
// 	std::cout << "TestCrate::Init()" << std::endl;
// 	config.ReadFromFile(configFile, crateID);
// 	return;
// }

// void TestCrate::Boot(unsigned short module_, unsigned short bootPattern_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::Boot(";
// 	if (module_ == 16) ss << "All Modules";
// 	else ss << "Module " << module_;
// 	ss << ", 0x" << std::hex << bootPattern_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::ReadModuleParameter(const std::string &name, unsigned int *par, unsigned short module_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::ReadModuleParameter(" << name;
// 	if (module_ == 16) ss << ", All Modules";
// 	else ss << ", Module " << module_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::ReadChannelParameter(const std::string &name, double *par, unsigned short module_, unsigned short channel_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::ReadChannelParameter(" << name;
// 	if (module_ == 16) ss <<", All Modules";
// 	else ss << ", Module " << module_;
// 	if (channel_ == 16) ss << ", All Channels";
// 	else ss << ", Channel " << channel_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::WriteModuleParameter(const std::string &name, unsigned int par, unsigned short module_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::WriteModuleParameter(" << name;
// 	ss << ", " << std::hex << par;
// 	if (module_ == 16) ss << ", All Modules";
// 	else ss << ", Module " << module_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


// void TestCrate::WriteChannelParameter(const std::string &name, double par, unsigned short module_, unsigned short channel_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::WriteChannelParameter(" << name;
// 	ss << ", " << par;
// 	if (module_ == 16) ss << ", All Modules";
// 	else ss << ", Module " << module_;
// 	if (channel_ == 16) ss << ", All Channels";
// 	else ss << ", Channel " << channel_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }


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