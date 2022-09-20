#ifndef __REMOTE_CRATE_H__
#define __REMOTE_CRATE_H__

#include "include/crate.h"

#include "src/proto/control_crate.grpc.pb.h"

namespace rxdaq {

class RemoteCrate final : public Crate {
public:

	/// @brief constructor
	///
	RemoteCrate(std::shared_ptr<grpc::Channel> channel) noexcept;


	/// @brief default destructor
	~RemoteCrate() = default;

	
	//-------------------------------------------------------------------------
	// 					method to get crate information
	//-------------------------------------------------------------------------

	/// @brief get module number
	///
	/// @returns module number
	///
	virtual inline unsigned short ModuleNum() const noexcept override {
		return module_num_;
	}


	//-------------------------------------------------------------------------
	//	 				method to initialize and boot
	//-------------------------------------------------------------------------


	/// @brief initialize the crate
	///
	/// @param[in] config_path path of config file, "" for default
	///
	/// @throws XiaError if initialize failed
	///
	virtual void Initialize(const std::string &) override;


	/// @brief boot modules
	///
	/// @param[in] module_id module to boot 
	/// @param[in] fast true for fast boot (don't boot fpga or load parameters)
	///
	virtual void Boot(unsigned short module_id, bool fast = true) override;
	

	// //-------------------------------------------------------------------------
	// //	 				method to read and write parameters
	// //-------------------------------------------------------------------------


	/// @brief read module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] module modules to read
	/// @returns parameter value
	///
	virtual unsigned int ReadParameter(
		const std::string &name,
		unsigned short module
	) override;


	/// @brief read channel parameters
	///
	/// @param[in] name name of the parameter
	/// @param[in] module the module to read 
	/// @param[in] channel the channel to read
	/// @returns parameter value
	///
	virtual double ReadParameter(
		const std::string &name,
		unsigned short module,
		unsigned short channel
	) override;


	/// @brief write module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] value value to write
	/// @param[in] module module to write
	///
	virtual void WriteParameter(
		const std::string &name,
		unsigned int value,
		unsigned short module
	) override;


	/// @brief write channel parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] value value to write
	/// @param[in] module module to write
	/// @param[in] channel channel to write
	///
	virtual void WriteParameter(
		const std::string &name,
		double value,
		unsigned short module,
		unsigned short channel
	) override;


	/// @brief import parameters from json file
	///
	/// @param[in] path path to import
	/// 
	virtual void ImportParameters(const std::string &path) override;
	
	
	/// @brief export parameters to json file
	///
	/// @param path path to export
	/// 
	virtual void ExportParameters(const std::string &path) override;


	/// @brief start list mode run
	///
	/// @param[in] module_id module to run in list mode 
	/// @param[in] seconds seconds to run, 0 for infinite time
	/// @param[in] run run number, -1 to read from config file
	///
	virtual void StartRun(
		unsigned short module_id,
		unsigned int seconds,
		int run
	) override;


	/// @brief stop list mode run
	///
	/// @param[in] module_id module to stop
	///
	virtual void StopRun() override;



private:

	struct AsyncClientCall {
		int type;
		std::unique_ptr<grpc::ClientAsyncResponseReader<RunReply>> reader;
		grpc::ClientContext context;
		RunReply reply;
		grpc::Status status;
	};


	/// @brief signal INT handler, stop the list mode run
	///  
	static void SigIntHandler(int);

	unsigned short module_num_;
	std::unique_ptr<ControlCrate::Stub> stub_;
	static RemoteCrate *instance_;
	grpc::CompletionQueue completion_queue_;
};

}		// namespace rxdaq

#endif 	// __REMOTE_CRATE_H__