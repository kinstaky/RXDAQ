/* Manager.h
 *
 *
 *
 */

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "lib/args.hxx"
#include "dstring.h"
#include "Config.h"

#include <string>
#include <vector>
#include <iostream>
#include <map>


const int MAX_CRATE_NUM = 4;


struct Task {
	enum class TType {
		// init system
		Init = 1,
		// boot modules
		Boot = 2,
		// read parameter
		Read = 3,
		// write parameter
		Write = 4
	};


	TType Type;
	std::vector<int> Module;
	std::vector<int> Channel;
	std::vector<std::string> Name;
	bool List;

	friend std::ostream& operator<<(std::ostream& os, const Task &task);
};



class Manager {
public:
	Manager();
	virtual ~Manager();

	int RunManager();
	int ParseCLI(int argc, char **argv);

	int PrintTasks() const;
	int PrintCrateInfo() const;

private:
	// task list
	std::multimap<Task::TPriority, Task> tasks;

	// debug output string
	dstring ds;

	// configs
	Config config;

	// command read and write funciton, handler for the args::Command
	void cmdReadWriteFunc(args::Subparser &parser, bool read);
	void cmdBootFunc(args::Subparser &parser);
	int checkModuleInfo() const;
	int checkBootRet(int retval) const;
};



#endif