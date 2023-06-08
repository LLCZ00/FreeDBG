/*
* FreeDBG - Debugger Class (Header)
*/

#ifndef FREEDBG_DEBUGGER
#define FREEDBG_DEBUGGER

#include <machine/reg.h>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

#define BYTE uint8_t

#if defined(__LP64__)
typedef uint32_t WORD;
typedef uint64_t DWORD;
#else
typedef uint16_t WORD;
typedef uint32_t DWORD;
#endif // (__LP64__)

typedef DWORD ADDR;


enum {
    R_EAX = 0,
    R_EBX,
    R_ECX,
    R_EDX,
    R_ESI,
    R_EDI,
    R_EBP,
    R_EIP,
    R_ESP
};


class Breakpoint {
private:
	int child_pid;
	bool enabled = false;
	ADDR address;
	BYTE saved_instruction = 0;

public:
	Breakpoint(int pid, ADDR addr);
	bool isEnabled();
	ADDR getAddress();
	bool enable();
	void disable();
};


class Debugger {
private:
	int child_pid;
	Breakpoint *current_breakpoint = NULL;
	volatile bool active = false;
	struct reg registers;
	std::unordered_map<ADDR,Breakpoint> breakpoints;
	bool waitOnChild();

public:
	Debugger(int pid);
	bool isActive();
	void start();
	void killProcess();
	void detachProcess();
	void continueExec();

	void setBreakpoint(ADDR address);
	void unsetBreakpoint(ADDR address);
	void deleteBreakpoint(ADDR address);
	void listBreakpoints();

	void stepInto();
	
	void writeRegister(int regcode, DWORD value);
	void writeMemory();
	void printRegisters();
	void printMemory(ADDR address, size_t size);
	
};


#endif // FREEDBG_DEBUGGER
