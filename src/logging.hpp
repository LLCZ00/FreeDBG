/*
* FreeDBG - Logging/Message Functions (Header)
*/

#ifndef FREEDBG_LOGGING_HPP
#define FREEDBG_LOGGING_HPP


#include <cstdarg>


void logError(const char *format, ...);
void logMsg(const char *format, ...);


#endif // FREEDBG_LOGGING_HPP
