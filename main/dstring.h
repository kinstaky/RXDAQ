/*
 * dstring.h
 */


#ifndef __DSTRING_H__
#define __DSTRING_H__

#include <iostream>
#include <string>
#include <map>


class dstring {
public:
	enum class Level {
		None = 0,
		Error,
		Warning,
		Info,
		Debug
	};

	static dstring *Instance();

	virtual ~dstring();

	int SetLevel(Level lv);
	int SetColor(bool color = true);
	int SetPrefix(bool prefix = true);
	int SetColorPrefix();
	std::string operator()(Level lv, const std::string &str) const;

protected:
	dstring(Level lv = Level::None, bool cf = false, bool pf = false);
	dstring(const dstring &other);
	const dstring &operator=(const dstring &rhs);

private:
	static dstring *instance;

	Level levelFlag;
	bool colorFlag;
	bool prefixFlag;

	// prefix map
	static const std::map<Level, std::string> prefixMap;
	// color map
	static const std::map<Level, std::string> colorMap;
};


class dstringPtr {
public:
	dstringPtr();
	virtual ~dstringPtr();

	int SetLevel(dstring::Level level_);
	int SetColor(bool color_ = true);
	int SetPrefix(bool prefix_ = true);
	int SetColorPrefix();

	std::string operator()(dstring::Level level_, const std::string &str_) const;
	dstring &operator*();
	dstring *operator->();
private:
	dstring *ds;
};

#endif