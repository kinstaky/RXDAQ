#include "include/parser.h"

#include <cstring>

#include "include/error.h"

namespace rxdaq {

std::unique_ptr<Interactor> Parser::Parse(int argc, char **argv) {
	std::unique_ptr<Interactor> result = nullptr;

	if (argc == 1) {
		throw UserError("too few arguments");
	}
	result = Interactor::CreateInteractor(argv[1]);
	if (!result) {
		throw UserError("invalid command");
	}
	result->Parse(argc-1, argv+1);
	return result;
}


} 	// namespace rxdaq