#include "main/Crate.h"

namespace RXDAQ {

class TestCrate: public Crate {
public:
	TestCrate();
	virtual ~TestCrate();

	// public method
	virtual void Init() override;
	virtual void Boot(unsigned short module_, unsigned short bootPattern_ = 0x0) override;
	virtual void ReadModuleParameter(const std::string &name, unsigned int *par, unsigned short module_) override;
	virtual void ReadChannelParameter(const std::string &name, double *par, unsigned short module_, unsigned short channel_) override;
	virtual void WriteModuleParameter(const std::string &name, unsigned int par, unsigned short module_) override;
	virtual void WriteChannelParameter(const std::string &name, double par, unsigned short module_, unsigned short channel_) override;
	virtual void Run(unsigned short module_, unsigned int time_) override;
	virtual void EndRun(unsigned short module_) override;
	virtual void ImportParameters(const std::string &path_) override;
	virtual void ExportParameters(const std::string &path_) override;
private:
};

}	// namespace RXDAQ