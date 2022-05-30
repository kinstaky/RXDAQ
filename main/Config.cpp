#include "main/Config.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>


// constructor, do nothing
Config::Config() {
}


// deconstuctor, do nothing
Config::~Config() {
}

const std::string Config::checkBootFiles[] = {
	"ldr",						// dsp ldr
	"par",						// dsp par
	"var",						// dsp var
	"fippi",					// fpga fippi
	"sys",						// fpga sys
	"trig"						// fpga trig
};

const std::string Config::checkFirmVers[] = {
	"rev",						// revision
	"rate",						// sampling rate
	"bits",						// ADC bits
};

const std::string Config::checkCratePars[] = {
	"slots",					// slot map
	"module"					// module config part
};


// Read from json file and check it
//	fileName -- config json file name
//	cid 	 -- crate id
void Config::ReadFromFile(const std::string &fileName, int cid) {
	std::ifstream fin(fileName, std::ios::in);
	if (!fin.good()) {
		throw std::runtime_error("Open file " + fileName + " failed.\n");
	}
	// read from config json file
	fin >> js;
	// check and record templates
	bootTemplates.clear();						// clear
	for (auto &it : js["template"]) {
		// check the template "name" explicitly
		if (it["name"].empty()) {
			// lack of parameter "name"
			throw std::runtime_error("template " + std::to_string(bootTemplates.size()) + " lack of parameter \"name\".\n");
		}
		for (auto &name : checkBootFiles) {
			if (it[name].empty()) {
				// lack of parameter describes boot file
				throw std::runtime_error("template " + std::string(it["name"]) + " lack of parameter \"" + name + "\".\n");
			}
		}
		for (auto &name : checkFirmVers) {
			if (it[name].empty()) {
				// lack of parameter describes firmware version
				throw std::runtime_error("template " + std::string(it["name"]) + " lack of parameter \"" + name + "\".\n");
			}
		}
		// now this template pass the completion check, add it to map
		bootTemplates.insert(make_pair(it["name"], it));
	}

	// check and record modules
	// check whether crate part exists?
	if (js["crate"].empty()) throw std::runtime_error("config file lack of \"crate\" part.\n");
	// crate part exists, check crate parameters and record it

	// crateID = -1;
	// // check all the crate id at first
	// // actually this program just care about one crate,
	// // but if we don't check the crate id in every program,
	// // none of the programs will check the crate config without "id" parameter
	// // so we check it explicitly and stupidly
	// for (auto &ic : js["crate"]) {
	// 	// check whether the parameter "id" exists in all crates
	// 	// don't care about the ugly using of the crateID (at least I can assign the corret value in the next loop)
	// 	if (ic["id"].empty()) throw std::runtime_error("crate " + std::to_string(-1 - crateID) + " lack of parameter \"id\".\n");
	// 	// now the "id" exists, check whether it's negative
	// 	// for no reason, just for convenient
	// 	if (ic["id"] < 0) throw std::runtime_error("crate " + std::to_string(-1 - crateID) + " \"id\" is negative: " + std::to_string(int(ic["id"])) + " .\n");
	// 	--crateID;
	// }
	// // clear the crateID

	crateID = cid;
	if (crateID >= int(js["crate"].size())) {
		throw std::runtime_error("crate id(" + std::to_string(crateID) + ") should smaller than " + std::to_string(js["crate"].size()) + ".\n");
	}
	// for (auto &ic : js["crate"]) {
		// // we don't care the correction of other crates' config
		// if (ic["id"] != cid) continue;

	// just care about the crate we need
	auto &ic = js["crate"][crateID];
	// check the crate parameters
	for (auto &name : checkCratePars) {
		if (ic[name].empty()) {
			throw std::runtime_error("crate " + std::to_string(crateID) + " lack of parameter \"" + name +"\".\n");
		}
	}
	// check slots size and module size
	if (ic["slots"].size() != ic["module"].size()) {
		throw std::runtime_error("crate " + std::to_string(crateID) + " slots size not equal to module size.\n");
	}
	if (ic["slots"].size() > MaxModule) {
		throw std::runtime_error("crate " + std::to_string(crateID) + " slots size too large.\n");
	}
	// pass crate parameter completion check
	moduleNum = ic["slots"].size();
	// PXISlotMap needed for init system
	for (int is = 0; is != moduleNum; ++is) {
		pxiSlotMap[is] = ic["slots"][is];
	}
	// record module information
	int is = 0;
	for (auto &im : ic["module"]) {
		moduleInfo[is].BootFiles.clear();
		// check completion of module config
		// without template, should have all parameters a template need except for "name"
		if (im["template"].empty()) {
			// clear the boot template, this module boots without template
			moduleInfo[is].BootTemp = nullptr;
			// check boot files and record them
			for (auto &name : checkBootFiles) {
				// parameter missing
				if (im[name].empty()) {
					throw std::runtime_error("module " + std::to_string(is) + " lack of parameter \"template\" or \"" + name + "\".\n");
				}
				// parameter exists, insert it to the bootFile map
				moduleInfo[is].BootFiles.insert(std::make_pair(name, im[name]));
			}

			// check firmware version and record them
			for (auto &name : checkFirmVers) {
				// parameter missing
				if (im[name].empty()) {
					throw std::runtime_error("module " + std::to_string(is) + " lack of parameter \"template\" or \"" + name + "\".\n");
				}
			}
			// all parameters describe firmware version exists, record them
			moduleInfo[is].Rev = im["rev"];
			moduleInfo[is].Bits = im["bits"];
			moduleInfo[is].Rate = im["rate"];
		} else {								// module use a template
			// check existence of template
			if (bootTemplates.find(im["template"]) == bootTemplates.end()) {
				// template doesn't exist
				throw std::runtime_error("template \"" + std::string(im["template"]) + "\" of module " + std::to_string(is) + " doesn't exist.\n");
			}
			// point to the template
			moduleInfo[is].BootTemp = &(bootTemplates[im["template"]]);
			// template exists, check whether the overwritten boot files parameters exist
			for (auto &name : checkBootFiles) {
				if (im[name].empty()) continue;
				// overwritten boot file exists
				moduleInfo[is].BootFiles.insert(std::make_pair(name, im[name]));
			}
			// copy firmware version from template
			moduleInfo[is].Rev = moduleInfo[is].BootTemp->at("rev");
			moduleInfo[is].Rate = moduleInfo[is].BootTemp->at("rate");
			moduleInfo[is].Bits = moduleInfo[is].BootTemp->at("bits");
		}
		++is;
	}
	return;
}


