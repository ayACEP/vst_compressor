#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

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

double normalizedValue2dBFS(Steinberg::Vst::ParamValue value)
{
	return 20.0 * log10(fabs(value) / 1.0);
}

Steinberg::Vst::ParamValue dBFS2NormalizedValue(double dBFS)
{
	return pow(10.0, dBFS / 20.0) * 1.0;
}
