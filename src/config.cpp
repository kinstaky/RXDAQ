#include "include/config.h"

#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace rxdaq {

const std::string Config::top_level_parameters_[] = {
	"messageLevel",
	"crateId",
	"modules",
	"run"
};

const std::string Config::boot_files_[] = {
	"ldr",						// dsp ldr
	"par",						// dsp par
	"var",						// dsp var
	"fippi",					// fpga fippi
	"sys",						// fpga sys
	"trig"						// fpga trig
};

const std::string Config::firmware_versions_[] = {
	"version",					// version
	"rev",						// revision
	"rate",						// sampling rate
	"bits",						// ADC bits
};

const std::string Config::run_parameters_[] = {
	"dataPath",
	"dataFile",
	"number"
};


void Config::ReadFromFile(const std::string &file_name) {
	// read from config json file
	std::ifstream fin(file_name, std::ios::in);
	if (!fin.good()) {
		throw std::runtime_error("Open file \"" + file_name + "\" failed.\n");
	}
	nlohmann::json json;
	fin >> json;

	// check top level parameters
	for (const auto &name : top_level_parameters_) {
		if (json[name].empty()) {
			// lack of paraemter in top level
			throw std::runtime_error(
				"Lack of parameter \"" + name + "\".\n"
			);
		}
	}
	message_level_ = json["messageLevel"];
	crate_id_ = json["crateId"];


	// check and record templates
	boot_templates_.clear();
	for (auto &iter : json["templates"]) {
		// check the template "name" explicitly
		if (iter["name"].empty()) {
			// lack of parameter "name"
			throw std::runtime_error(
				"Template " + std::to_string(boot_templates_.size())
				+ " lack of parameter \"name\".\n"
			);
		}
		for (auto &name : boot_files_) {
			if (iter[name].empty()) {
				// lack of parameter describes boot file
				throw std::runtime_error(
					"Template \"" + std::string(iter["name"])
					+ "\" lack of parameter \"" + name + "\".\n"
				);
			}
		}
		for (auto &name : firmware_versions_) {
			if (iter[name].empty()) {
				// lack of parameter describes firmware version
				throw std::runtime_error(
					"Template \"" + std::string(iter["name"])
					+ "\" lack of parameter \"" + name + "\".\n"
				);
			}
		}
		// now this template pass the completion check, add it to map
		boot_templates_.insert(make_pair(iter["name"], iter));
	}


	// check modules
	module_num_ = json["modules"].size();
	if (module_num_ > kModuleNum) {
		throw std::runtime_error("Modules size over range(1-13).\n");
	}

	// record module information
	int is = 0;
	for (auto &im : json["modules"]) {
		module_information_[is].boot_files.clear();
		// check completion of module config
		if (im["slot"].empty()) {
			// without slot
			throw std::runtime_error("Module " + std::to_string(is) + " lack of parameter \"slot\".\n");
		}
		// check slots
		if (im["slot"] < 2 || im["slot"] > 14) {
			throw std::runtime_error(
				"Slot (" + im["slot"].dump() + ") of module "
				+ std::to_string(is) + " is invalid(2 - 14).\n"
			);
		}
		for (int i = 0; i < is; ++i) {
			if (im["slot"] == module_information_[i].slot) {
				throw std::runtime_error(
					"Module " + std::to_string(i) + " and module "
					+ std::to_string(is) + " share the same slot "
					+ im["slot"].dump() + ".\n"
				);
			}
		}
		module_information_[is].slot = im["slot"];


		if (im["template"].empty()) {
			// without template, should have all parameters a template need except for "name"

			// clear the boot template, this module boots without template
			module_information_[is].boot_template = nullptr;
	
			// check boot files and record them
			for (const auto &name : boot_files_) {
				// parameter missing
				if (im[name].empty()) {
					throw std::runtime_error("Module " + std::to_string(is) + " lack of parameter \"template\" or \"" + name + "\".\n");
				}
				// parameter exists, insert it to the bootFile map
				module_information_[is].boot_files.insert(std::make_pair(name, im[name]));
			}

			// check firmware version and record them
			for (const auto &name : firmware_versions_) {
				// parameter missing
				if (im[name].empty()) {
					throw std::runtime_error("Module " + std::to_string(is) + " lack of parameter \"template\" or \"" + name + "\".\n");
				}
			}
			// all parameters describe firmware version exists, record them
			module_information_[is].version = im["veresion"];
			module_information_[is].revision = im["rev"];
			module_information_[is].bits = im["bits"];
			module_information_[is].rate = im["rate"];
		} else {								// module use a template
			// check existence of template
			if (boot_templates_.find(im["template"]) == boot_templates_.end()) {
				// template doesn't exist
				throw std::runtime_error("Template \"" + std::string(im["template"]) + "\" of module " + std::to_string(is) + " doesn't exist.\n");
			}
			// point to the template
			module_information_[is].boot_template = &(boot_templates_[im["template"]]);
			// template exists, check whether the overwritten boot files parameters exist
			for (const auto &name : boot_files_) {
				if (im[name].empty()) continue;
				// overwritten boot file exists
				module_information_[is].boot_files.insert(std::make_pair(name, im[name]));
			}
			// copy firmware version from template
			module_information_[is].revision = module_information_[is].boot_template->at("rev");
			module_information_[is].rate = module_information_[is].boot_template->at("rate");
			module_information_[is].bits = module_information_[is].boot_template->at("bits");
		}
		++is;
	}


	// check run config
	for (const auto &name : run_parameters_) {
		if (json["run"][name].empty()) {
			throw std::runtime_error("Run lack of parameter \"" + name + "\".\n");
		}
	}
	run_information_ = {
		json["run"]["number"],
		json["run"]["dataPath"],
		json["run"]["dataFile"]
	};
	if (run_information_.data_path.back() != '/') {
		run_information_.data_path += "/";
	}

	return;
}



void Config::WriteToFile(const std::string &file_name) {
	// file to write
	std::ofstream fout(file_name);
	if (!fout.good()) {
		throw std::runtime_error("Open file \"" + file_name + "\" failed.\n");
	}

	nlohmann::json json;
	json["messageLevel"] = message_level_;
	json["crateId"] = crate_id_;
	for (const auto &[key, value] : boot_templates_) {
		json["templates"].push_back(value);
	}
	for (unsigned short i = 0; i < module_num_; ++i) {
		ModuleInfo &info = module_information_[i];
		nlohmann::json module;
		module["slot"] = info.slot;
		if (info.boot_template) {
			module["template"] = info.boot_template->at("name"); 
		}
		for (const auto &[key, file] : info.boot_files) {
			module[key] = file;
		}
		json["modules"].push_back(module);
	}
	json["run"]["dataPath"] = run_information_.data_path;
	json["run"]["dataFile"] = run_information_.data_file;
	json["run"]["number"] = run_information_.run_number;

	fout << json.dump(4) << std::endl;
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

std::string Config::GetBootFile(size_t index, const std::string &name) const {
	auto search = module_information_[index].boot_files.find(name);
	if (search != module_information_[index].boot_files.end()) {
		return search->second;
	} else {
		return std::string(module_information_[index].boot_template->at(name));
	}
}


} // namespace rxdaq