std::string Config::CrateInfoStr() const {
	std::stringstream ss;
	// print crate info
	ss << "--------------------Crate info--------------------" << std::endl;
	ss << "CrateID";
	for (int i = 0; i != 15; ++i) {
		ss << std::setw(4) << i;
	}
	ss << "  total" << std::endl;
	ss << std::setw(7) << crateID;
	int moduleBit = 0;
	for (int i = 0; i != moduleNum; ++i) {
		moduleBit |= 1 << pxiSlotMap[i];
	}
	for (int i = 0; i != 15; ++i) {
		ss << "   " << "* "[((1<<i)&moduleBit) == 0];
	}
	ss << std::setw(7) << moduleNum << std::endl;

	// for (int i = 0; i != crateInfo.ModuleNum; ++i) {
	// 	ss << crateInfo.PXISlotMap[i] << "  ";
	// }
	// ss << std::endl;

	return ss.str();
}


unsigned short Config::ModuleNum() const {
	return moduleNum;
}


unsigned short* Config::PXISlotMap() {
	return pxiSlotMap;
}

unsigned short Config::Rev(int index) const {
	return moduleInfo[index].Rev;
}


unsigned short Config::Rate(int index) const {
	return moduleInfo[index].Rate;
}

unsigned short Config::Bits(int index) const {
	return moduleInfo[index].Bits;
}


std::string Config::Ldr(int index) const {
	auto it = moduleInfo[index].BootFiles.find("ldr");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("ldr"));
	}
}


std::string Config::Par(int index) const {
	auto it = moduleInfo[index].BootFiles.find("par");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("par"));
	}
}


std::string Config::Var(int index) const {
	auto it = moduleInfo[index].BootFiles.find("var");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("var"));
	}
}


std::string Config::Fippi(int index) const {
	auto it = moduleInfo[index].BootFiles.find("fippi");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("fippi"));
	}
}


std::string Config::Sys(int index) const {
	auto it = moduleInfo[index].BootFiles.find("sys");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("sys"));
	}
}


std::string Config::Trig(int index) const {
	auto it = moduleInfo[index].BootFiles.find("trig");
	if (it != moduleInfo[index].BootFiles.end()) {
		return it->second;
	} else {
		return std::string(moduleInfo[index].BootTemp->at("trig"));
	}
}