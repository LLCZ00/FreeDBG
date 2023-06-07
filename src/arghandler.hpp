/*
* FreeDBG - Commandline Argument Handler (Header)
*/

#ifndef FREEDBG_ARG_HANDLER
#define FREEDBG_ARG_HANDLER


typedef struct {
	char *target_elf;
	char **target_args;
} DbgArgs;


int parseArguments(int argc, char **argv, DbgArgs &args);


#endif // FREEDBG_ARG_HANDLER
