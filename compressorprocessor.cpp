#include "compressorprocessor.h"
#include <math.h>
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
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
	mThreshold = 0.5;
	mRatio = 1;
	mGain = 0.5;
	mAttack = 0;
	mRelease = 0;
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

	mDelayIn[0] = nullptr;
	mDelayIn[1] = nullptr;
	mProcessIn[0] = nullptr;
	mProcessIn[1] = nullptr;
	mTrueRatio[0] = -1;
	mTrueRatio[1] = -1;

	return result;
}

tresult PLUGIN_API CompressorProcessor::terminate()
{
	if (mDelayIn[0]) delete mDelayIn[0];
	if (mDelayIn[1]) delete mDelayIn[1];
	if (mProcessIn[0]) delete mProcessIn[0];
	if (mProcessIn[1]) delete mProcessIn[1];
	return AudioEffect::terminate();
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
		//LOG_PROCESS("numInputs: %d\n", data.numInputs);
		//LOG_PROCESS_FLOW("numSamples: %d\n", data.numSamples);
		//LOG_PROCESS_FLOW("channels: %d\n", numChannels)
		process2(data);
	}	
	return kResultTrue;
}

tresult PLUGIN_API CompressorProcessor::process1(ProcessData& data)
{
	Sample32* ins0 = data.inputs[0].channelBuffers32[0];
	Sample32* outs0 = data.outputs[0].channelBuffers32[0];
	Sample32* ins1 = data.inputs[0].channelBuffers32[1];
	Sample32* outs1 = data.outputs[0].channelBuffers32[1];

	if (mBypass) {
		memcpy(outs0, ins0, data.numSamples * sizeof(Sample32));
		memcpy(outs1, ins1, data.numSamples * sizeof(Sample32));
		return kResultFalse;
	}

	for (int32 i = 0; i < data.numSamples; i++)
	{
		Sample32 in0 = ins0[i] > 0 ? ins0[i] : -ins0[i];
		Sample32 in1 = ins1[i] > 0 ? ins1[i] : -ins1[i];
		Sample32 peek = in0 > in1 ? in0 : in1;
		if (peek > mEnv) mEnv = peek;
	}
	return kResultTrue;
}

tresult PLUGIN_API CompressorProcessor::process2(ProcessData& data)
{
	SpeakerArrangement arr;
	getBusArrangement(kOutput, 0, arr);
	int32 numChannels = SpeakerArr::getChannelCount(arr);

	// bypass copy
	if (mBypass) {
		for (int i = 0; i < numChannels; i++) {
			Sample32* ins = data.inputs[0].channelBuffers32[i];
			Sample32* outs = data.outputs[0].channelBuffers32[i];
			memcpy(outs, ins, data.numSamples * sizeof(Sample32));
		}
		return kResultFalse;
	}

	//LOG_PROCESS("%d, %d\n", data.numSamples, data.numInputs);
	// alloc memory
	if (mDelayIn[0] == nullptr)
	{
		// delay 15ms
		mDelaySampleSize = (int)(data.processContext->sampleRate / 1000 * 5);
		mDelaySampleSize = mDelaySampleSize < data.numSamples ? mDelaySampleSize : data.numSamples;

		mDelayIn[0] = new Sample32[mDelaySampleSize]{ 0 };
		mDelayIn[1] = new Sample32[mDelaySampleSize]{ 0 };
		mProcessIn[0] = new Sample32[mDelaySampleSize + data.numSamples]{ 0 };
		mProcessIn[1] = new Sample32[mDelaySampleSize + data.numSamples]{ 0 };
	}

	for (int i = 0; i < numChannels; i++) {
		Sample32* ins = data.inputs[0].channelBuffers32[i];
		Sample32* outs = data.outputs[0].channelBuffers32[i];

		// calc processIn
		//memcpy(mProcessIn[i], mDelayIn[i], mDelaySampleSize * sizeof(Sample32));
		//memcpy(mProcessIn[i] + mDelaySampleSize, ins, data.numSamples * sizeof(Sample32));

		//processInternal(i, mProcessIn[i], mDelaySampleSize + data.numSamples, outs, data.numSamples);

		//memcpy(mDelayIn[i], ins + (data.numSamples - mDelaySampleSize), mDelaySampleSize * sizeof(Sample32));
	}

	return kResultTrue;
}

void CompressorProcessor::processInternal(int inIndex, Sample32* processIns, int processInSize, Sample32* outs, int outSize)
{
	Sample32 prevIn = 0;
	int begin = 0;
	int processedSize = 0;
	for (int i = 0; i < processInSize; i++) {
		// find halfcycle and process
		if ((prevIn < 0 && processIns[i] >= 0) 
				|| (prevIn > 0 && processIns[i] <= 0) 
				|| (i == processInSize - 1)) {
			// process final sample of this frame
			if (i == processInSize - 1) {
				i = i + 1;
			}
			processHalfCycle(inIndex, processIns, begin, i, outs, outSize);
			processedSize += (i - begin);
			begin = i;
			if (processedSize > outSize) {
				break;
			}
		}
		prevIn = processIns[i];
	}
}

void CompressorProcessor::processHalfCycle(int inIndex, Sample32* processIns, int inBegin, int inEnd, Sample32* outs, int outSize)
{
	float trueRatio = 1;
	if (inBegin == 0 && mTrueRatio[inIndex] != -1) {
		trueRatio = mTrueRatio[inIndex];
	} else {
		// find max in half cycle
		Sample32 max = 0;
		Sample32 in = 0;
		for (int i = inBegin; i < inEnd; i++) {
			in = processIns[i] > 0 ? processIns[i] : -processIns[i];
			max = in > max ? in : max;
		}
		Sample32 over = max - mThreshold;
		if (over > 0) {
			trueRatio = ((over / ParamUtils::get_ratio_range().toUsefulValue(mRatio)) + mThreshold) / max;
		} else {
			trueRatio = 1;
		}
	}
	// process every sample
	inEnd = inEnd < outSize ? inEnd : outSize;
	for (int i = inBegin; i < inEnd; i++) {
		outs[i] = processIns[i] * trueRatio;
	}
	mTrueRatio[inIndex] = trueRatio;
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
