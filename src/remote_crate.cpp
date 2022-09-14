#include "grpcpp/grpcpp.h"

#include "include/error.h"
#include "include/remote_crate.h"

namespace rxdaq {

RemoteCrate::RemoteCrate(std::shared_ptr<grpc::Channel> channel) noexcept
: stub_(ControlCrate::NewStub(channel)) {
}


void RemoteCrate::Initialize(const std::string &) {
	EmptyMessage request;
	InitializeReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->Initialize(&context, request, &reply);
	if (status.ok()) {
			module_num_ = reply.num();
	} else {
		std::cerr << "RemoteCrate: Initialize failed: " << status.error_code() << ": "
			<< status.error_message() << "\n" << reply.message() << "\n";
	}
}


void RemoteCrate::Boot(unsigned short module_id, bool fast) {
	BootRequest request;
	request.set_module(module_id);
	request.set_fast(fast);
	BootReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->Boot(&context, request, &reply);
	if (!status.ok()) {
		std::cerr << status.error_code() << ": " << status.error_message()
			<< "\n";
		
		if (reply.exception_type() == ExceptionType::WARNING) {
			throw UserError(reply.message());
		} else {
			throw std::runtime_error(reply.message());
		}
	}
}


}			// namespace rxdaq

