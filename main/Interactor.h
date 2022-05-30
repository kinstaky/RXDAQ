#ifndef __INTERACTOR_H__
#define __INTERACTOR_H__

#include "dstring.h"
#include "Crate.h"

#include <string>

namespace RXDAQ {

class Interactor {
public:
	virtual ~Interactor();

	virtual void Run(Crate *crate) = 0;
	virtual void SetConfigFile(const char *path);
	virtual const char *ConfigFile();
	virtual dstring::Level GetOutputLevel() const;

	// public method
	virtual unsigned short CrateID() const = 0;


protected:
	Interactor();

	dstringPtr ds;
	std::string configFileName;
};


class Parser: public Interactor {
public:
	Parser(int argc, char **argv);
	virtual ~Parser();

	// public method
	void Run(Crate *crate);
	bool RPC();
	dstring::Level GetOutputLevel() const;
	unsigned short CrateID() const;
protected:
	void parse(int argc, char **argv);


	// the commands
	bool boot;
	bool read;
	bool write;
	bool rpc;
	bool run;
	bool exportParam;
	bool importParam;
	bool info;

	// the options
	dstring::Level level;
	unsigned short crate;
	unsigned short module;
	unsigned short channel;
	bool list;
	std::string name;
	std::string value;
	int time;
	std::string path;
	bool runConfig;
	unsigned int runNumber;
	std::string runDataPath;
	std::string runDataFile;
};


}		// namespace RXDAQ
#endif