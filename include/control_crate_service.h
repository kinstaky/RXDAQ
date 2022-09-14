#ifndef __CONTROL_CRATE_SERVICE_H__
#define __CONTROL_CRATE_SERVICE_H__

#include "grpcpp/grpcpp.h"

#include "include/crate.h"
#include "src/proto/control_crate.grpc.pb.h"


namespace rxdaq {

class ControlCrateService : public ControlCrate::Service {
public:

	/// @brief constructor
	///
	ControlCrateService(std::shared_ptr<Crate> crate);
	
	/// @brief initialize the crate
	///
	/// @param[in] context extra context from client 
	/// @param[in] request empty message as placeholder
	/// @param[out] reply inclues module number or error message 
	/// @returns grpc status
	/// 
	grpc::Status Initialize(
		grpc::ServerContext *context,
		const EmptyMessage *request,
		InitializeReply *reply
	);


	/// @brief boot the crate
	///
	/// @param[in] context extra context from client
	/// @param[in] request includes module id and boot pattern
	/// @param[out] reply error message
	/// @returns grpc status 
	///
	grpc::Status Boot(
		grpc::ServerContext *context,
		const BootRequest *request,
		EmptyReply *reply
	);


	/// @brief read parameter
	///
	/// @param[in] context extra context from client
	/// @param[in] request includes parameter name, module and channel
	/// @param[out] reply includes parameter value 
	/// @returns grpc status
	///
	grpc::Status ReadParameter(
		grpc::ServerContext *context,
		const ReadRequest *request,
		ReadReply *reply
	);


	/// @brief write parameter
	///
	/// @param[in] context extra context from client
	/// @param[in] request includes parameter name, module, channel and value
	/// @param[out] reply includes
	/// @returns grpc status
	///
	grpc::Status WriteParameter(
		grpc::ServerContext *context,
		const WriteRequest *request,
		EmptyReply *reply
	);

private:
	std::shared_ptr<Crate> crate_;
};


}			// namespace rxdaq

#endif 		// __CONTROL_CRATE_SERVICE_H__