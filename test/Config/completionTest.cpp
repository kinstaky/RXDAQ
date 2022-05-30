/*
 * Config/completionTest.cpp
 *  This is the  completion test of Config class. Config should be able to
 *  check the parameters completion of the config json file.
 */

#include "main/Config.h"

#include <iostream>

int main() {
	Config config;
	int errors = 0;

	// test 1
	// file not exist
	try {
		config.ReadFromFile("NotExist.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("Open file NotExist.json failed.\n")) ++errors;
	}

	// test2
	// empty json file
	try {
		config.ReadFromFile("test/Config/data/completion/Empty.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("config file lack of \"crate\" part.\n")) ++errors;
	}

	// test3
	// only parameter crate and emtpy crate
	try {
		config.ReadFromFile("test/Config/data/completion/EmptyCrate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("crate 0 lack of parameter \"slots\".\n")) ++errors;
	}

	// test4
	// crate lack of module
	try {
		config.ReadFromFile("test/Config/data/completion/LackModule.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("crate 0 lack of parameter \"module\".\n")) ++errors;
	}

	// test5
	// crate lack of slots
	try {
		config.ReadFromFile("test/Config/data/completion/LackSlots.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("crate 0 lack of parameter \"slots\".\n")) ++errors;
	}

	// test 6
	// crate with empty module
	try {
		config.ReadFromFile("test/Config/data/completion/EmptyModule.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"ldr\".\n")) ++errors;
	}

	// test 7
	// module lack of ldr
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackLdr.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"ldr\".\n")) ++errors;
	}

	// test 8
	// module lack of par
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackPar.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"par\".\n")) ++errors;
	}

	// test 9
	// module lack of var
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackVar.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"var\".\n")) ++errors;
	}

	// test 10
	// module lack of fippi
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackFippi.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"fippi\".\n")) ++errors;
	}

	// test 11
	// module lack of sys
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackSys.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"sys\".\n")) ++errors;
	}

	// test 12
	// module lack of trig
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackTrig.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"trig\".\n")) ++errors;
	}

	// test 13
	// module lack of rev
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackRev.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"rev\".\n")) ++errors;
	}

	// test 14
	// module lack of rate
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackRate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"rate\".\n")) ++errors;
	}

	// test 15
	// module lack of bits
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleLackBits.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("module 0 lack of parameter \"template\" or \"bits\".\n")) ++errors;
	}

	// test 16
	// module with template
	try {
		config.ReadFromFile("test/Config/data/completion/NoTemplate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template \"100M\" of module 0 doesn't exist.\n")) ++errors;
	}

	// test 17
	// module with template
	try {
		config.ReadFromFile("test/Config/data/completion/EmptyTemplate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"ldr\".\n")) ++errors;
	}

	// test 18
	// template lack of ldr
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackLdr.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"ldr\".\n")) ++errors;
	}

	// test 19
	// template lack of par
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackPar.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"par\".\n")) ++errors;
	}

	// test 20
	// template lack of var
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackVar.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"var\".\n")) ++errors;
	}

	// test 21
	// template lack of fippi
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackFippi.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"fippi\".\n")) ++errors;
	}

	// test 22
	// template lack of sys
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackSys.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"sys\".\n")) ++errors;
	}

	// test 23
	// template lack of trig
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackTrig.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"trig\".\n")) ++errors;
	}

	// test 24
	// template lack of rev
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackRev.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"rev\".\n")) ++errors;
	}

	// test 25
	// template lack of rate
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackRate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"rate\".\n")) ++errors;
	}

	// test 26
	// template lack of bits
	try {
		config.ReadFromFile("test/Config/data/completion/TemplateLackBits.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template 100M lack of parameter \"bits\".\n")) ++errors;
	}

	// test 27
	// lack of template
	try {
		config.ReadFromFile("test/Config/data/completion/LackTemplate.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("template \"100M\" of module 0 doesn't exist.\n")) ++errors;
	}

	// test 28
	// module size not equal to slots size
	try {
		config.ReadFromFile("test/Config/data/completion/ModuleSizeError.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("crate 0 slots size not equal to module size.\n")) ++errors;
	}

	// test 29
	// crate id too large
	try {
		config.ReadFromFile("test/Config/data/completion/Completion.json", 1);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		if (e.what() == std::string("crate id(1) should smaller than 1.\n")) ++errors;
	}

	// test 30
	// completed config file, this should pass the test
	try {
		config.ReadFromFile("test/Config/data/completion/Completion.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		return -1;
	}

	// test 31
	// completed config file, this should pass the test, there is no template in this config file
	try {
		config.ReadFromFile("test/Config/data/completion/CompletionModule.json", 0);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		return -1;
	}

	if (errors == 29) return 0;
	return -1;
}