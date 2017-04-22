#include "utils.h"
#include <stdio.h>
#include <stdarg.h>

void LOG(char* format, ...)
{
#ifndef _DEBUG
	return;
#endif
	char buffer[256] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf(buffer);
	va_end(args);
}
