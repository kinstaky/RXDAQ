#include <stdexcept>
#include <string>


class UserError: public std::runtime_error {
public:
	UserError(const std::string& what_arg);
};


class RXError: public std::runtime_error {
public:
	RXError(const std::string &what_arg);
};