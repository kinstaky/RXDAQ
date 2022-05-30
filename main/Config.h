/*
 * Config.h
 */


#include "lib/nlohmann/json.hpp"
#include <string>
#include <map>

class Config {
public:
	Config();
	// Config(const Config &config);
	// const Config& operator=(const Config &config);
	virtual ~Config();

	// read config from file
	void ReadFromFile(const std::string &fileName, int cid);
	// crate information in string
	std::string CrateInfoStr() const;
	// module number
	unsigned short ModuleNum() const;
	// PXISlotMap for init system
	unsigned short* PXISlotMap();
	// firmware version
	unsigned short Rev(int index) const;
	unsigned short Rate(int index) const;
	unsigned short Bits(int index) const;
	// boot files
	std::string Ldr(int index) const;
	std::string Par(int index) const;
	std::string Var(int index) const;
	std::string Fippi(int index) const;
	std::string Sys(int index) const;
	std::string Trig(int index) const;
private:
	// maximum module number in one crate, actually is 13
	static const int MaxModule = 13;
	// parameter name in template
	static const std::string checkBootFiles[];			// boot files
	static const std::string checkFirmVers[];			// firmware version
	// parameter name in crate config
	static const std::string checkCratePars[];



	// crate information
	int crateID;
	unsigned short moduleNum;
	unsigned short pxiSlotMap[MaxModule];
	// module information
	struct ModuleInfo {
		nlohmann::json *BootTemp;							// boot template
		std::map<std::string, std::string> BootFiles;		// boot files
		unsigned short Rev;									// module revision
		unsigned short Rate;								// module sampling rate
		unsigned short Bits;								// module ADC bits
		// unsigned int ModSerNum;							// module serial number
	} moduleInfo[MaxModule];								// index of module, different from the slot id


	// module boot templates
	std::map<std::string, nlohmann::json> bootTemplates;
	// tool to read json file
	nlohmann::json js;

};