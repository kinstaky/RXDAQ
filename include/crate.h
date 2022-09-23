#ifndef __CRATE_H__
#define __CRATE_H__

#include <fstream>
#include <string>
#include <mutex>

#include "pixie/pixie16/crate.hpp"
#include "nlohmann/json.hpp"

#include "include/config.h"
#include "include/message.h"

namespace rxdaq {

const unsigned short kChannelNum = 16;


/// parameters type
enum class ParameterType {
	kAll = 0,
	kCrate,
	kModule,
	kChannel,
	kInvalid
};
/// parameters type - parameter type name
const std::map<ParameterType, std::string> kParameterTypeNames = {
	{ParameterType::kAll, "all"},
	{ParameterType::kCrate, "crate"},
	{ParameterType::kModule, "module"},
	{ParameterType::kChannel, "channel"},
	{ParameterType::kInvalid, "invalid"}
};


// verbose parameters
namespace vparam {
	struct VerboseParameterInformation {
		std::string name;
		unsigned int bit;
		unsigned int length;
		std::vector<std::string> alias;
	};


	const std::map<std::string, std::vector<VerboseParameterInformation>>
		verbose_parameters =
	{
		{
			"MODULE_CSRB",
			{
				{"PULLUP", 0, 1, {"CPLDPULLUP"}},
				{"DIRMOD", 4, 1, {"DIRMOD"}},
				{"CMASTER", 6, 1, {"CHASSISMASTER"}},
				{"GFTSEL", 7, 1, {"GFTSEL"}},
				{"ETSEL", 8, 1, {"ETSEL"}},
				{"INHIBIT", 10, 1, {"INHIBITENA"}},
				{"MCRATE", 11, 1, {"MULTICRATES"}},
				{"SORT", 12, 1, {"SORTEVENTS"}},
				{"BFAST", 13, 1, {"BKPLFASTTRIG"}}
			}
		},
		{
			"CHANNEL_CSRA",
			{
				{"FTS", 0, 1, {"FRTIGSEL"}},
				{"MSE", 1, 1, {"EXTTRIGSEL"}},
				{"GC", 2, 1, {"GOOD"}},
				{"CSE", 3, 1, {"CHANTRIGSEL"}},
				{"BDA", 4, 1, {"SYNCDATAACQ"}},
				{"SP", 5, 1, {"POLARITY"}},
				{"CTV", 6, 1, {"VETOENA"}},
				{"HE", 7, 1, {"HISTOE", "HIST"}},
				{"TC", 8, 1, {"TRACEENA"}},
				{"QDC", 9 ,1, {"QDECENA", "EQS"}},
				{"CFD", 10, 1, {"CFDMODE", "ECT"}},
				{"MVT", 11, 1, {"GLOBTRIG"}},
				{"ERB", 12, 1, {"ESUMSENA"}},
				{"CVT", 13, 1, {"CHANTRIG"}},
				{"IR", 14, 1, {"ENARELAY"}},
				{"NPR", 15, 1, {"PILEUPCTRL"}},
				{"IPR", 16, 1, {"INVERSEPILEUP"}},
				{"NTL", 17, 1, {"ENAENERGYCUT"}},
				{"GTS", 18, 1, {"GROUPTRIGSEL"}},
				{"CVS", 19, 1, {"CHANVETOSEL"}},
				{"MVS", 20, 1, {"MODVETOSEL"}},
				{"ETS", 21, 1, {"EXTTSENA"}}
			}
		}
	};


	/// @brief check type of parameter
	///
	/// @param[in] name name of the parameter type 
	/// @returns parameter type
	/// 
	ParameterType CheckParameter(const std::string &name);


	/// @brief check whether the variable is valid
	///
	/// @param[in] name name of the parameter
	/// @returns true if valid, false otherwise
	///
	inline bool Valid(const std::string &name) {
		return CheckParameter(name) != ParameterType::kInvalid;
	}


	/// @brief get list of verbose parameter name from the origin parameter
	///
	/// @param[in] name name of the parameter 
	/// @returns list of verbose names
	///
	std::vector<std::string> VerboseNames(const std::string &name);


	/// @brief get list of value of verbose parameter
	///
	/// @param[in] name name of the parameter 
	/// @param[in] value value of the parameter
	/// @returns list of verbose values
	/// 
	std::vector<unsigned int> VerboseValues(
		const std::string &name,
		unsigned int value
	);


	/// @brief get list of value of verbose parameter
	///
	/// @param[in] name name of the parameter 
	/// @param[in] value value of the parameter
	/// @returns list of verbose values
	/// 
	std::vector<unsigned int> VerboseValues(
		const std::string &name,
		double value
	);


	/// @brief list the available verbose parameters in string
	///
	/// @param[in] type type of the parameters
	/// @returns parameters in string
	/// 
	std::string ListParameters(ParameterType type = ParameterType::kAll);
};




/// This class represents a physical XIA crate, and provides some interface to
/// control it. So the interactors can call this methods include boot, read and
/// write parameters, get firmware information, import or export the parameters
/// of firmware and run in list mode to accquire data.
class Crate {
public:

	/// @brief constructor
	///
	Crate() noexcept;


	/// @brief destructor
	///
	virtual ~Crate();


