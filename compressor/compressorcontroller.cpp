//-----------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/adelay/source/adelaycontroller.cpp
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

#include "compressorcontroller.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "compressorids.h"
#include "ViewController.h"
#include "utils.h"

#if TARGET_OS_IPHONE
#include "interappaudio/iosEditor.h"
#endif

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
tresult PLUGIN_API CompressorController::initialize (FUnknown* context)
{
	LOG("CompressorController::initialize\n");
	tresult result = EditController::initialize (context);
	if (result == kResultTrue)
	{
		parameters.addParameter(STR16("Bypass"), 0, 1, 0, ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass, ParamTag::kBypassId);
		parameters.addParameter(STR16("Threshold"), STR16("db"), 0, 1, ParameterInfo::kCanAutomate, ParamTag::kThresholdId);
		parameters.addParameter(STR16("Ratio"), STR16(""), 0, 1, ParameterInfo::kCanAutomate, ParamTag::kRatioId);
		parameters.addParameter(STR16("Gain"), STR16("db"), 0, 1, ParameterInfo::kCanAutomate, ParamTag::kGainId);
		parameters.addParameter(STR16("Attack"), STR16("ms"), 0, 1, ParameterInfo::kCanAutomate, ParamTag::kAttackId);
		parameters.addParameter(STR16("Release"), STR16("ms"), 0, 1, ParameterInfo::kCanAutomate, ParamTag::kReleaseId);
	}
	return kResultTrue;
}


//-----------------------------------------------------------------------------
IPlugView* PLUGIN_API CompressorController::createView (FIDString name)
{
	LOG("CompressorController::createView\n");
	if (strcmp (name, ViewType::kEditor) == 0)
	{
		return new VSTGUI::VST3Editor(this, "view", "compressor.uidesc");
	}
	return 0;
}


//------------------------------------------------------------------------
tresult PLUGIN_API CompressorController::setComponentState (IBStream* state)
{
	LOG("CompressorController::setComponentState\n");
	// we receive the current state of the component (processor part)
	// we read only the gain and bypass value...
	if (state)
	{
		ParamValue savedThreshold = 0.f;
		if (state->read (&savedThreshold, sizeof (savedThreshold)) == kResultFalse)
		{
			return kResultFalse;
		}

#if BYTEORDER == kBigEndian
		SWAP_32 (savedDelay)
#endif
		LOG("threshold %f\n", savedThreshold);
		setParamNormalized (ParamTag::kThresholdId, savedThreshold);

		// read the bypass
		int32 bypassState;
		if (state->read (&bypassState, sizeof (bypassState)) == kResultTrue)
		{
#if BYTEORDER == kBigEndian
			SWAP_32 (bypassState)
#endif
			LOG("bypass %d\n", bypassState);
			setParamNormalized (kBypassId, bypassState ? 1 : 0);
		}
	}

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorController::setState(IBStream* state) {
	LOG("CompressorController::setState\n");
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CompressorController::getState(IBStream* state) {
	LOG("CompressorController::getState\n");
	return kResultFalse;
}

CView * CompressorController::verifyView(CView * view, const UIAttributes & attributes, const IUIDescription * description, VST3Editor * editor)
{
	LOG("CompressorController::verifyView\n");
	CControl* control = dynamic_cast<CControl*>(view);
	if (control)
	{
		setShowUnits(control);
	}
	return view;
}

void CompressorController::setShowUnits(CControl* control)
{
	CTextEdit* textEdit = dynamic_cast<CTextEdit*>(control);
	if (textEdit)
	{
		TChar* units = getParameterObject(control->getTag())->getInfo().units;
		textEdit->setValueToStringFunction([units](float value, char* utf8String, CParamDisplay* display)
		{
			sprintf(utf8String, "%.4f %ws\0", value, units);
			return true;
		});
		textEdit->setStringToValueFunction([&](UTF8StringPtr txt, float &result, CTextEdit* textEdit)
		{
			sscanf(txt, "%f", &result);
			return true;
		});
	}
}

IController * CompressorController::createSubController(UTF8StringPtr name, const IUIDescription * description, VST3Editor * editor)
{
	LOG("CompressorController::createSubController\n");
	if (UTF8StringView(name) == "ViewController")
	{
		return new ViewController(this);
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
