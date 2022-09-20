#include "include/config.h"

#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace rxdaq {

const std::string top_level_parameters[] = {
	"messageLevel",
	"crateId",
	"modules",
	"run",
	"xiaLogLevel",
	"parameterFile"
};

const std::string boot_files[] = {
	"ldr",						// dsp ldr
	"var",						// dsp var
	"fippi",					// fpga fippi
	"sys",						// fpga sys
	"version"					// version
};

const std::string firmware_versions[] = {
	"rev",						// revision
	"rate",						// sampling rate
	"bits",						// ADC bits
};

const std::string run_parameters[] = {
	"dataPath",
	"dataFile",
	"number"
};



bool CheckLogLevel(const std::string &level) {
	if (
		level == "error" ||
		level == "warning" ||
		level == "info" ||
		level == "debug"
	) {
		return true;
	}
	return false;
}


void Config::CheckTopLevelParameters() {
	for (const auto &name : top_level_parameters) {
		if (!json_.contains(name)) {
			// lack of paraemter in top level
			throw std::runtime_error(
				"Lack of parameter \"" + name + "\".\n"
			);
		}
	}
	if (!CheckLogLevel(json_["messageLevel"])) {
		throw std::runtime_error(
			"messageLevel should be one of: error, warning, info, debug.\n"
		);
	}
	if (!CheckLogLevel(json_["xiaLogLevel"])) {
		throw std::runtime_error(
			"xiaLogLevel should be one of: error, warning, info, debug.\n"
		);
	}
	if (json_["crateId"] < 0) {
		throw std::runtime_error("crateId should be positive.\n");
	}
}


void Config::CheckTemplatesParameters() {
	if (json_.contains("templates")) {
		size_t index = 0;
		for (auto &temp : json_["templates"]) {
			// check name
			if (!temp.contains("name")) {
				throw std::runtime_error(
					"Template " + std::to_string(index)
					+ " lack of parameter \"name\".\n"
				);
			}
			// check boot files
			for (auto &name : boot_files) {
				if (!temp.contains(name)) {
					// lack of parameter describes boot file
					throw std::runtime_error(
						"Template \"" + std::string(temp["name"])
						+ "\" lack of parameter \"" + name + "\".\n"
					);
				}
			}
			// check firmware version information
			for (auto &name : firmware_versions) {
				if (!temp.contains(name)) {
					// lack of parameter describes firmware version
					throw std::runtime_error(
						"Template \"" + std::string(temp["name"])
						+ "\" lack of parameter \"" + name + "\".\n"
					);
				}
			}
			// check revision
			if (temp["rev"] < 10 || temp["rev"] > 15) {
				throw std::runtime_error(
					"rev should be between 10 and 15 of template " +
					std::string(temp["name"]) + ".\n"
				);
			}
			// check firmware rate
			if (
				temp["rate"] != 100 && 
				temp["rate"] != 250 &&
				temp["rate"] != 500
			) {
				throw std::runtime_error(
					"rate should be 100, 250 or 500 of template " +
					std::string(temp["name"]) + ".\n"
				);
			}
			// check firmware bits
			if (
				temp["bits"] != 12 &&
				temp["bits"] != 14 &&
				temp["bits"] != 16
			) {
				throw std::runtime_error(
					"bits should be 12, 14 or 16 of template " +
					std::string(temp["name"]) + ".\n"
				);
			}
			++index;
		}
	}
}


void Config::CheckModulesParameters() {
	if (json_["modules"].size() > kModuleNum) {
		throw std::runtime_error("Modules size over range(1-13).\n");
	}

	// record module information
	size_t index = 0;
	for (auto &module : json_["modules"]) {
		// check slots
		if (!module.contains("slot")) {
			throw std::runtime_error(
				"Module " + std::to_string(index) +
				" lack of parameter \"slot\".\n"
			);
		}
		if (module["slot"] < 2 || module["slot"] > 14) {
			throw std::runtime_error(
				"Slot (" + module["slot"].dump() + ") of module "
				+ std::to_string(index) + " is invalid(2 - 14).\n"
			);
		}
		for (size_t i = 0; i < index; ++i) {
			if (module["slot"] == Slot(i)) {
				throw std::runtime_error(
					"Module " + std::to_string(i) + " and module "
					+ std::to_string(index) + " share the same slot "
					+ module["slot"].dump() + ".\n"
				);
			}
		}


		if (!module.contains("template")) {
			// without template, should have all parameters that a template
			// need except for "name"

			// check boot files and record them
			for (const auto &name : boot_files) {
				if (!module.contains(name)) {
					throw std::runtime_error(
						"Module " + std::to_string(index) +
						" lack of parameter \"template\" or \"" +
						name + "\".\n"
					);
				}
			}

			// check firmware version and record them
			for (const auto &name : firmware_versions) {
				// parameter missing
				if (!module.contains(name)) {
					throw std::runtime_error(
						"Module " + std::to_string(index) +
						" lack of parameter \"template\" or \"" +
						name + "\".\n"
					);
				}
			}

		} else{
			// check existence of template
			bool found = false;
			for (size_t i = 0; i < json_["templates"].size(); ++i) {
				if (json_["templates"][i]["name"] == module["template"]) {
					found = true;
					break;
				}
			}
			if (!found) {
				// template doesn't exist
				throw std::runtime_error(
					"Template \"" + std::string(module["template"]) +
					"\" of module " + std::to_string(index) +
					" doesn't exist.\n"
				);
			}
		}
		++index;
	}
}


void Config::CheckRunParameters() {
	for (const auto &name : run_parameters) {
		if (json_["run"][name].empty()) {
			throw std::runtime_error("Run lack of parameter \"" + name + "\".\n");
		}
	}
	if (RunDataPath().back() != '/') {
		SetRunDataPath(RunDataPath()+"/");
	}
}



void Config::ReadFromFile(const std::string &file_name) {
	// read from config json file
	std::ifstream fin(file_name, std::ios::in);
	if (!fin.good()) {
		throw std::runtime_error("Open file \"" + file_name + "\" failed.\n");
	}
	fin >> json_;
	fin.close();

	CheckTopLevelParameters();

	CheckTemplatesParameters();

	CheckModulesParameters();

	CheckRunParameters();

	return;
}



void Config::WriteToFile(const std::string &file_name) {
	// file to write
	std::ofstream fout(file_name);
	if (!fout.good()) {
		throw std::runtime_error("Open file \"" + file_name + "\" failed.\n");
	}

	fout << json_.dump(4) << std::endl;
	fout.close();
}


// std::string Config::CrateInformationString() const {
// 	std::stringstream ss;
// 	// print crate info
// 	ss << "--------------------Crate info--------------------\n";

// 	// table first line
// 	ss << "CrateID";
// 	for (size_t i = 0; i < 15; ++i) {
// 		ss << std::setw(4) << i;
// 	}
// 	ss << "  total" << std::endl;
// 	ss << std::setw(7) << crate_id_;


// 	unsigned int module_bit = 0;
// 	for (unsigned short i = 0; i < module_num_; ++i) {
// 		module_bit |= 1 << pxi_slot_map_[i];
// 	}
// 	for (int i = 0; i < 15; ++i) {
// 		ss << "   " << "* "[((1<<i)&module_bit) == 0];
// 	}
// 	ss << std::setw(7) << module_num_ << std::endl;

// 	// for (int i = 0; i != crateInfo.ModuleNum; ++i) {
// 	// 	ss << crateInfo.PXISlotMap[i] << "  ";
// 	// }
// 	// ss << std::endl;

// 	return ss.str();
// }

} // namespace rxdaq