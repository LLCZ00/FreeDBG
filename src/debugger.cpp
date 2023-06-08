/*
* FreeDBG - Debugger Class
*
* TODO:
*	- Standardize x86 and x64 register structs
*	- Make printRegisters function less shitty
*	- Take endianness into account (lil endian is default right now)
*/

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <machine/reg.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include "logging.hpp" // logError, logMsg
#include "debugger.hpp" // Debugger, Breakpoint, BYTE, WORD, DWORD, ADDR

// int ptrace(int request, pid_t pid, caddr_t addr, int data);
// http://fxr.watson.org/fxr/source/sys/signal.h?v=FREEBSD-8-3


/***************************
* Breakpoint Class Methods *
***************************/
Breakpoint::Breakpoint(int pid, ADDR addr) : child_pid(pid), address(addr) {}

bool Breakpoint::isEnabled() { return enabled; }

ADDR Breakpoint::getAddress() { return address; }

bool Breakpoint::enable()
{
	if (enabled) { return true; }

	DWORD instructions = ptrace(PT_READ_I, child_pid, (caddr_t)address, 0);
	if (instructions == -1) { return false; }

	/* Assuming little endianness */
	saved_instruction = static_cast<BYTE>(instructions & 0xFF);
	DWORD instruction_w_interrupt = ((instructions & ~0xFF) | 0xcc); // INT3 opcode

	if (ptrace(PT_WRITE_I, child_pid, (caddr_t)address, instruction_w_interrupt) < 0) { return false; }

	enabled = true;
	return true;
}

void Breakpoint::disable()
{
	if (enabled)
	{
		DWORD instructions = ptrace(PT_READ_I, child_pid, (caddr_t)address, 0);
		DWORD og_instruction = ((instructions & ~0xFF) | saved_instruction);
		ptrace(PT_WRITE_I, child_pid, (caddr_t)address, og_instruction);
		enabled = false;
	}
}


/*************************
* Debugger Class Methods *
*************************/
Debugger::Debugger(int pid) : child_pid(pid) {}

bool Debugger::isActive() { return active; }

bool Debugger::waitOnChild()
{
	int waitstatus;
	if (waitpid(child_pid, &waitstatus, 0) < 0)
	{
		logError("Error occured while waiting for child process");
		active = false;
	}
	else if (WIFEXITED(waitstatus))
	{
		logMsg("Process %d has exited: %d", child_pid, WEXITSTATUS(waitstatus));
		active = false;
	}
	else if (WIFSIGNALED(waitstatus))
	{
		logMsg("Process terminated by signal: %d", WTERMSIG(waitstatus));
		active = false;
	}
	else if (WIFSTOPPED(waitstatus))
	{
		if (WSTOPSIG(waitstatus) == 11) // Might flesh out later
		{
			logError("Process stopped by signal: SIGSEGV (Segmentation fault)");
			active = false;
		}
		else if (WSTOPSIG(waitstatus) == 5) // SIGTRAP (trace trap)
		{
			ptrace(PT_GETREGS, child_pid, (caddr_t)&registers, 0);
			auto it = breakpoints.find(registers.r_eip-1);
			if (it != breakpoints.end())
			{
				if (it->second.isEnabled())
				{
					logMsg("Stopped on breakpoint @0x%X", it->first);
					current_breakpoint = &it->second;
					current_breakpoint->disable();
					registers.r_eip--;
					ptrace(PT_SETREGS, child_pid, (caddr_t)&registers, 0);
				}
			}
		}
	}
	return active;
}

void Debugger::start()
{
	active = true;
	if (!waitOnChild()) { return; }
	logMsg("Attached to process %d", child_pid);
	logMsg("Stopped @0x%X", registers.r_eip);
}

void Debugger::killProcess()
{
	if (ptrace(PT_KILL, child_pid, 0, 0) < 0)
	{
		logError("Failed to kill child process %d", child_pid);
	}
	else
	{
		logMsg("Killed child process %d", child_pid);
		active = false;
	}
}

void Debugger::detachProcess()
{
	ptrace(PT_DETACH, child_pid, 0, 0);
	logMsg("Detached from child process %d", child_pid);
	active = false;
}

void Debugger::continueExec()
{
	if (current_breakpoint != NULL) // Step 1 instruction, re-enable breakpoint, then continue
	{
		Breakpoint *bp = current_breakpoint;
		ptrace(PT_STEP, child_pid, (caddr_t)1, 0);
		if (!waitOnChild()) { return; }
		bp->enable();

		if (bp == current_breakpoint) { current_breakpoint = NULL; } // Just return if another breakpoint is immediatly after the last one
		else { return; }
	}
	ptrace(PT_CONTINUE, child_pid, (caddr_t)1, 0);
	waitOnChild();
}


void Debugger::setBreakpoint(ADDR address)
{
	auto it = breakpoints.find(address);
	if (it != breakpoints.end())
	{
		Breakpoint bp = it->second;
		if (bp.isEnabled()) { logMsg("Breakpoint @0x%X already enabled", address); }
		else
        {
            if (bp.enable()) { logMsg("Breakpoint @0x%X enabled", address); }
            else
            {
                logError("Unable to enable breakpoint @0x%X (removing from list)", address);
                breakpoints.erase(address);
            }
        }
	}
	else
    {
        Breakpoint bp(child_pid, address);
        if (bp.enable())
        {
            logMsg("Breakpoint @0x%X set/enabled", address);
            breakpoints.emplace(address, bp);
        }
        else { logError("Unable to set breakpoint @0x%X", address); }
    }
}

