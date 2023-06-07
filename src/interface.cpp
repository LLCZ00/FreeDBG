/*
* FreeDBG - Commandline Interface
*
* TODO:
*	- Make help menu
*	- Make less atrocious, if possible
*/

#include <string>
#include <vector>
#include <sstream> 
#include <cstdio>
#include <iostream>
#include <exception>
#include "interface.hpp" // DebuggerCLI, Command
#include "logging.hpp" // logError, logMsg


static const char *HELP[] = { // Update this and add more detail
	"help(h)",
	"quit(q)",
	"clear",
	"detach",
	"continue(c)",
	"breakpoint(break) ADDR [enable|disable]",
	"step(s)",
	"set [memory(mem)|register(reg)] TARGET VALUE",
	"print [memory(mem)|register(reg)]",
	0
};


/************************
* Command Class Methods *
************************/
std::string &Command::operator[] (size_t i) { return cmdline[i]; }

void Command::getInput()
{
    std::string cmd;
    std::string command;
    std::getline(std::cin, command);
    std::stringstream cmdstream(command);
    while(std::getline(cmdstream, cmd, delim))
    {
        cmdline.push_back(cmd);
    }
    argcount = cmdline.size();
}

int Command::length() { return argcount; }


/**************************** 
* DebuggerCLI Class Methods *
****************************/
DebuggerCLI::DebuggerCLI(Debugger &dbgr) : debugger(&dbgr)
{
	if (!debugger->isActive()) { debugger->start(); }
}


void DebuggerCLI::loop()
{
	printf("\n~ FreeDBG Interactive Interface ~\n(Type 'help' for list of commands)");
	const char *prefix = "\nDBG> ";
	bool running = true; // Might take this out

	while (running && debugger->isActive())
	{
		printf("%s", prefix);
		Command command;
		command.getInput();
		
		if (command.length() == 0) {} // Do nothing
		else if (!command[0].compare("q") || !command[0].compare("quit"))
		{
			debugger->killProcess();
			running = false;
		}
		else if (!command[0].compare("clear")) // Might remove this
		{
			std::system("clear");
		}
		
		else if (!command[0].compare("c") || !command[0].compare("continue"))
		{
			debugger->continueExec();
		}
		else if (!command[0].compare("break") || !command[0].compare("breakpoint"))
		{
			if (command.length() < 2)
			{
				logError("Command 'breakpoint' requires argument 'address' and enable|disable (optional)");
				continue;
			}

			if (!command[1].compare("list"))
			{
				debugger->listBreakpoints();
				continue;
			}

			unsigned long address; // Fix this
			try { address = std::stoul(command[1], 0, 16); }
			catch(...)
			{
				logError("Invalid address '%s'", command[1].c_str());
				continue;
			}

			if (command.length() == 3)
			{
				if (!command[2].compare("enable")) { debugger->setBreakpoint(address); }
				else if (!command[2].compare("disable")) { debugger->unsetBreakpoint(address); }
				else { logError("Invalid breakpoint option '%s'", command[2].c_str()); }
			}
			else // Default: Set/enable
			{ 
				debugger->setBreakpoint(address); 
			} 
		}
		else if (!command[0].compare("s") || !command[0].compare("step"))
		{
			debugger->stepInto();
		}
		else if (!command[0].compare("set"))
		{
			if (command.length() < 3)
			{
				logError("Command 'set' requires argument (register, memory) and value");
				continue;
			}
			
			if (!command[1].compare("reg") || !command[1].compare("register"))
			{
				printf("Set register %s=%s\n", command[2].c_str(), command[3].c_str());
			}
			else if (!command[1].compare("mem") || !command[1].compare("memory"))
			{
				printf("Set memory @0x%s=%s\n", command[2].c_str(), command[3].c_str());
			}
			else
			{
				logError("Invalid set target '%s'", command[1].c_str());
			}
		}
		else if (!command[0].compare("print"))
		{
			if (command.length() < 2)
			{
				logError("Command 'print' requires argument (registers, memory)");
				continue;
			}
			if (!command[1].compare("regs") || !command[1].compare("registers"))
			{
				if (command.length() == 3)
				{
					printf("Print register '%s'\n", command[2].c_str());
				}
				else
				{
					debugger->printRegisters();
				}
			}
			else if (!command[1].compare("mem") || !command[1].compare("memory"))
			{
				if (command.length() < 3)
				{
					logError("Command 'print memory' requires argument 'address' and 'size' (optional)");
					continue;
				}

				unsigned long address; // Fix this
				size_t datasize = 4;
				try { address = std::stoul(command[2], 0, 16); }
				catch(...)
				{
					logError("Invalid address '%s'", command[2].c_str());
					continue;
				}

				if (command.length() == 4)
				{
					try { datasize = std::stoi(command[3]); }
					catch(...)
					{
						logError("Invalid size '%s'", command[3].c_str());
						continue;
					}
				}
				debugger->printMemory(address, datasize);
			}
			else
			{
				logError("Invalid print target '%s'", command[1].c_str());
			}
		}
		else if (!command[0].compare("h") || !command[0].compare("help"))
		{
			for (int i = 0; HELP[i]; i++) 
			{
				printf("%s\n", HELP[i]); 
			}
		}
		else if (!command[0].compare("detach"))
		{
			debugger->detachProcess();
			running = false;
		}
		else
		{
			logError("Unknown command '%s'", command[0].c_str());
		}
	}

}
