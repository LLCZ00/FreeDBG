/*
* FreeDBG - FreeBSD Debugger
*
* TODO:
*	- Read from "script" conf file to automatically run interface commands
*	- 
*/

#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ptrace.h>
#include "logging.hpp" // logError, logMsg
#include "arghandler.hpp" // DbgArgs, parseArguments
#include "debugger.hpp" // Debugger
#include "interface.hpp" // DebuggerCLI


int main(int argc, char **argv)
{
	DbgArgs args;
	if (parseArguments(argc, argv, args) < 0) { return 1; }

	int pid = fork();
	if (pid < 0)
	{
		logError("Error occured while forking");
	}
	else if (pid == 0)
	{
		if (ptrace(PT_TRACE_ME, 0, 0, 0) < 0)
		{
			logError("Error establishing ptrace");
			return 1;
		}
		execv(args.target_elf, args.target_args);
	}
	else
	{
		Debugger debugger(pid);
		DebuggerCLI cli(debugger);
		cli.loop();
	}

	logMsg("FreeDBG exited gracefully");	
	return 0;
}



