#include "compressorprocessor.h"
#include <algorithm>
#include <cstdlib>
#include <math.h>
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "ParamTag.h"
#include "ParamUtils.h"
#include "ids.h"
#include "utils.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
CompressorProcessor::CompressorProcessor ()
{
	mBypass = false;
	mThreshold = 1;
	mRatio = 1;
	mGain = 0.5;
	mAttack = 0;
	mRelease = 0;
	prevIn = 0;
	setControllerClass (CompressorControllerUID);
}

CompressorProcessor::~CompressorProcessor()
{
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::initialize (FUnknown* context)
{
	LOG("CompressorProcessor::initialize\n");
	tresult result = AudioEffect::initialize (context);
	if (result == kResultTrue)
	{
		addAudioInput (STR16 ("AudioInput"), SpeakerArr::kStereo);
		addAudioOutput (STR16 ("AudioOutput"), SpeakerArr::kStereo);
	}
	return result;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::setBusArrangements (SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts)
{
	LOG("CompressorProcessor::setBusArrangements\n");
	// we only support one in and output bus and these buses must have the same number of channels
	if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
		return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::setActive (TBool state)
{
	LOG("CompressorProcessor::setActive\n");
	SpeakerArrangement arr;
	if (getBusArrangement (kOutput, 0, arr) != kResultTrue)
		return kResultFalse;
	int32 numChannels = SpeakerArr::getChannelCount (arr);
	if (numChannels == 0)
		return kResultFalse;

	return AudioEffect::setActive (state);
}
int i = 0;
//-----------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::process (ProcessData& data)
{
	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData (index);
			if (paramQueue)
			{
				ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount ();
				switch (paramQueue->getParameterId ())
				{
				case ParamTag::kBypassId:
					if (paramQueue->getPoint (numPoints - 1,  sampleOffset, value) == kResultTrue)
					{
						mBypass = (value > 0.5f);
					}
					break;
				case ParamTag::kThresholdId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						mThreshold = value;
					}
					break;
				case ParamTag::kRatioId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						mRatio = value;
					}
					break;
				case ParamTag::kGainId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						mGain = value;
					}
					break;
				}
			}
		}
	}

	if (data.numSamples > 0)
	{
		SpeakerArrangement arr;
		getBusArrangement (kOutput, 0, arr);
		int32 numChannels = SpeakerArr::getChannelCount (arr);
		
		//LOG_PROCESS("numInputs: %d\n", data.numInputs);
		//LOG_PROCESS_FLOW("numSamples: %d\n", data.numSamples);
		//LOG_PROCESS_FLOW("channels: %d\n", numChannels);
		
		for (int32 channel = 0; channel < numChannels; channel++)
		{
			float* inputChannel = data.inputs[0].channelBuffers32[channel];
			float* outputChannel = data.outputs[0].channelBuffers32[channel];

			int begin = 0;

			for (int32 i = 0; i < data.numSamples; i++)
			{
				float &in = inputChannel[i];
				float &out = outputChannel[i];
				//LOG("%.4f ", in);
				// bypass copy only
				if (mBypass)
				{
					out = in;
					continue;
				}
				else
				{
					// 1.get half cycle max
					if ((in > 0 && prevIn <= 0) || (in < 0 && prevIn >= 0)) {
						processHalfCycle(inputChannel, outputChannel, begin, i);
						begin = i;// record half cycle begin point
					}
				}
				prevIn = in;
			}
		}
	}	
	return kResultTrue;
}

void CompressorProcessor::processHalfCycle(float* inBuf, float* outBuf, int begin, int end)
{
	if (end <= begin)
	{
		return;
	}
	LOG("1 %d %d\n", begin, end);
	// 1.find max
	float max = 0;
	for (int i = begin; i < end; i++)
	{
		outBuf[i] = abs(inBuf[i]);
		float &in = outBuf[i];
		max = in > max ? in : max;
	}
	LOG_PROCESS("%f\n", max);
	// 2.calc reduce ratio
	float over = max - mThreshold;
	float reduceRatio = 1;
	if (over > 0)
	{
		reduceRatio = (over / ParamUtils::get_ratio_range().toUsefulValue(mRatio) + mThreshold) / max;
	}
	else
	{
		return;
	}
	LOG_PROCESS("2\n");
	// 3.reduce
	for (int i = begin; i < end; i++)
	{
		outBuf[i] = inBuf[i] < 0 ? -outBuf[i] * reduceRatio : outBuf[i] * reduceRatio;
	}
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::setState (IBStream* state)
{
	LOG("CompressorProcessor::setState\n");
	if (!state)
	{
		return kResultFalse;
	}

	// called when we load a preset, the model has to be reloaded

	int32 savedBypass = 0;
	ParamValue savedThreshold = 0.f;
	ParamValue savedRatio = 0.f;
	ParamValue savedGain = 0.f;
	ParamValue savedAttack = 0.f;
	ParamValue savedRelease = 0.f;

	if (state->read(&savedBypass, sizeof(int32)) != kResultOk)
	{
		return kResultFalse;
	}
	if (state->read (&savedThreshold, sizeof (ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}
	if (state->read (&savedRatio, sizeof (ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}
	if (state->read (&savedGain, sizeof (ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}
	if (state->read(&savedAttack, sizeof(ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}
	if (state->read(&savedRelease, sizeof(ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}

#if BYTEORDER == kBigEndian
	SWAP_32(savedBypass)
	SWAP_32(savedThreshold)
	SWAP_32(savedRatio)
	SWAP_32(savedGain)
	SWAP_32(savedAttack)
	SWAP_32(savedRelease)
#endif

	LOG("set bypass: %d\n", savedBypass);
	LOG("set threshold: %f\n", savedThreshold);
	LOG("set ratio: %f\n", savedRatio);
	LOG("set gain: %f\n", savedGain);
	LOG("set attack: %f\n", savedAttack);
	LOG("set release: %f\n", savedRelease);

	mBypass = savedBypass > 0;
	mThreshold = savedThreshold;
	mRatio = savedRatio;
	mGain = savedGain;
	mAttack = savedAttack;
	mRelease = savedRelease;

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::getState (IBStream* state)
{
	LOG("CompressorProcessor::getState\n");
	// here we need to save the model

	int32 toSaveBypass = mBypass ? 1 : 0;
	ParamValue toSaveThreshold = mThreshold;
	ParamValue toSaveRatio = mRatio;
	ParamValue toSaveGain = mGain;
	ParamValue toSaveAttack = mAttack;
	ParamValue toSaveRelease = mRelease;

#if BYTEORDER == kBigEndian
	SWAP_32(toSaveBypass)
	SWAP_32(toSaveThreshold)
	SWAP_32(toSaveRatio)
	SWAP_32(toSaveGain)
	SWAP_32(toSaveAttack)
	SWAP_32(toSaveRelease)
#endif

	LOG("bypass: %d\n", toSaveBypass);
	LOG("threshold: %f\n", toSaveThreshold);
	LOG("ratio: %f\n", toSaveRatio);
	LOG("gain: %f\n", toSaveGain);
	LOG("attack: %f\n", toSaveAttack);
	LOG("release: %f\n", toSaveRelease);

	state->write(&toSaveBypass, sizeof(int32));
	state->write(&toSaveThreshold, sizeof (ParamValue));
	state->write(&toSaveRatio, sizeof(ParamValue));
	state->write(&toSaveGain, sizeof(ParamValue));
	state->write(&toSaveAttack, sizeof(ParamValue));
	state->write(&toSaveRelease, sizeof(ParamValue));

	return kResultTrue;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
