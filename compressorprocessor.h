#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
class CompressorProcessor : public AudioEffect
{
public:
	CompressorProcessor();
	~CompressorProcessor();

	static FUnknown* createInstance(void*)
	{
		return (IAudioProcessor*)new CompressorProcessor();
	}

	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

	tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API process (ProcessData& data) SMTG_OVERRIDE;
	
	//------------------------------------------------------------------------
	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

private:
	void processHalfCycle(float* inBuf, float* outBuf, int begin, int end);

protected:
	bool mBypass;
	ParamValue mThreshold;
	ParamValue mRatio;
	ParamValue mGain;
	ParamValue mAttack;
	ParamValue mRelease;
private:
	float prevIn;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
