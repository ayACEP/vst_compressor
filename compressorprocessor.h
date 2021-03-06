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
	tresult PLUGIN_API terminate() override;
	tresult PLUGIN_API setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;

	tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;
	tresult PLUGIN_API process (ProcessData& data) SMTG_OVERRIDE;
	
	//------------------------------------------------------------------------
	tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

private:
	tresult PLUGIN_API process1(ProcessData& data);
	tresult PLUGIN_API process2(ProcessData& data);
	void processInternal(int inIndex, Sample32* ins, int inSize, Sample32* outs, int outSize);
	void processHalfCycle(int inIndex, Sample32* processIns, int begin, int end, Sample32* outs, int outSize);

protected:
	bool mBypass;
	ParamValue mThreshold;
	ParamValue mRatio;
	ParamValue mGain;
	ParamValue mAttack;
	ParamValue mRelease;
private:
	Sample32 mEnv;
	Sample32* mDelayIn[2];
	Sample32* mProcessIn[2];
	int mDelaySampleSize;
	float mTrueRatio[2];
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
