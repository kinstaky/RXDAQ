/*
 * This is the completion and correction test of Config class. Config should be
 * able to check the parameters completion of the config json file and extract
 * data correctly.
 */

#include "include/config.h"

#include <gtest/gtest.h>

#include <iostream>
#include <vector>


#ifndef TEST_DATA_DIRECTORY
#define TEST_DATA_DIRECTORY "test/data/config/"
#endif


using namespace rxdaq;

const std::string kTestDataDir = TEST_DATA_DIRECTORY;
const std::vector<std::string> kIncompletionTestDataFiles = {
	"not-exist.json",
	"empty.json",
	"lack-crate-id.json",
	"lack-modules.json",
	"lack-run.json",
	"empty-modules.json",
	"invalid-slot.json",
	"module-lack-ldr.json",
	"module-lack-bits.json",
	"no-templates.json",
	"empty-templates.json",
	"template-lack-name.json",
	"template-lack-rev.json",
	"lack-template.json",
	"modules-size-over-range.json",
	"slot-conflict.json",
	"run-lack-data-path.json",
	"run-lack-number.json"
};
const std::vector<std::string> kIncompletionTestErrorMessages = {
	"Open file \"" + kTestDataDir + "completion/not-exist.json\" failed.\n",
	"Lack of parameter \"messageLevel\".\n",
	"Lack of parameter \"crateId\".\n",
	"Lack of parameter \"modules\".\n",
	"Lack of parameter \"run\".\n",
	"Module 0 lack of parameter \"slot\".\n",
	"Slot (1) of module 0 is invalid(2 - 14).\n",
	"Module 0 lack of parameter \"template\" or \"ldr\".\n",
	"Module 0 lack of parameter \"template\" or \"bits\".\n",
	"Template \"100M\" of module 0 doesn't exist.\n",
	"Template \"100M\" lack of parameter \"ldr\".\n",
	"Template 0 lack of parameter \"name\".\n",
	"Template \"100M\" lack of parameter \"rev\".\n",
	"Template \"100M\" of module 0 doesn't exist.\n",
	"Modules size over range(1-13).\n",
	"Module 0 and module 1 share the same slot 2.\n",
	"Run lack of parameter \"dataPath\".\n",
	"Run lack of parameter \"number\".\n"
};
const std::vector<std::string> kCompletionTestDataFiles = {
	"completion.json",
	"completion-module.json"
};

struct ModuleInformation {
	unsigned short slot;
	unsigned short revision;
	unsigned short rate;
	unsigned short bits;
	std::string ldr;
	std::string par;
	std::string var;
	std::string fippi;
	std::string sys;
	std::string trig;
};
const std::vector<ModuleInformation> kModules = {
	{
		2,
		1,
		100,
		11,
		"ldr1",
		"par1",
		"var1",
		"fippi1",
		"sys1",
		"trig1"
	},
	{
		3,
		1,
		100,
		11,
		"ldr3",
		"par3",
		"var3",
		"fippi3",
		"sys3",
		"trig3"
	},
	{
		4,
		2,
		200,
		12,
		"ldr2",
		"par2",
		"var2",
		"fippi2",
		"sys2",
		"trig2"
	},
	{
		5,
		2,
		200,
		12,
		"ldr4",
		"par4",
		"var4",
		"fippi4",
		"sys4",
		"trig4"
	},
	{
		8,
		5,
		500,
		15,
		"ldr5",
		"par5",
		"var5",
		"fippi5",
		"sys5",
		"trig5"
	}
};


TEST(ConfigTest, Completion) {
	Config config;

	// handle completion errors
	for (size_t i = 0; i < kIncompletionTestDataFiles.size(); ++i) {
		try {
			config.ReadFromFile(
				kTestDataDir
				+ std::string("completion/")
				+ kIncompletionTestDataFiles[i]
			);
			FAIL() << "Error: case didn't throw error " << i;
		} catch (const std::runtime_error &e) {
			EXPECT_STREQ(e.what(), kIncompletionTestErrorMessages[i].c_str())
			 	<< "Erorr: unexpected exception message " << i;
		}
	}

	for (size_t i = 0; i < kCompletionTestDataFiles.size(); ++i) {
		EXPECT_NO_THROW(config.ReadFromFile(
			kTestDataDir
			+ std::string("completion/")
			+ kCompletionTestDataFiles[i]
		))
			<< "Error: case didn't pass " << i;
	}
}


TEST(ConfigTest, Correction) {
	Config config;
	EXPECT_NO_THROW(config.ReadFromFile(kTestDataDir + "correction/config.json"));
	
	EXPECT_STREQ(config.MessageLevel().c_str(), "debug");

	EXPECT_EQ(config.CrateId(), 1);

	EXPECT_EQ(config.ModuleNum(), kModules.size());

	EXPECT_EQ(config.RunNumber(), 10);

	EXPECT_STREQ(config.RunDataPath().c_str(), "./");

	EXPECT_STREQ(config.RunDataFile().c_str(), "data");
	
	for (unsigned short i = 0; i < config.ModuleNum(); ++i) {
		EXPECT_EQ(config.Slot(i), kModules[i].slot)
			<< "Error: slot " << i;
		
		EXPECT_EQ(config.Revision(i), kModules[i].revision)
			<< "Error: revision " << i;

		EXPECT_EQ(config.Rate(i), kModules[i].rate)
			<< "Error: rate " << i;

		EXPECT_EQ(config.Bits(i), kModules[i].bits)
			<< "Error: bits " << i;

		EXPECT_EQ(config.Ldr(i), kModules[i].ldr)
			<< "Error: ldr " << i;

		EXPECT_EQ(config.Par(i), kModules[i].par)
			<< "Eroro: par " << i;

		EXPECT_EQ(config.Var(i), kModules[i].var)
			<< "Error: var " << i;

		EXPECT_EQ(config.Fippi(i), kModules[i].fippi)
			<< "Error: fippi " << i;
		
		EXPECT_EQ(config.Sys(i), kModules[i].sys)
			<< "Error: sys " << i;

		EXPECT_EQ(config.Trig(i), kModules[i].trig)
			<< "Error: trig " << i;
	}
}