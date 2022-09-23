#include "test_crate.h"

#include <iostream>

namespace rxdaq {


TestCrate::TestCrate() noexcept
: list_(false), run_time_(0), run_number_(0) {
	// initialize virtual modules
	for (unsigned short i = 0; i < kModuleNum; ++i) {
		modules_[i].status = ModuleStatus::kInitial;
		modules_[i].boot_mode = true;
		modules_[i].config_file = "";
	}
	list_ = false;
}


void TestCrate::Initialize(const std::string&) noexcept {
	return;
}


void TestCrate::Boot(unsigned short module, bool fast) noexcept {
	if (module == kModuleNum) {
		for (unsigned short i = 0; i < ModuleNum(); ++i) {
			modules_[i].status = ModuleStatus::kBooted;
			modules_[i].boot_mode = fast;
		}
	} else {
		modules_[module].status = ModuleStatus::kBooted;
		modules_[module].boot_mode = fast;
	}
	return;
}


std::string TestCrate::ListParameters(ParameterType, bool) noexcept {
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


void TestCrate::ImportParameters(const std::string &path) {
	for (unsigned short m = 0; m < ModuleNum(); ++m) {
		modules_[m].config_file = path;
		modules_[m].status = ModuleStatus::kImported;
	}
	return;
}


void TestCrate::ExportParameters(const std::string &path) {
	for (unsigned short m = 0; m < ModuleNum(); ++m) {
		modules_[m].config_file = path;
		modules_[m].status = ModuleStatus::kExported;
	}
	return;
}



void TestCrate::StartRun(unsigned short module, unsigned int seconds, int run) {
	run_time_ = seconds;
	run_number_ = run;
	if (module == kModuleNum) {
		for (unsigned short m = 0; m < ModuleNum(); ++m) {
			modules_[m].status = ModuleStatus::kRunning;
		}
	} else {
		modules_[module].status = ModuleStatus::kRunning;
	}
	return;
}



// void TestCrate::EndRun(unsigned short module_) {
// 	std::stringstream ss;
// 	ss << "TestCrate::EndRun(";
// 	if (module_ == 16) ss << "All Modules";
// 	else ss << "Module " << module_;
// 	ss << ")";
// 	std::cout << ss.str() << std::endl;
// 	return;
// }





}	 	// namespace rxdaq