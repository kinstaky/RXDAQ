#include <iostream>
#include <fstream>

#include "main/RXDAQ.h"
#include "test/TestCrate.h"


int main(int argc, char **argv) {
	RXDAQ::RXDAQ daq;
	RXDAQ::TestCrate crate;
	RXDAQ::Parser parser(argc, argv);

	daq.AddCrate(&crate);
	if (parser.RPC()) {
		std::cout << "rpc" << std::endl;
		return 0;
	} else {
		daq.AddInteractor(&parser);
	}
	daq.Run();
	return 0;
}