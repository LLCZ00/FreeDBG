# FreeDBG
### _FreeBSD Debugger_
*FreeDBG* is a simple/crude debugger for FreeBSD-style ELF files. WIP.

## Usage
```
Usage: ./freedbg PROG [ARGS]

Options:
	-h, --help                Show this message and exit.
	PROG [ARGS]               Path of file (and arguments, optionally) to execute and debug
```

## Commands
```
help(h)
	 - Display this list of commands
quit(q)
	 - Kill debugee process and exit FreeDBG
clear
	 - Clear terminal screen
detach
	 - Detach from debugee process and exit FreeDBG
continue/run [to/until ADDR]
	 - Resume execution of debugee process indefinetly or until specified address
breakpoint(break) ADDR [enable|disable|delete]
	 - Set/enable, disable, or delete breakpoint at given address
step(s) [to/until ADDR]
	 - Execute one instruction, or until given address
set %register VALUE
	 - Assign value to register (preceeded by percent sign)
print [ADDRESS SIZE | registers(regs)]
	 - Read/print registers or SIZE bytes of data at given address (Default: 4 bytes)
```
## Known Issues & TODO
- Allow breakpoints to be named, so they can be identified by that instead of their address
- Allow for writing directly to memory
- Add "step over" command
- Currently only compatible with 32-bit executables
- Add option to attach to running process with given PID
