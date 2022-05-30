/*
 * dstring/outputTest.cpp
 *	This test check output string of dstring. It sets all 5 different dstring output
 *  level, including:
 *		None, Error, Warning, Info, Debug
 * 	Then dstring should just output string of higher level.
 */
#include <sstream>
#include "main/dstring.h"


int main() {
	std::stringstream ss;
	// dstring *dstring::instance = nullptr;
	dstringPtr ds;


	// none
	ds.SetLevel(dstring::Level::None);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "") return -1;
	ss.str("");

	// error
	ds.SetLevel(dstring::Level::Error);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "Error") return -1;
	ss.str("");


	// warning
	ds.SetLevel(dstring::Level::Warning);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "ErrorWarning") return -1;
	ss.str("");


	// info
	ds.SetLevel(dstring::Level::Info);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "ErrorWarningInfo") return -1;
	ss.str("");


	// debug
	ds.SetLevel(dstring::Level::Debug);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "ErrorWarningInfoDebug") return -1;
	ss.str("");

	return 0;
}