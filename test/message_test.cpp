#include "include/message.h"

#include <gtest/gtest.h>

#include <vector>
#include <sstream>
#include <thread>

using namespace std;
using namespace rxdaq;


const vector<string> kContent = {
	"Error",
	"Warning",
	"Info",
	"Debug"
};
const vector<Message::Level> kLevels = {
	Message::Level::kNone,
	Message::Level::kError,
	Message::Level::kWarning,
	Message::Level::kInfo,
	Message::Level::kDebug
};
const vector<string> kColorfulPrefixOutput = {
	"",
	"\033[1;31mERROR: \033[0mError",
	"\033[0;33mWARNING: \033[0mWarning",
	"\033[0;32mINFO: \033[0mInfo",
	"\033[0;36mDEBUG: \033[0mDebug"
};


TEST(MessageTest, Output) {
	for (size_t i = 0; i < kLevels.size(); ++i) {
		Message message(kLevels[i]);
		stringstream ss;

		for (size_t j = 1; j < kLevels.size(); ++j) {
			ss << message(kLevels[j]) << kContent[j-1];
		}

		string output = "";
		for (size_t j = 0; j < i; ++j) {
			output += kContent[j];
		}

		EXPECT_STREQ(ss.str().c_str(), output.c_str())
			<< "Error: Unexpected output at " << i;
	}
}


TEST(MessageTest, ColorfulPrefix) {
	for (size_t i = 0; i < kLevels.size(); ++i) {
		Message message(kLevels[i]);
		message.SetColorfulPrefix();
		stringstream ss;

		for (size_t j = 1; j < kLevels.size(); ++j) {
			ss << message(kLevels[j]) << kContent[j-1];
		}

		string output = "";
		for (size_t j = 0; j <= i; ++j) {
			output += kColorfulPrefixOutput[j];
		}

		EXPECT_STREQ(ss.str().c_str(), output.c_str())
			<< "Error: Unexpected colorful prefix output at " << i;
	}
}

void PrintMessage(std::ostream &os, Message &message) {
	os << message(Message::Level::kDebug) << "This" << "-message" << "-should"
		<< "-not" << "-be" << "-seperated.\n";	
}

TEST(MessageTest, MultiThread) {
	Message message(Message::Level::kDebug);
	stringstream ss;
	vector<thread> threads;
	for (size_t i = 0; i < 128; ++i) {
		threads.emplace_back(PrintMessage, ref(ss), ref(message));
	}
	for (size_t i = 0; i < threads.size(); ++i) {
		threads[i].join();
	}
	
	string output("This-message-should-not-be-seperated.\n");
	for (size_t i = 0; i < 7; ++i) {
		output += output;
	}
	EXPECT_EQ(ss.str(), output);
}