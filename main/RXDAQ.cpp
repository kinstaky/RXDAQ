#include "RXDAQ.h"

#include <exception>
#include <iostream>

namespace RXDAQ {

typedef dstring::Level dlv;

//--------------------------------------------------
//						RXDAQ
//--------------------------------------------------


RXDAQ::RXDAQ() {
	interactor = nullptr;
	crate = nullptr;
	ds.SetColorPrefix();
}


RXDAQ::~RXDAQ() {
}


void RXDAQ::AddInteractor(Interactor* interactor_) {
	interactor = interactor_;
	return;
}


void RXDAQ::AddCrate(Crate *crate_) {
	crate = crate_;
	return;
}



// run the daq
void RXDAQ::Run() {
	// check existence of Interactor
	if (!interactor) {
		std::cout << ds(dlv::Error, "Invalid interactor.\n");
		exit(-1);
	}
	if (!crate) {
		std::cout << ds(dlv::Error, "Invalid crate.\n");
		exit(-1);
	}

	// The daq get some information from the Interactor
	// set output level
	ds.SetLevel(interactor->GetOutputLevel());
	// set config file
	crate->SetConfigFile(interactor->ConfigFile());
	// set crate
	crate->SetCrateID(interactor->CrateID());

	std::cout << ds(dlv::Debug, "RXDAQ Run\n");


	// Run the Interactor. From now on, the daq do nothing and
	// the Interactor call the public method of the Crate actively.
	interactor->Run(crate);
}

}		// namespace RXDAQ