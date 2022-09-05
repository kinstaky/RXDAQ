#ifndef __PARSER_H__
#define __PARSER_H__

#include <memory>

// #include "include/external/cxxopts.hpp"

#include "include/interactor.h"

namespace rxdaq {

class Parser {
public:
	/// @brief default constructor
	///
	Parser() = default;
	

	/// @brief default destructor
	///
	~Parser() = default;

	/// @brief parse the commands and options
	///
	std::unique_ptr<Interactor> Parse(int argc, char **argv);
};


}  // namespace rxdaq

#endif					// __PARSER_H__