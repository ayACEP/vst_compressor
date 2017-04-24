#include "utils.h"
#include <stdio.h>
#include <stdarg.h>

int c = 0;

void logInternal(char* format, va_list args)
{
	char buffer[256] = { 0 };
	vsprintf_s(buffer, format, args);
	printf(buffer);
}

void LOG(char* format, ...)
{
#ifndef _DEBUG
	return;
#endif
	va_list args;
	va_start(args, format);
	logInternal(format, args);
	va_end(args);
}

void LOG_PROCESS(char * format, ...)
{
	if (c++ % 100 != 0)
	{
		return;
	}
	char buffer[256] = { 0 };
	va_list args;
	va_start(args, format);
	logInternal(format, args);
	va_end(args);
}

void LOG_PROCESS_FLOW(char * format, ...)
{
	if (c % 100 != 0)
	{
		return;
	}
	char buffer[256] = { 0 };
	va_list args;
	va_start(args, format);
	logInternal(format, args);
	va_end(args);
}

void rangeToNormalize(int min, int max)
{

}

void normalizeToRange(int min, int max)
{

}
