#include "public.sdk\source\main\pluginfactoryvst3.h"
#include "compressorcontroller.h"
#include "compressorprocessor.h"
#include "compressorids.h"
#include "version.h"
#include <Windows.h>

#define stringPluginName "Compressor"

BEGIN_FACTORY("ayACEP",
	"",
	"mailto:srx1982@gmail.com",
	PFactoryInfo::kNoFlags)

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::CompressorProcessorUID),
		PClassInfo::kManyInstances,
		kVstAudioEffectClass,
		stringPluginName,
		Vst::kDistributable,
		"Fx|Compressor",
		FULL_VERSION_STR,		// Plug-in version (to be changed)
		kVstVersionString,
		Steinberg::Vst::CompressorProcessor::createInstance)

	DEF_CLASS2(INLINE_UID_FROM_FUID(Steinberg::Vst::CompressorControllerUID),
		PClassInfo::kManyInstances,
		kVstComponentControllerClass,
		stringPluginName "Controller",	// controller name (could be the same than component name)
		0,						// not used here
		"",						// not used here
		FULL_VERSION_STR,		// Plug-in version (to be changed)
		kVstVersionString,
		Steinberg::Vst::CompressorController::createInstance)

END_FACTORY

bool InitModule() {
#ifdef _DEBUG
	AllocConsole();
	freopen("conout$", "w", stdout);
#endif
	return true;
}

bool DeinitModule() {
#ifdef _DEBUG
	FreeConsole();
#endif
	return true;
}
