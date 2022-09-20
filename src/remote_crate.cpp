#include <signal.h>
#include <stdint.h>

#include "grpcpp/grpcpp.h"

#include "include/error.h"
#include "include/remote_crate.h"

namespace rxdaq {

RemoteCrate *RemoteCrate::instance_ = nullptr;

template <typename Reply>
void ThrowErrors(grpc::Status &status, Reply &reply) {
	std::cerr << "rpc status not ok: error code "
		<< status.error_code() << ": " << status.error_message() << "\n";
	
	if (reply.exception_type() == ExceptionType::WARNING) {
		throw UserError(reply.message());
	} else {
		throw std::runtime_error(reply.message());
	}
}


RemoteCrate::RemoteCrate(std::shared_ptr<grpc::Channel> channel) noexcept
: stub_(ControlCrate::NewStub(channel)) {
	instance_ = this;
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


void RemoteCrate::ImportParameters(const std::string &path) {
	ImportExportRequest request;
	request.set_path(path);

	EmptyReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->ImportParameters(&context, request, &reply);

	if (!status.ok()) {
		ThrowErrors(status, reply);
	}
}


void RemoteCrate::ExportParameters(const std::string &path) {
	ImportExportRequest request;
	request.set_path(path);

	EmptyReply reply;
	grpc::ClientContext context;

	grpc::Status status = stub_->ExportParameters(&context, request, &reply);

	if (!status.ok()) {
		ThrowErrors(status, reply);
	}
}


void RemoteCrate::StartRun(
	unsigned short module_id,
	unsigned int seconds,
	int run
) {
	RunRequest request;
	request.set_module(module_id);
	request.set_seconds(seconds);
	request.set_run_number(run);

	AsyncClientCall *call = new AsyncClientCall;
	call->type = 1;
	
	call->reader = stub_->PrepareAsyncStartRun(
		&call->context, request, &completion_queue_
	);
	call->reader->StartCall();
	call->reader->Finish(&call->reply, &call->status, (void*)call);

	void *got_tag;
	bool ok = false;
	signal(SIGINT, SigIntHandler);
	while (completion_queue_.Next(&got_tag, &ok)) {
		GPR_ASSERT(ok);
		AsyncClientCall *call = static_cast<AsyncClientCall*>(got_tag);
		if (!call->status.ok()) {
			grpc::Status status = call->status;
			RunReply reply = call->reply;
			delete call;
			ThrowErrors(status, reply);
		}
		if (call->type == 1) {
			// exit the loop
			signal(SIGINT, SIG_DFL);
			std::cout << RunTimeInfo(call->reply.seconds(), call->reply.run_number())
				<< std::endl;
			delete call;
			return;
		} else {
			std::cout << "Stop run success." << std::endl;
			delete call;
		}
	}

} 


void RemoteCrate::StopRun() {
	EmptyMessage request;

	AsyncClientCall *call = new AsyncClientCall;
	call->type = 2;

	call->reader = stub_->PrepareAsyncStopRun(
		&call->context, request, &completion_queue_
	);
	call->reader->StartCall();
	call->reader->Finish(&call->reply, &call->status, (void*)call);
}


void RemoteCrate::SigIntHandler(int) {
	std::cout << "\nYou press Ctrl+C to stop list mode run." << std::endl;
	instance_->StopRun();
}


}			// namespace rxdaq

