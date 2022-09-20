#include "include/frame.h"

#include <exception>
#include <iostream>

#include "grpcpp/grpcpp.h"

#include "include/error.h"
#include "include/remote_crate.h"

namespace rxdaq {

//-----------------------------------------------------------------------------
//									Frame
//-----------------------------------------------------------------------------

typedef xia::pixie::error::error XiaError;

Frame::Frame()
:interactor_(nullptr), crate_(nullptr) {
}


void Frame::Parse(int argc, char **argv) {
	try {
		Parser parser;
		interactor_ = std::move(parser.Parse(argc, argv));
	} catch (const UserError &e) {
		std::cerr << "operation error: " << e.what() << std::endl;
		auto help = std::make_unique<HelpCommandParser>();
		std::cout << help->Help() << std::endl;
		exit(-2);
	} catch (const RXError &e) {
		std::cerr << "rxdaq error: " << e.what() << std::endl;
		exit(-1);
	} catch (const std::exception &e) {
		std::cerr << "fatal error: " << e.what() << std::endl;
		exit(-1);
	}
	return;
}


// run the daq
void Frame::Run() {
	// check existence of Interactor
	if (!interactor_) {
		std::cerr << "Error: Frame's interactor not found." << std::endl;
		exit(-1);
	}
	// create crate if needed
	if (
		interactor_->Type() == Interactor::InteractorType::kRpcCommandParser ||
		interactor_->Type() == Interactor::InteractorType::kUndefined
	) {	
		crate_ = std::make_shared<Crate>();
	} else if (interactor_->Type() != Interactor::InteractorType::kHelpCommandParser) {
		crate_ = std::make_shared<RemoteCrate>(
			grpc::CreateChannel("localhost:12300", grpc::InsecureChannelCredentials())
		);
	}

	try {
		// Run the Interactor. From now on, the daq do nothing and
		// the Interactor call the public method of the Crate actively.
		interactor_->Run(crate_);
	} catch (const UserError &e) {
		std::cerr << "operation error: " << e.what() << std::endl;
		exit(-2);
	} catch (const RXError &e) {
		std::cerr << "rxdaq error: " << e.what() << std::endl;
		exit(-1);
	} catch (const XiaError &e) {
		std::cerr << "xia error: " << e.result_text() << "\n"
			<< e.what() << std::endl;
		exit(-1);
	} catch (const std::exception &e) {
		std::cerr << "fatal error: " << e.what() << std::endl;
		exit(-1);
	}
}

}		// namespace rxdaq