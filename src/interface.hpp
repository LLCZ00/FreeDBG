/*
* FreeDBG - Commandline Interface (Header)
*/

#ifndef FREEDBG_DBG_INTERFACE
#define FREEDBG_DBG_INTERFACE

#include <vector>
#include <cstddef>
#include "debugger.hpp"


class Command {
private:
    std::vector<std::string> cmdline;
    int argcount = 0;
    const char delim = ' ';

public:
	std::string &operator[] (size_t i);
    void getInput();
    int length();
};


class DebuggerCLI {
private:
	Debugger *debugger;

public:
	DebuggerCLI(Debugger &dbgr);
	void loop();
};


#endif // FREEDBG_DBG_INTERFACE
