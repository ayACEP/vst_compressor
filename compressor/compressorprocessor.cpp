//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/adelay/source/adelayprocessor.cpp
// Created by  : Steinberg, 06/2009
// Description : 
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2017, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "compressorprocessor.h"
#include "compressorids.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "utils.h"
#include <algorithm>
#include <cstdlib>

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
CompressorProcessor::CompressorProcessor ()
: mThreshold(1)
, mBypass (false)
{
	setControllerClass (CompressorControllerUID);
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

	//if (state)
	//{
	//	mBuffer = (float**)std::malloc (numChannels * sizeof (float*));
	//	
	//	size_t size = (size_t)(processSetup.sampleRate * sizeof (float) + 0.5);
	//	for (int32 channel = 0; channel < numChannels; channel++)
	//	{
	//		mBuffer[channel] = (float*)std::malloc (size);	// 1 second delay max
	//		memset (mBuffer[channel], 0, size);
	//	}
	//	mBufferPos = 0;
	//}
	//else
	//{
	//	if (mBuffer)
	//	{
	//		for (int32 channel = 0; channel < numChannels; channel++)
	//		{
	//			std::free (mBuffer[channel]);
	//		}
	//		std::free (mBuffer);
	//		mBuffer = 0;
	//	}
	//}
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
					case kThresholdId:
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) == kResultTrue)
							mThreshold = value;
						break;
					case kBypassId:
						if (paramQueue->getPoint (numPoints - 1,  sampleOffset, value) == kResultTrue)
						{
							mBypass = (value > 0.5f);
						}
						break;
				}
			}
		}
	}

	if (mBypass) {
		return kResultFalse;
	}

	if (data.numSamples > 0)
	{
		SpeakerArrangement arr;
		getBusArrangement (kOutput, 0, arr);
		int32 numChannels = SpeakerArr::getChannelCount (arr);

		// TODO do something in Bypass : copy inpuit to output if necessary...

		// apply delay
		//int32 delayInSamples = std::max<int32> (1, (int32)(mDelay * processSetup.sampleRate)); // we have a minimum of 1 sample delay here
		//for (int32 channel = 0; channel < numChannels; channel++)
		//{
		//	float* inputChannel = data.inputs[0].channelBuffers32[channel];
		//	float* outputChannel = data.outputs[0].channelBuffers32[channel];

		//	int32 tempBufferPos = mBufferPos;
		//	for (int32 sample = 0; sample < data.numSamples; sample++)
		//	{
		//		float tempSample = inputChannel[sample];
		//		outputChannel[sample] = mBuffer[channel][tempBufferPos];
		//		mBuffer[channel][tempBufferPos] = tempSample;
		//		tempBufferPos++;
		//		if (tempBufferPos >= delayInSamples)
		//			tempBufferPos = 0;
		//	}
		//}
		//mBufferPos += data.numSamples;
		//while (delayInSamples && mBufferPos >= delayInSamples)
		//	mBufferPos -= delayInSamples;

		for (int32 channel = 0; channel < numChannels; channel++)
		{
			float* inputChannel = data.inputs[0].channelBuffers32[channel];
			float* outputChannel = data.outputs[0].channelBuffers32[channel];
			for (int32 sample = 0; sample < data.numSamples; sample++)
			{
				outputChannel[sample] = inputChannel[sample] * mThreshold;
			}
		}
	}	
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::setState (IBStream* state)
{
	LOG("CompressorProcessor::setState\n");
	if (!state)
		return kResultFalse;

	// called when we load a preset, the model has to be reloaded

	float savedThreshold = 0.f;
	if (state->read (&savedThreshold, sizeof (ParamValue)) != kResultOk)
	{
		return kResultFalse;
	}

	int32 savedBypass = 0;
	if (state->read (&savedBypass, sizeof (int32)) != kResultOk)
	{
		// could be an old version, continue 
	}

#if BYTEORDER == kBigEndian
	SWAP_32 (savedDelay)
	SWAP_32 (savedBypass)
#endif

	mThreshold = savedThreshold;
	mBypass = savedBypass > 0;

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorProcessor::getState (IBStream* state)
{
	LOG("CompressorProcessor::getState\n");
	// here we need to save the model

	float toSaveThreshold = mThreshold;
	int32 toSaveBypass = mBypass ? 1 : 0;

#if BYTEORDER == kBigEndian
	SWAP_32 (toSaveThreshold)
	SWAP_32 (toSaveBypass)
#endif

	state->write (&toSaveThreshold, sizeof (ParamValue));
	state->write (&toSaveBypass, sizeof (int32));

	return kResultTrue;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
