#pragma once

#include <stdio.h>

#ifdef _DEBUG
	#define LOG printf
#else 
	#define LOG
#endif