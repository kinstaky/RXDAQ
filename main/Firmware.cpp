#include "Firmware.h"

namespace RXDAQ {

Firmware::Firmware(const std::string &version, const int mod_revision, const int mod_adc_msps, const int mod_adc_bits, const std::string &device, const std::string &fileName)
:firmware(version, mod_revision, mod_adc_msps, mod_adc_bits, device), readData(false) {
	filename = fileName;
}

Firmware::~Firmware() {
}

void Firmware::load() {
	lck.lock();
	if (!readData) {
		firmware::load();
		readData = true;
	}
	lck.unlock();
	return;
}


}		// namespace RXDAQ