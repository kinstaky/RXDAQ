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
	///
	/// @returns grpc status 
	grpc::Status Boot(
		grpc::ServerContext *context,
		const BootRequest *request,
		BootReply *reply
	);

private:
	std::shared_ptr<Crate> crate_;
};


}			// namespace rxdaq

#endif 		// __CONTROL_CRATE_SERVICE_H__