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
	"lack-xia-log-level.json",
	"lack-parameter-file.json",
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
	"Lack of parameter \"xiaLogLevel\".\n",
	"Lack of parameter \"parameterFile\".\n",
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
	std::string var;
	std::string fippi;
	std::string sys;
	std::string version;
};
const std::vector<ModuleInformation> kModules = {
	{
		2,
		10,
		100,
		14,
		"ldr1",
		"var1",
		"fippi1",
		"sys1",
		"1"
	},
	{
		3,
		10,
		100,
		14,
		"ldr3",
		"var3",
		"fippi3",
		"sys3",
		"3"
	},
	{
		4,
		12,
		250,
		12,
		"ldr2",
		"var2",
		"fippi2",
		"sys2",
		"2"
	},
	{
		5,
		12,
		250,
		12,
		"ldr4",
		"var4",
		"fippi4",
		"sys4",
		"4"
	},
	{
		8,
		15,
		500,
		16,
		"ldr5",
		"var5",
		"fippi5",
		"sys5",
		"5"
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
			<< "Error: completion case didn't pass " << i;
	}
}


TEST(ConfigTest, Correction) {
	Config config;
	EXPECT_NO_THROW(config.ReadFromFile(kTestDataDir + "correction/config.json"));
	
	EXPECT_STREQ(config.MessageLevel().c_str(), "debug");

	EXPECT_EQ(config.CrateId(), 1);

	EXPECT_EQ(config.ModuleNum(), kModules.size());

	EXPECT_EQ(config.RunNumber(), 10);

	EXPECT_STREQ(config.ParameterFile().c_str(), "parameters.json");

	EXPECT_STREQ(config.XiaLogLevel().c_str(), "warning");

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

		EXPECT_EQ(config.Var(i), kModules[i].var)
			<< "Error: var " << i;

		EXPECT_EQ(config.Fippi(i), kModules[i].fippi)
			<< "Error: fippi " << i;
		
		EXPECT_EQ(config.Sys(i), kModules[i].sys)
			<< "Error: sys " << i;

		EXPECT_EQ(config.Version(i), kModules[i].version)	
			<< "Error: version " << i;
	}
}