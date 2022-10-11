#include "include/control_crate_service.h"
#include "include/error.h"

namespace rxdaq {

template <typename Func, typename Reply, typename... Arg>
grpc::Status HandleError(Func func, Reply *reply, Arg... args) {
	std::string status_message = "";
	try {
		func(reply, args...);
		reply->set_status_type(StatusType::SUCCESS);
		reply->set_status_message("success");
	} catch (const UserError &e) {
		reply->set_status_type(StatusType::WARNING);
		reply->set_status_message("operation error: " + std::string(e.what()) + "\n");
		status_message = "operation error: " + std::string(e.what()) + "\n";
	} catch (const RXError &e) {
		reply->set_status_type(StatusType::FATALERROR);
		reply->set_status_message("rxdaq error: " + std::string(e.what()) + "\n");
		status_message = "rxdaq error: " + std::string(e.what()) + "\n";
	} catch (const xia::pixie::error::error &e) {
		reply->set_status_type(StatusType::FATALERROR);
		reply->set_status_message("xia error: " + e.result_text() + "\n" + std::string(e.what()));
		status_message = "xia error: " + e.result_text() + "\n" + std::string(e.what());
	} catch (const std::exception &e) {
		reply->set_status_type(StatusType::FATALERROR);
		reply->set_status_message("exception: " + std::string(e.what()) + "\n");
		status_message = "exception: " + std::string(e.what()) + "\n";
	}
	return grpc::Status::OK;
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

	reply->set_status_type(StatusType::FATALERROR);
	reply->set_status_message("rxdaq error: unknown read parameter type\n");
	return grpc::Status::OK;
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
	reply->set_status_type(StatusType::FATALERROR);
	reply->set_status_message("rxdaq error: unknown write parameter type\n");
	return grpc::Status::OK;
}


grpc::Status ControlCrateService::ImportParameters(
	grpc::ServerContext *,
	const ImportExportRequest *request,
	EmptyReply *reply
) {

	return HandleError(
		[](
			EmptyReply*,
			std::shared_ptr<Crate> crate,
			const std::string &path 
		) {
			crate->ImportParameters(path);
		},
		reply,
		crate_,
		request->path()
	);
}


grpc::Status ControlCrateService::ExportParameters(
	grpc::ServerContext *,
	const ImportExportRequest *request,
	EmptyReply *reply
) {

	return HandleError(
		[](
			EmptyReply*,
			std::shared_ptr<Crate> crate,
			const std::string &path
		) {
			crate->ExportParameters(path);
		},
		reply,
		crate_,
		request->path()
	);
}


grpc::Status ControlCrateService::StartRun(
	grpc::ServerContext*,
	const RunRequest *request,
	RunReply *reply
) {

	return HandleError(
		[](
			RunReply *reply,
			std::shared_ptr<Crate> crate,
			unsigned short module,
			unsigned int seconds,
			int run
		) {
			auto start_time = std::chrono::steady_clock::now();
			crate->StartRun(module, seconds, run);
			auto stop_time = std::chrono::steady_clock::now();
			reply->set_seconds(
				std::chrono::duration_cast<std::chrono::seconds>(
					stop_time - start_time
				).count()
			);
			reply->set_run_number(crate->RunNumber()-1);
		},
		reply,
		crate_,
		request->module(),
		request->seconds(),
		request->run_number()
	);
}


grpc::Status ControlCrateService::StopRun(
	grpc::ServerContext *,
	const EmptyMessage *,
	RunReply *reply
) {

	return HandleError(
		[](
			RunReply *,
			std::shared_ptr<Crate> crate
		) {
			crate->StopRun();
		},
		reply,
		crate_
	);
}


}	 	// namespace rxdaq