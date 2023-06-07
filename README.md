# FreeDBG
### _FreeBSD Debugger_
*FreeDBG* is a simple/crude debugger for FreeBSD-style ELF files. WIP.

## Known Issues & TODO
- Currently segfaults shortly after executing debugee lol
    - Possible issue with buffer flushing
    - Plans to run this on other machines are in place
- Add way for "naming" breakpoints
- Add command to write to debugee memory
- Currently basically only works with x86 executables
- Add section in readme for debugging commands
