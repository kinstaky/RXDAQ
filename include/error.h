#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdexcept>
#include <string>

namespace rxdaq {

/// This class represents errors made by user, like module id over range,
/// invalid parameter name. The policy to handle this kind of errors is
/// ignoring the operation caused error and continue to run the program.
class UserError final : public std::runtime_error {
public:

	/// @brief constructor
	///
	/// @param[in] what_arg 
	UserError(const std::string& what_arg);
};

/// This class represents errors caused by the programer. It implies that the
/// code should be edit somewhere. Actually it should not be thrown.
class RXError final : public std::runtime_error {
public:

	/// @brief constructor
	/// @param[in] what_arg 
	RXError(const std::string &what_arg);
};

}		// namespace rxdaq

#endif	// __ERROR_H__