#include "include/control_crate_service.h"
#include "include/error.h"

namespace rxdaq {

template <typename Func, typename Reply, typename... Arg>
grpc::Status HandleError(Func func, Reply *reply, Arg... args) {
	try {
		func(reply, args...);
		reply->set_message("success");
		return grpc::Status::OK;
	} catch (const UserError &e) {
		reply->set_exception_type(ExceptionType::WARNING);
		reply->set_message("operation error: " + std::string(e.what()) + "\n");
	} catch (const RXError &e) {
		reply->set_exception_type(ExceptionType::FATALERROR);
		reply->set_message("rxdaq error: " + std::string(e.what()) + "\n");
	} catch (const xia::pixie::error::error &e) {
		reply->set_exception_type(ExceptionType::FATALERROR);
		reply->set_message("xia error: " + e.result_text() + "\n" + std::string(e.what()));
	} catch (const std::exception &e) {
		reply->set_exception_type(ExceptionType::FATALERROR);
		reply->set_message("exception: " + std::string(e.what()) + "\n");
	}
	return grpc::Status::CANCELLED;
};


ControlCrateService::ControlCrateService(std::shared_ptr<Crate> crate)
: crate_(crate) {
}


grpc::Status ControlCrateService::Initialize(
	grpc::ServerContext*,
	const EmptyMessage*,
	InitializeReply *reply
) {
	
	return HandleError(
		[](
			InitializeReply *reply,
			std::shared_ptr<Crate> crate
		) {
			reply->set_num(crate->ModuleNum());
		},
		reply,
		crate_
	);
	
}


grpc::Status ControlCrateService::Boot(
	grpc::ServerContext*,
	const BootRequest *request,
	EmptyReply *reply
) {

	return HandleError(
		[](
			EmptyReply*,
			std::shared_ptr<Crate> crate,
			unsigned short module,
			bool fast
		) {
			crate->Boot(module, fast);
		},
		reply,
		crate_,
		request->module(),
		request->fast()
	);
}


grpc::Status ControlCrateService::ReadParameter(
	grpc::ServerContext*,
	const ReadRequest *request,
	ReadReply *reply
) {

	if (request->type() == uint32_t(ParameterType::kModule)) {
		return HandleError(
			[](
				ReadReply *reply,
				std::shared_ptr<Crate> crate,
				std::string name,
				unsigned short module
			) {
				reply->set_module_value(crate->ReadParameter(name, module));
			},
			reply,
			crate_,
			request->name(),
			request->module()
		);
	}

	if (request->type() == uint32_t(ParameterType::kChannel)) {
		return HandleError(
			[](
				ReadReply *reply,
				std::shared_ptr<Crate> crate,
				std::string name,
				unsigned short module,
				unsigned short channel
			) {
				reply->set_channel_value(
					crate->ReadParameter(name, module, channel)
				);
			},
			reply,
			crate_,
			request->name(),
			request->module(),
			request->channel()
		);
	}

	reply->set_exception_type(ExceptionType::FATALERROR);
	reply->set_message("rxdaq error: unknown read parameter type\n");
	return grpc::Status::CANCELLED;
}


grpc::Status ControlCrateService::WriteParameter(
	grpc::ServerContext*,
	const WriteRequest *request,
	EmptyReply *reply
) {
	if (request->type() == uint32_t(ParameterType::kModule)) {
		return HandleError(
			[](
				EmptyReply*,
				std::shared_ptr<Crate> crate,
				std::string name,
				unsigned int value,
				unsigned short module
			) {
				crate->WriteParameter(name, value, module);
			},
			reply,
			crate_,
			request->name(),
			request->module_value(),
			request->module()
		);
	}
	if (request->type() == uint32_t(ParameterType::kChannel)) {
		return HandleError(
			[](
				EmptyReply*,
				std::shared_ptr<Crate> crate,
				std::string name,
				double value,
				unsigned short module,
				unsigned short channel
			) {
				crate->WriteParameter(name, value, module, channel);
			},
			reply,
			crate_,
			request->name(),
			request->channel_value(),
			request->module(),
			request->channel()
		);
	}
	reply->set_exception_type(ExceptionType::FATALERROR);
	reply->set_message("rxdaq error: unknown write parameter type\n");
	return grpc::Status::CANCELLED;
}


}	 	// namespace rxdaq