void Debugger::unsetBreakpoint(ADDR address)
{
	auto it = breakpoints.find(address);
	if (it != breakpoints.end())
	{
		it->second.disable();
		logMsg("Breakpoint @0x%X disabled", address);
	}
	else
	{
		logError("No breakpoint set @0x%X", address);
	}
}

void Debugger::deleteBreakpoint(ADDR address)
{
	if (breakpoints.find(address) != breakpoints.end())
    {
        breakpoints.erase(address);
        logMsg("Breakpoint @0x%X deleted", address);
    }
    else { logError("No breakpoint set @0x%X", address); }
}

void Debugger::listBreakpoints()
{
	for (auto &addr_bp: breakpoints)
    {
        ADDR address = addr_bp.first;
        Breakpoint bp = addr_bp.second;
        printf("Breakpoint @0x%X: ", address);
        if (bp.isEnabled()) { puts("Enabled"); }
        else { puts("Disabled"); }
    }
}


void Debugger::stepInto()
{
	if (current_breakpoint != NULL)
	{
		Breakpoint *bp = current_breakpoint;
		ptrace(PT_STEP, child_pid, (caddr_t)1, 0);
		if (!waitOnChild()) { return; }
		bp->enable();
		if (bp == current_breakpoint) { current_breakpoint = NULL; } // Just return if another breakpoint is immediatly after the last one
		else { return; }
	}
	else
	{
		ptrace(PT_STEP, child_pid, (caddr_t)1, 0);
		if (!waitOnChild()) { return; }
	}

	if (current_breakpoint == NULL) { logMsg("Stopped @0x%X", registers.r_eip); }
}

void Debugger::writeRegister(int regcode, DWORD value)
{
	struct reg regs;
	ptrace(PT_GETREGS, child_pid, (caddr_t)&regs, 0);
	switch (regcode)
	{
		case R_EAX:
			regs.r_eax = value;
			break;
		case R_EBX:
			regs.r_ebx = value;
			break;
		case R_ECX:
			regs.r_ecx = value;
			break;
		case R_EDX:
			regs.r_edx = value;
			break;
		case R_ESI:
			regs.r_esi = value;
			break;
		case R_EDI:
			regs.r_edi = value;
			break;
		case R_EBP:
			regs.r_ebp = value;
			break;
		case R_EIP:
			regs.r_eip = value;
			break;
		case R_ESP:
			regs.r_esp = value;
			break;
		default:
			break;
	}
	ptrace(PT_SETREGS, child_pid, (caddr_t)&regs, 0);
}

void Debugger::writeMemory()
{

}

void Debugger::printRegisters()
{
	struct reg regs;
	ptrace(PT_GETREGS, child_pid, (caddr_t)&regs, 0);
	printf("EAX: %X\n", regs.r_eax);
	printf("EBX: %X\n", regs.r_ebx);
	printf("ECX: %X\n", regs.r_ecx);
	printf("EDX: %X\n", regs.r_edx);
	printf("ESI: %X\n", regs.r_esi);
	printf("EDI: %X\n", regs.r_edi);
	printf("EBP: %X\n", regs.r_ebp);
	printf("EIP: %X\n", regs.r_eip);
	printf("ESP: %X\n", regs.r_esp);
}

void Debugger::printMemory(ADDR address, size_t size)
{
	BYTE buffer[size];
	struct ptrace_io_desc io_desc;
	io_desc.piod_op = PIOD_READ_D;
	io_desc.piod_offs = (void *)address;
	io_desc.piod_addr = (void *)buffer;
	io_desc.piod_len = size;

	if (ptrace(PT_IO, child_pid, (caddr_t)&io_desc, 0) < 0)
	{
		logError("Unable to read from 0x%X", address);
		return;
	}

	int8_t rowlength = 16; // Bytes per line
	int8_t columnsize = 8; // Where to put space between output in each line
	uint16_t linenumber = 0;
	uint8_t ascii_chars[rowlength]; // Characters for ascii output after each line

	std::memset(ascii_chars, '.', rowlength);

	printf("%04X: ", linenumber);

	for (size_t offset = 0; offset < size; offset++)
	{
		printf("%02X ", buffer[offset]);

		if ( buffer[offset] > 0x20 && buffer[offset] <= 0x7e )
		{
			ascii_chars[offset % rowlength] = buffer[offset];
		}

		if ( (((offset + 1) % rowlength) == 0) || (offset + 1 == size) ) 
		{
			// Align ascii columns
			for (int cursor = (((offset) % rowlength) + 1) * 3; cursor < rowlength*3; cursor++)
			{
				printf(" ");
			}

			printf("\t| ");

			// Print ascii representation
			for (int i = 0; i < rowlength; i++)
			{
				printf("%c ", ascii_chars[i]);
			}
			printf("|\n");

			// Print new linenumber and refill ascii_chars with periods
			if (offset + 1 != size)
			{
				printf("%04X: ", linenumber + (offset + 1));
				std::memset(ascii_chars, '.', rowlength);
			}			
		}
	}
}
