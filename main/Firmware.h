#include "lib/pixie_sdk/include/pixie/fw.hpp"


namespace RXDAQ {

class Firmware : public xia::pixie::firmware::firmware {
public:
	Firmware(const std::string &version, const int mod_revision, const int mod_adc_msps, const int mod_adc_bits, const std::string &device, const std::string &fileName);
	~Firmware();

	void load();

	bool readData;
	std::mutex lck;
};

}		// namespace RXDAQ