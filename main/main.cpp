/*
 * main.cpp
 */

#include "RXDAQ.h"

int main(int argc, char **argv) {
	RXDAQ::Crate crate;
	RXDAQ::Parser parser(argc, argv);

	RXDAQ::RXDAQ daq;
	daq.AddCrate(&crate);
	if (parser.RPC()) {
		// user rpc server as the interactor
		// not supported now
		exit(-1);
	}
	else {
		daq.AddInteractor(&parser);
	}
	daq.Run();
	return 0;
}