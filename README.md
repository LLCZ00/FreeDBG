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
    - Kill debugee process and exit debugger
clear
    - Clear terminal screen
detach
    - Detach from debugee process and exit debugger
continue(c)
    - Resume execution of debugee process
breakpoint(break) ADDR [enable|disable|delete]
    - Set/enable, disable, or delete breakpoint
step(s)
    - Step 1 instruction
write %register VALUE
    - Set register value (register names preceded by percent sign, start value with 0x to interpret as hex)
print [memory(mem) ADDR SIZE | registers(regs)]
    - Read/print SIZE bytes (default: 4) starting from memory address, or all registers
```
## Known Issues & TODO
- Allow breakpoints to be named, so they can be identified by that instead of their address
- Add command to write to debugee memory
- Add ability to edit/print register flags
- Allow for writing directly to memory
- Add "step until ADDRESS" command
- Add "step over" command
- Currently basically only works with x86 executables
