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



std::unordered_map<std::string, int> register_map = { // second values from enum in debugger.hpp
    {"eax", R_EAX},
    {"ebx", R_EBX},
    {"ecx", R_ECX},
    {"edx", R_EDX},
    {"esi", R_ESI},
    {"edi", R_EDI},
    {"ebp", R_EBP},
    {"eip", R_EIP},
    {"esp", R_ESP},
    {"cflag", R_CARRY},
    {"zflag", R_ZERO},
    {"sflag", R_SIGN},
    {"oflag", R_OVERFLOW}
};


static const char *HELP[] = { // Update this and add more detail
	"help(h)",
	"\t - Display this list of commands",
	"quit(q)",
	"\t - Kill debugee process and exit FreeDBG",
	"clear",
	"\t - Clear terminal screen",
	"detach",
	"\t - Detach from debugee process and exit FreeDBG",
	"continue/run [to/until ADDR]",
	"\t - Resume execution of debugee process indefinetly or until specified address",
	"breakpoint(break) ADDR [enable|disable|delete]",
	"\t - Set/enable, disable, or delete breakpoint at given address",
	"step(s) [to/until ADDR]",
	"\t - Execute one instruction, or until given address",
	"set %register VALUE",
	"\t - Assign value to register (preceeded by percent sign)",
	"print [ADDRESS SIZE | registers(regs)]",
	"\t - Read/print registers or SIZE bytes of data at given address (Default: 4 bytes)",
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


void DebuggerCLI::loop() // Kind of a trainwreck
{
	printf("\n~ FreeDBG Interactive Interface ~\n(Type 'help' for list of commands)");
	const char *prefix = "\nDBG> ";

	while (debugger->isActive())
	{
		printf("%s", prefix);
		fflush(stdout);
		Command command;
		command.getInput();
		
		if (command.length() == 0) {} // Do nothing
		else if (!command[0].compare("q") || !command[0].compare("quit"))
		{
			debugger->killProcess();
			break;
		}
		else if (!command[0].compare("clear")) // Might remove this
		{
			std::system("clear");
		}
		else if (!command[0].compare("continue") || !command[0].compare("run"))
		{
			if (command.length() > 2)
			{
				if (!command[1].compare("to") || !command[1].compare("until"))
				{
					try { debugger->stepUntil(std::stoul(command[2], 0, 16)); }
					catch(...)
					{
						logError("Invalid address '%s'", command[1].c_str());
						continue;
					}
				}
				else
				{
					logError("Command 'run until' requires argument 'address'");
				}
			}
			else
			{
				debugger->continueExec();
			}
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
				else if (!command[2].compare("delete")) { debugger->deleteBreakpoint(address); }
				else { logError("Invalid breakpoint option '%s'", command[2].c_str()); }
			}
			else // Default: Set/enable
			{ 
				debugger->setBreakpoint(address); 
			} 
		}
		else if (!command[0].compare("s") || !command[0].compare("step"))
		{
			if (command.length() < 2) { debugger->stepInto(); }
			else if (!command[1].compare("to") || !command[1].compare("until"))
			{
				if (command.length() != 3)
				{
					logError("Command 'step until' requires argument 'address'");
					continue;
				}
				try { debugger->stepUntil(std::stoul(command[2], 0, 16)); }
				catch(...)
				{
					logError("Invalid address '%s'", command[1].c_str());
					continue;
				}
			}
			else
			{
				logError("Invalid print target '%s'", command[1].c_str());
			}
		}
		else if (!command[0].compare("set"))
		{
			if (command.length() < 3)
			{
				logError("Command 'set' requires argument (%register or memory_address) and value");
				continue;
			}
			
			if (command[1][0] == '%')
			{
				auto it = register_map.find(command[1].substr(1));
				if (it == register_map.end()) { logError("Unsupported register: %s\n", command[1].c_str()); }
				else
				{
					try { debugger->writeRegister(it->second, std::stoul(command[2], 0, 16)); }
					catch(...)
					{
						logError("Invalid value '%s'", command[2].c_str());
					}
				}
			}
			else
			{
				logError("Invalid 'set' target '%s'", command[1].c_str());
			}
		}
		else if (!command[0].compare("print"))
		{
			if (command.length() < 2)
			{
				logError("Command 'print' requires argument 'registers' or '0xADDRESS')");
				continue;
			}
			if (!command[1].compare("regs") || !command[1].compare("registers"))
			{
				debugger->printRegisters();
			}
			else
			{
				unsigned long address; // Fix this
				size_t datasize = 4;
				try { address = std::stoul(command[1], 0, 16); }
				catch(...)
				{
					logError("Invalid address or 'print' target '%s'", command[1].c_str());
					continue;
				}

				if (command.length() == 3)
				{
					try { datasize = std::stoi(command[2]); }
					catch(...)
					{
						logError("Invalid size '%s'", command[2].c_str());
						continue;
					}
				}
				debugger->printMemory(address, datasize);
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
			break;
		}
		else
		{
			logError("Unknown command '%s'", command[0].c_str());
		}
	}

}

