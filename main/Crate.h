#ifndef __CRATE_H__
#define __CRATE_H__

#include <fstream>
#include <string>
#include <mutex>

#include "lib/pixie_sdk/include/pixie/pixie16/crate.hpp"
#include "lib/pixie_sdk/include/pixie/fw.hpp"
#include "lib/nlohmann/json.hpp"
#include "Config.h"
#include "dstring.h"
#include "Firmware.h"



namespace RXDAQ {

const int TypeAllParameter = 0;
const int TypeModuleParameter = 1;
const int TypeChannelParameter = 2;

class Crate {
public:
	Crate();
	virtual ~Crate();

	// settings
	virtual void SetConfigFile(const char *path);
	virtual void SetCrateID(unsigned short cid);
	// public method
	virtual void Init();
	virtual void Boot(unsigned short module_, unsigned short bootPattern_ = 0x0);
	virtual void ReadModuleParameter(const std::string &name, unsigned int *par, unsigned short module);
	virtual void ReadChannelParameter(const std::string &name, double *par, unsigned short module, unsigned short channel);
	virtual void WriteModuleParameter(const std::string &name, unsigned int par, unsigned short module);
	virtual void WriteChannelParameter(const std::string &name, double par, unsigned short module, unsigned short channel);
	virtual std::string ListParameters(int type = TypeAllParameter);
	virtual void Run(unsigned short module_, unsigned int time_);
	virtual void EndRun(unsigned short module_);
	virtual void ImportParameters(const std::string &path);
	virtual void ExportParameters(const std::string &path);
	virtual void PrintInfo() const;
	virtual void SetRunConfig(const std::string &path);
	virtual void SetRunConfig(const std::string &path, const std::string &name, unsigned int run);

	virtual int CheckParameterName(const std::string &name);
	virtual unsigned short ModuleNum() const;
protected:
	dstringPtr ds;
	Config config;

	std::string configFile;
	int crateID;

	struct RunConfig {
		std::string dataPath;
		std::string dataFileName;
		unsigned int run;
		nlohmann::json js;
		std::string configFile;
	} runConfig;

private:
	// parts of the crate
	xia::pixie::crate::crate xiaCrate;

	std::map<std::string, std::shared_ptr<Firmware>> firmwares;

	// lock
	std::mutex lck;

	void bootOneModule(unsigned short module_, unsigned short bootPattern);
	void readListData(xia::pixie::crate::module_handle &module_, std::ofstream &fout, unsigned int threshold);
};

}	// namespace RXDAQ



#endif