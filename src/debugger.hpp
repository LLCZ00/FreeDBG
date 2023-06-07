/*
* FreeDBG - Debugger Class (Header)
*/

#ifndef FREEDBG_DEBUGGER
#define FREEDBG_DEBUGGER


#include <cstdint>
#include <vector>
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


class Breakpoint {
private:
	int child_pid;
	bool enabled = false;
	ADDR address;
	BYTE saved_instruction = 0;

public:
	Breakpoint(int pid, ADDR addr);
	bool operator== (DWORD addr);
	bool isEnabled();
	ADDR getAddress();
	bool enable();
	void disable();
};


class Debugger {
private:
	int child_pid;
	volatile bool active = false;
	std::vector<Breakpoint> breakpoints; // Would unordered_map be more efficient?
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
	void stepInto();
	void listBreakpoints();
	void printRegisters();
	void printMemory(ADDR address, size_t size);
	
};


#endif // FREEDBG_DEBUGGER
