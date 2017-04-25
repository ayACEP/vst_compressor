#pragma once

#include <pluginterfaces\vst\vsttypes.h>

void LOG(char* format, ...);

void LOG_PROCESS(char* format, ...);

void LOG_PROCESS_FLOW(char * format, ...);

double normalizedValue2dBFS(Steinberg::Vst::ParamValue value);

Steinberg::Vst::ParamValue dBFS2NormalizedValue(double dBFS);
