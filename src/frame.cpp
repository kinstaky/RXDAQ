#include "include/frame.h"

#include <exception>
#include <iostream>

namespace rxdaq {

//-----------------------------------------------------------------------------
//									Frame
//-----------------------------------------------------------------------------


Frame::Frame()
:interactor_(nullptr) {
}



void Frame::SetInteractor(std::unique_ptr<Interactor> &interactor) {
	interactor_ = std::move(interactor);
	return;
}


void Frame::Parse(int argc, char **argv) {
	Parser parser;
	interactor_ = std::move(parser.Parse(argc, argv));
	return;
}


// run the daq
void Frame::Run() const {
	// check existence of Interactor
	if (!interactor_) {
		exit(-1);
	}
	// if (!crate) {
	// 	exit(-1);
	// }

	// // set config file
	// crate->SetConfigFile(interactor->ConfigFile());
	// // set crate
	// crate->SetCrateID(interactor->CrateID());

	// std::cout << ds(dlv::Debug, "RXDAQ Run\n");


	// Run the Interactor. From now on, the daq do nothing and
	// the Interactor call the public method of the Crate actively.
	// interactor_->Run(crate);
	std::cout << "running" << std::endl;
}

}		// namespace rxdaq