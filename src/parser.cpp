#include "include/parser.h"

#include <cstring>

namespace rxdaq {

std::unique_ptr<Interactor> Parser::Parse(int argc, char **argv) {
	if (argc == 1) {
		throw std::runtime_error("invalid argument number");
	}
	if (!strcmp(argv[1], "help")) {
		return std::make_unique<HelpCommandParser>();
	}
	throw std::runtime_error("invalid arguments");
	return nullptr;
}


} 	// namespace rxdaq