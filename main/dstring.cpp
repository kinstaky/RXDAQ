/*
 * dstring.cpp
 */


#include "main/dstring.h"

//--------------------------------------------------
//				dstring
//--------------------------------------------------

dstring *dstring::instance = nullptr;

dstring *dstring::Instance() {
	if (instance == nullptr) {
		instance = new dstring(Level::Error, false, false);
	}
	return instance;
}


dstring::dstring(Level lv, bool cf, bool pf) {
	levelFlag = lv;
	colorFlag = cf;
	prefixFlag = pf;
}

dstring::dstring(const dstring &other) {
	levelFlag = other.levelFlag;
	colorFlag = other.colorFlag;
	prefixFlag = other.prefixFlag;
}

const dstring &dstring::operator=(const dstring &rhs) {
	levelFlag = rhs.levelFlag;
	colorFlag = rhs.colorFlag;
	prefixFlag = rhs.prefixFlag;
	return *this;
}

dstring::~dstring() {}

// set the level flag, none, error, warning, info or debug
int dstring::SetLevel(Level lv) {
	levelFlag = lv;
	if (levelFlag > Level::Debug) return -1;
	return 0;
}

// set the prefix flag
// true  -- print prefix
// false -- without prefix
int dstring::SetPrefix(bool prefix) {
	prefixFlag = prefix;
	return 0;
}

// set the prefix color flag
int dstring::SetColor(bool color) {
	colorFlag = color;
	return 0;
}


// set color and prefix flag
int dstring::SetColorPrefix() {
	colorFlag = true;
	prefixFlag = true;
	return 0;
}

const std::map<dstring::Level, std::string> dstring::prefixMap = {
	{Level::None, ""},
	{Level::Error, "ERROR: "},
	{Level::Warning, "WARNING: "},
	{Level::Info, "INFO: "},
	{Level::Debug, "DEBUG: "}
};

const std::map<dstring::Level, std::string> dstring::colorMap = {
	{Level::None, ""},							// nothing
	{Level::Error, "\033[1;31m"},				// light red
	{Level::Warning, "\033[0;33m"},				// orange
	{Level::Info, "\033[0;32m"},				// green
	{Level::Debug, "\033[0;36m"},				// cyan
};



// operator(), output if type correct
std::string dstring::operator()(Level lv, const std::string &str) const {
	if (lv > levelFlag) return "";
	std::string ret = "";
	if (prefixFlag) {
		// add prefix
		ret = prefixMap.at(lv);
		// add color
		if (colorFlag && lv != Level::None) {			// None without color
			ret = colorMap.at(lv) + ret + "\033[0m";
		}
	}
	return ret + str;
}

//--------------------------------------------------
//			dstringPtr
//--------------------------------------------------


dstringPtr::dstringPtr() {
	ds = dstring::Instance();
}

dstringPtr::~dstringPtr() {
}

std::string dstringPtr::operator()(dstring::Level level_, const std::string &str) const {
	return ds->operator()(level_, str);
}

dstring &dstringPtr::operator*() {
	return *ds;
}

dstring *dstringPtr::operator->() {
	return ds;
}

int dstringPtr::SetLevel(dstring::Level level_) {
	return ds->SetLevel(level_);
}

int dstringPtr::SetColor(bool color_) {
	return ds->SetColor(color_);
}

int dstringPtr::SetPrefix(bool prefix_) {
	return ds->SetColor(prefix_);
}

int dstringPtr::SetColorPrefix() {
	return ds->SetColorPrefix();
}
