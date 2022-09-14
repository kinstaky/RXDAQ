#include "grpcpp/grpcpp.h"

#include "include/error.h"
#include "include/remote_crate.h"

namespace rxdaq {

template <typename Reply>
void ThrowErrors(grpc::Status &status, Reply &reply) {
	std::cerr << status.error_code() << ": " << status.error_message() << "\n";
	
	if (reply.exception_type() == ExceptionType::WARNING) {
		throw UserError(reply.message());
	} else {
		throw std::runtime_error(reply.message());
	}
}


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
	
	EmptyReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->Boot(&context, request, &reply);
	if (!status.ok()) {
		ThrowErrors(status, reply);
	}
}


unsigned int RemoteCrate::ReadParameter(
	const std::string &name,
	unsigned short module
) {
	ReadRequest request;
	request.set_name(name);
	request.set_module(uint32_t(module));
	request.set_type(uint32_t(ParameterType::kModule));
	
	ReadReply reply;
	grpc::ClientContext context;
	
	grpc::Status status = stub_->ReadParameter(&context, request, &reply);

	if (status.ok()) {
		return reply.module_value();
	} else {
		ThrowErrors(status, reply);
		return 0;
	}
}


double RemoteCrate::ReadParameter(
	const std::string &name,
	unsigned short module,
	unsigned short channel
) {
	ReadRequest request;
	request.set_name(name);
	request.set_module(module);
	request.set_channel(channel);
	request.set_type(uint32_t(ParameterType::kChannel));

	ReadReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->ReadParameter(&context, request, &reply);
	if (status.ok()) {
		return reply.channel_value();
	} else {
		ThrowErrors(status, reply);
		return 0.0;
	}
}


void RemoteCrate::WriteParameter(
	const std::string &name,
	unsigned value,
	unsigned short module
) {
	WriteRequest request;
	request.set_name(name);
	request.set_module_value(value);
	request.set_module(module);
	request.set_type(uint32_t(ParameterType::kModule));

	EmptyReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->WriteParameter(&context, request, &reply);
	if (!status.ok()) {
		ThrowErrors(status, reply);
	}
}


void RemoteCrate::WriteParameter(
	const std::string &name,
	double value,
	unsigned short module,
	unsigned short channel
) {
	WriteRequest request;
	request.set_name(name);
	request.set_channel_value(value);
	request.set_module(module);
	request.set_channel(channel);
	request.set_type(uint32_t(ParameterType::kChannel));

	EmptyReply reply;
	grpc::ClientContext context;


	grpc::Status status = stub_->WriteParameter(&context, request, &reply);
	if (!status.ok()) {
		ThrowErrors(status, reply);
	}
}


}			// namespace rxdaq