	//-------------------------------------------------------------------------
	// 					method to get crate information
	//-------------------------------------------------------------------------

	/// @brief get module number
	///
	/// @returns module number
	///
	virtual inline unsigned short ModuleNum() const noexcept {
		return config_.ModuleNum();
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
	virtual void Initialize(const std::string &config_path = "");


	/// @brief boot modules
	///
	/// @param[in] module_id module to boot 
	/// @param[in] fast true for fast boot (don't boot fpga or load parameters)
	///
	virtual void Boot(unsigned short module_id, bool fast = true);


	//-------------------------------------------------------------------------
	//	 					method for auto task
	//-------------------------------------------------------------------------
	

	/// @brief process auto task
	///
	/// @param[in] task_name name of task to process
	/// @param[in] module module to process
	///
	virtual void Task(const std::string &task_name, unsigned short module);


	/// @brief get the available tasks
	///
	/// @returns available tasks list in string
	///
	virtual std::string ListTasks() const;


	//-------------------------------------------------------------------------
	//	 				method to read and write parameters
	//-------------------------------------------------------------------------

	/// @brief list parameters
	///
	/// @returns list of parameters in string format
	///
	virtual std::string ListParameters(
		ParameterType type = ParameterType::kAll,
		bool verbose = false
	) noexcept;


	/// @brief check parameter type by name
	///
	/// @param[in] name parameter name
	/// @returns parameter type enum class
	///
	static ParameterType CheckParameter(const std::string &name) noexcept;
	

	/// @brief read module parameter
	///
	/// @param[in] name name of the parameter
	/// @param[in] module modules to read
	/// @returns parameter value
	///
	virtual unsigned int ReadParameter(
		const std::string &name,
		unsigned short module
	);


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
	);


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
	);


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
	);


	/// @brief import parameters from json file
	///
	/// @param[in] path path to import
	/// 
	virtual void ImportParameters(const std::string &path);
	
	
	/// @brief export parameters to json file
	///
	/// @param path path to export
	/// 
	virtual void ExportParameters(const std::string &path);


	//-------------------------------------------------------------------------
	//	 					method for list mode run
	//-------------------------------------------------------------------------

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
	);


	/// @brief stop list mode run
	///
	virtual inline void StopRun() {
		keep_running_ = false;
	}


	/// @brief get the run number
	///
	virtual inline unsigned int RunNumber() const {
		return config_.RunNumber();
	}


	
	// virtual void PrintInfo() const;

private:

	/// @brief load the firmware
	///
	/// @param[in] module_id module to load
	/// 
	virtual void LoadFirmware(unsigned short module_id);


	/// @brief read list mode data from hardware to binary files
	///
	/// @param[in] module_id module to read from
	/// @param[in] threshold threshold of buffer, write if over this threshold
	///
	void ReadListModeData(unsigned short module_id, unsigned int threshold);


	/// @brief wait for all modules finish
	///
	/// @param[in] modules module to check
	///
	/// @throws rx error if not all module stop properly
	///
	void WaitFinished(const std::vector<unsigned short> &modules);


	/// @brief do something after run finished
	///
	/// @param[in] module_id module to finish
	///
	void FinishRun(unsigned short module_id);


	static void SigIntHandler(int) {
		std::cout << "\nYor press Ctrl+C to stop run, press again to quit."
			 << std::endl;
		keep_running_ = false;
	}


	// xia crate, the lower level compoment
	xia::pixie::crate::crate xia_crate_;

	// message output control
	Message message_;

	// boot flags
	bool booted_;

	// firmwares and lock
	std::map<std::string, xia::pixie::firmware::firmware_ref> firmwares_;
	std::mutex firmwares_lock_;

	// config
	std::string config_path_;
	Config config_;

	// run variables
	std::vector<std::ofstream> run_output_streams_;
	std::chrono::steady_clock::time_point run_start_time_;
	static std::atomic<bool> keep_running_;
};


/// @brief generate run time information
///
/// @param[in] duration duration time in seconds
/// @param[in] run run number to display, -1 not to display
/// @returns run time information in string
///
std::string RunTimeInfo(unsigned int duration, int run = -1);


/// @brief create vector of indexes for modules or channels
///
/// @param[in] max_index maximum index of the vector 
/// @param[in] reality_limit reality limit of the vector size
/// @param[in] index if index is equal to max_index, create the list from 0
///		to reality limit, otherwise create a vector with only this index 
/// @returns vector of the indexes
///
std::vector<unsigned short> CreateRequestIndexes(
	unsigned short max_index,
	unsigned short reality_limit,
	unsigned short index
);


/// @brief check whether module is larger than 13 or smaller than 0
///
/// @param[in] module index of module
///
/// @throws UserError if module number is not in [0, 13]
/// 
void CheckModuleNumber(int module);


/// @brief check whether channel is larger than 16 or smaller than 0
///
/// @param[in] channel index of channel
//
/// @throws UserError if module number is not in [0, 16]
/// 
void CheckChannelNumber(int channel);


/// @brief check whether task name is available
///
/// @param[in] task_name task name to check
///
/// @throw UserError if task name is not available
///
void CheckTaskName(const std::string &task_name);

}	// namespace rxdaq





#endif		// __CRATE_H__