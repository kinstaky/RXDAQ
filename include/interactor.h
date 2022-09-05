#ifndef __INTERACTOR_H__
#define __INTERACTOR_H__

#include <string>

namespace rxdaq {

class Interactor {
public:
	
	/// @brief default constructor
	///
	Interactor() = default;

	/// @brief default destructor
	///
	virtual ~Interactor() = default;


	// virtual void Run() = 0;

protected:

};


class CommandParser : public Interactor {
public:

	/// @brief default constructor
	///
	CommandParser() = default;

	/// @brief default destructor
	///
	virtual ~CommandParser() = default;

private:

};


class HelpCommandParser final : public CommandParser {
public:
	
	/// @brief default constructor
	///
	HelpCommandParser() = default;


	/// @brief default destructor
	///
	virtual ~HelpCommandParser() = default;

private:


};


class InfoCommandParser : public CommandParser {
public:

	/// @brief default constructor
	///
	InfoCommandParser() = default;

	/// @brief default destructor
	///
	virtual ~InfoCommandParser() = default;
};


}		// namespace rxdaq
#endif