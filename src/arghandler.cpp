/*
* FreeDBG - Commandline Argument Handler
*
* TODO:
*	- Add arg for setting custom envp
*/

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <elf.h>
#include "logging.hpp" // logError, logMsg
#include "arghandler.hpp"


#if defined(__LP64__)
#define ElfHdr Elf64_Ehdr
#define WORD uint64_t
#else
#define ElfHdr Elf32_Ehdr
#define WORD uint32_t
#endif // (__LP64__)


static const char *HELP[] = {
	"~ FreeDbg v1.0.0, by LLCZ00 ~",
	"",
	"Description: A basic (crude) debugger for FreeBSD",
	"",
	"Usage: ./freedbg PROG [ARGS]",
	"",
	"Options:", 
	"\t-h, --help                Show this message and exit.",
	"\tPROG [ARGS]               Path of file (and arguments, optionally) to execute and debug"
	"",
	0
};

static const char *TRYMSG = "Try './freedbg --help' for more information\n";


static int checkFile(const char *filepath)
{
	ElfHdr header;
	FILE *file = fopen(filepath, "rb");
	if (!file)
	{
		logError("Unable to find file '%s'\n%s", filepath, TRYMSG);
		return -1;
	}
	fread(&header, sizeof(header), 1, file);
	fclose(file);
	if (memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) // Verify ELF signature
	{
		logError("Invalid ELF file '%s'\n%s", filepath, TRYMSG);
		return -1;
	}
	return 0;
}


int parseArguments(int argc, char **argv, DbgArgs &args)
{

	if (argc < 2)
	{
		fprintf(stderr, "Usage: ./freedbg PROG [ARGS]\n%s", TRYMSG);
		return -1;
	}

	/* Assign default values */
	args.target_elf = 0;
	args.target_args = 0;

	int index = 1;

	while(1)
	{
		if (index == argc) { break; }

		/* Display help message */
		else if ((strncmp(argv[index], "-h\0", 3) == 0) || (strncmp(argv[index], "--help\0", 7)) == 0)
		{
			for (int i = 0; HELP[i]; i++) 
			{
				fprintf(stderr, "%s\n", HELP[i]); 
			}
			return -1;
		}

		/* Set target ELF */
		else
		{

			if (checkFile(argv[index]) < 0)
			{
				logError("Unknown command '%s'\n%s", argv[index], TRYMSG);
				return -1;
			}
			else
			{
				args.target_elf = argv[index];
				args.target_args = &argv[index];
				return 0;
			}
		}
	}
	return 0;
}