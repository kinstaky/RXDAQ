/*
 * correctionTest.cpp
 *  This test checks the correction of the config read from file.
 */

#include "main/Config.h"
#include <iostream>
#include <string>

using std::string;

int main() {
	Config config1, config2;
	int ret = 0;

	try {
		config1.ReadFromFile("test/Config/data/correction/config.json", 0);
		config2.ReadFromFile("test/Config/data/correction/config.json", 1);
	} catch (const std::exception &e) {
		std::cerr << e.what();
		ret--;
	}


	// test ModuleNum()
	if (config1.ModuleNum() != 3) {
		std::cerr << "Crate 0 ModuleNum() error." << std::endl;
		std::cerr << config1.ModuleNum() << std::endl;
		ret--;
	}
	if (config2.ModuleNum() != 2) {
		std::cerr << "Crate 1 ModuleNum() error." << std::endl;
		std::cerr << config2.ModuleNum() << std::endl;
		ret--;
	}


	// test PXISlotMap()
	unsigned short *slots = config1.PXISlotMap();
	if (slots[0] != 1 || slots[1] != 2 || slots[2] != 3) {
		std::cerr << "Crate 0 PXISlotMap() error." << std::endl;
		std::cerr << slots[0] << "  " << slots[1] << "  " << slots[2] << std::endl;
		ret--;
	}
	slots = config2.PXISlotMap();
	if (slots[0] != 3 || slots[1] != 4) {
		std::cerr << "Crate 1 PXISlotMap() error." << std::endl;
		std::cerr << slots[0] << "  " << slots[1] << std::endl;
		ret--;
	}


	// test Rev()
	if (config1.Rev(0) != 1 || config1.Rev(1) != 1 || config1.Rev(2) != 2) {
		std::cerr << "Crate 0 Rev() error." << std::endl;
		std::cerr << config1.Rev(0) << "  " << config1.Rev(1) << "  " << config1.Rev(2) << std::endl;
		ret--;
	}
	if (config2.Rev(0) != 2 || config2.Rev(1) != 5) {
		std::cerr << "Crate 1 Rev error." << std::endl;
		std::cerr << config2.Rev(0) << "  " << config2.Rev(1) << std::endl;
		ret--;
	}


	// test Rate()
	if (config1.Rate(0) != 100 || config1.Rate(1) != 100 || config1.Rate(2) != 200) {
		std::cerr << "Crate 0 Rate() error." << std::endl;
		std::cerr << config1.Rate(0) << "  " << config1.Rate(1) << "  " << config1.Rate(2) << std::endl;
		ret--;
	}
	if (config2.Rate(0) != 200 || config2.Rate(1) != 500) {
		std::cerr << "Crate 1 Rate() error." << std::endl;
		std::cerr << config2.Rate(0) << "  " << config2.Rate(1) << std::endl;
		ret--;
	}


	// test Bits()
	if (config1.Bits(0) != 11 || config1.Bits(1) != 11 || config1.Bits(2) != 12) {
		std::cerr << "Crate 0 Bits() error." << std::endl;
		std::cerr << config1.Bits(0) << "  " << config1.Bits(1) << "  " << config1.Bits(2) << std::endl;
		ret--;
	}
	if (config2.Bits(0) != 12 || config2.Bits(1) != 15) {
		std::cerr << "Crate 1 Bits() error." << std::endl;
		std::cerr << config2.Bits(0) << "  " << config2.Bits(1) << std::endl;
		ret--;
	}


	// test Ldr()
	if (config1.Ldr(0) != string("ldr1") || config1.Ldr(1) != string("ldr3") || config1.Ldr(2) != string("ldr2")) {
		std::cerr << "Crate 0 Ldr() error." << std::endl;
		std::cerr << config1.Ldr(0) << "  " << config1.Ldr(1) << "  " << config1.Ldr(2) << std::endl;
		ret--;
	}
	if (config2.Ldr(0) != string("ldr4") || config2.Ldr(1) != string("ldr5")) {
		std::cerr << "Crate 1 Ldr() error." << std::endl;
		std::cerr << config2.Ldr(0) << "  " << config2.Ldr(1) << std::endl;
		ret--;
	}


	// test Par()
	if (config1.Par(0) != string("par1") || config1.Par(1) != string("par3") || config1.Par(2) != string("par2")) {
		std::cerr << "Crate 0 Par() error." << std::endl;
		std::cerr << config1.Par(0) << "  " << config1.Par(1) << "  " << config1.Par(2) << std::endl;
		ret--;
	}
	if (config2.Par(0) != string("par4") || config2.Par(1) != string("par5")) {
		std::cerr << "Crate 1 Par() error." << std::endl;
		std::cerr << config2.Par(0) << "  " << config2.Par(1) << std::endl;
		ret--;
	}


	// test Var()
	if (config1.Var(0) != string("var1") || config1.Var(1) != string("var3") || config1.Var(2) != string("var2")) {
		std::cerr << "Crate 0 Var() error." << std::endl;
		std::cerr << config1.Var(0) << "  " << config1.Var(1) << "  " << config1.Var(2) << std::endl;
		ret--;
	}
	if (config2.Var(0) != string("var4") || config2.Var(1) != string("var5")) {
		std::cerr << "Crate 1 Var() error." << std::endl;
		std::cerr << config2.Var(0) << "  " << config2.Var(1) << std::endl;
		ret--;
	}



	// test Fippi()
	if (config1.Fippi(0) != string("fippi1") || config1.Fippi(1) != string("fippi3") || config1.Fippi(2) != string("fippi2")) {
		std::cerr << "Crate 0 Fippi() error." << std::endl;
		std::cerr << config1.Fippi(0) << "  " << config1.Fippi(1) << "  " << config1.Fippi(2) << std::endl;
		ret--;
	}
	if (config2.Fippi(0) != string("fippi4") || config2.Fippi(1) != string("fippi5")) {
		std::cerr << "Crate 1 Fippi() error." << std::endl;
		std::cerr << config2.Fippi(0) << "  " << config2.Fippi(1) << std::endl;
		ret--;
	}

	// test Sys()
	if (config1.Sys(0) != string("sys1") || config1.Sys(1) != string("sys3") || config1.Sys(2) != string("sys2")) {
		std::cerr << "Crate 0 Sys() error." << std::endl;
		std::cerr << config1.Sys(0) << "  " << config1.Sys(1) << "  " << config1.Sys(2) << std::endl;
		ret--;
	}
	if (config2.Sys(0) != string("sys4") || config2.Sys(1) != string("sys5")) {
		std::cerr << "Crate 1 Sys() error." << std::endl;
		std::cerr << config2.Sys(0) << "  " << config2.Sys(1) << std::endl;
		ret--;
	}



	// test Trig()
	if (config1.Trig(0) != string("trig1") || config1.Trig(1) != string("trig3") || config1.Trig(2) != string("trig2")) {
		std::cerr << "Crate 0 Trig() error." << std::endl;
		std::cerr << config1.Trig(0) << "  " << config1.Trig(1) << "  " << config1.Trig(2) << std::endl;
		ret--;
	}
	if (config2.Trig(0) != string("trig4") || config2.Trig(1) != string("trig5")) {
		std::cerr << "Crate 1 Trig() error." << std::endl;
		std::cerr << config2.Trig(0) << "  " << config2.Trig(1) << std::endl;
		ret--;
	}


	return ret;
}