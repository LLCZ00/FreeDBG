/*
* FreeDBG - Logging/Message Functions
*/


#include <iostream>
#include "logging.hpp"

#include <cstdio>
#include <cstdarg>


void logError(const char *format, ...)
{
	fprintf(stderr, "[!] ");
	va_list arg;
	va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
    fputs("\n", stderr);
}


void logMsg(const char *format, ...)
{
	fprintf(stdout, "[*] ");
	va_list arg;
	va_start(arg, format);
    vfprintf(stdout, format, arg);
    va_end(arg);
    fputs("\n", stdout);
}

