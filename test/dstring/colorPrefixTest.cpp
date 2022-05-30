/*
 * dstring/colorPrefixTest.cpp
 *	This test checks output string of dstring with color and prefix set, set all
 *  5 different dstring output level, including:
 *		None, Error, Warning, Info, Debug
 * 	Then dstring should output the string with different colorful prefix string
 *  of different level
 */

#include <sstream>
#include "main/dstring.h"


int main() {
	std::stringstream ss;
	dstringPtr ds;
	ds.SetColorPrefix();


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
	if (ss.str() != "\033[1;31mERROR: \033[0mError") return -1;
	ss.str("");


	// warning
	ds.SetLevel(dstring::Level::Warning);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "\033[1;31mERROR: \033[0mError\033[0;33mWARNING: \033[0mWarning") return -1;
	ss.str("");


	// info
	ds.SetLevel(dstring::Level::Info);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "\033[1;31mERROR: \033[0mError\033[0;33mWARNING: \033[0mWarning\033[0;32mINFO: \033[0mInfo") return -1;
	ss.str("");


	// debug
	ds.SetLevel(dstring::Level::Debug);
	ss << ds(dstring::Level::Error, "Error");
	ss << ds(dstring::Level::Warning, "Warning");
	ss << ds(dstring::Level::Info, "Info");
	ss << ds(dstring::Level::Debug, "Debug");
	std::cout << ss.str() << std::endl;
	if (ss.str() != "\033[1;31mERROR: \033[0mError\033[0;33mWARNING: \033[0mWarning\033[0;32mINFO: \033[0mInfo\033[0;36mDEBUG: \033[0mDebug") return -1;
	ss.str("");

	return 0;
}