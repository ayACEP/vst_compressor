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
#include "ParamTag.h"
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

		thresholdRange.setRange(-60, 0);
		ratioRange.setRange(0.1f, 16.0f);
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
		int32 savedBypass;
		ParamValue savedThreshold = 0.f;
		ParamValue savedRatio = 0.f;
		ParamValue savedGain = 0.f;
		ParamValue savedAttack = 0.f;
		ParamValue savedRelease = 0.f;

		if (state->read(&savedBypass, sizeof(savedBypass)) == kResultFalse)
		{
			return kResultFalse;
		}
		if (state->read (&savedThreshold, sizeof (savedThreshold)) == kResultFalse)
		{
			return kResultFalse;
		}
		if (state->read(&savedRatio, sizeof(savedRatio)) == kResultFalse)
		{
			return kResultFalse;
		}
		if (state->read(&savedGain, sizeof(savedGain)) == kResultFalse)
		{
			return kResultFalse;
		}
		if (state->read(&savedAttack, sizeof(savedAttack)) == kResultFalse)
		{
			return kResultFalse;
		}
		if (state->read(&savedRelease, sizeof(savedRelease)) == kResultFalse)
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

		LOG("bypass %d\n", savedBypass);
		LOG("threshold %f\n", savedThreshold);
		LOG("ratio %f\n", savedRatio);
		LOG("gain %f\n", savedGain);
		LOG("attack %f\n", savedAttack);
		LOG("release %f\n", savedRelease);

		setParamNormalized(ParamTag::kBypassId, savedBypass ? 1 : 0);
		setParamNormalized(ParamTag::kThresholdId, savedThreshold);
		setParamNormalized(ParamTag::kRatioId, savedRatio);
		setParamNormalized(ParamTag::kGainId, savedGain);
		setParamNormalized(ParamTag::kAttackId, savedAttack);
		setParamNormalized(ParamTag::kReleaseId, savedRelease);
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

tresult PLUGIN_API CompressorController::setParamNormalized(ParamID tag, ParamValue value)
{
	//LOG("CompressorController::setParamNormalized %d, %f\n", tag, value);
	return EditController::setParamNormalized(tag, value);
}

tresult PLUGIN_API CompressorController::getParamStringByValue(ParamID tag, ParamValue valueNormalized, String128 string)
{
	tresult result = kResultTrue;
	TChar* units = getParameterObject(tag)->getInfo().units;
	switch (tag)
	{
	case ParamTag::kThresholdId: {
		ParamValue value = thresholdRange.toUsefulValue(valueNormalized);
		swprintf(string, L"%.1f", value);
	} break;
	case ParamTag::kRatioId: {
		ParamValue value = ratioRange.toUsefulValue(valueNormalized);
		swprintf(string, L"%.1f:1", value);
	} break;
	default:
		swprintf(string, L"%.2f %ws\0", valueNormalized, units);
	}
	//LOG("CompressorController::getParamStringByValue %d, from %f to %ws\n", tag, valueNormalized, string);
	return result;
}

tresult PLUGIN_API CompressorController::getParamValueByString(ParamID tag, TChar * string, ParamValue & valueNormalized)
{
	tresult result = kResultTrue;
	switch (tag)
	{
	case ParamTag::kRatioId:
	default:
		swscanf(string, L"%lf", &valueNormalized);
	}
	//LOG("CompressorController::getParamValueByString %d, from %ws to %f\n", tag, string, valueNormalized);
	return result;
}

CView * CompressorController::verifyView(CView * view, const UIAttributes & attributes, const IUIDescription * description, VST3Editor * editor)
{
	//LOG("CompressorController::verifyView\n");
	CControl* control = dynamic_cast<CControl*>(view);
	if (control == nullptr)
	{
		return view;
	}
	switch (control->getTag()) {
	case ParamTag::kRatioId:
		configRatioShow(control);
		break;
	default:
		configDefulatShow(control);
	}
	return view;
}

void CompressorController::configRatioShow(CControl* control)
{
	CTextEdit* textEdit = dynamic_cast<CTextEdit*>(control);
	if (textEdit)
	{
		/*textEdit->setValueToStringFunction([](float value, char* utf8String, CParamDisplay* display)
		{
			value = value * 16;
			value = value < 0.1 ? 0.1 : value;
			sprintf(utf8String, "%.1f:1", value);
			LOG("to string: %s\n", utf8String);
			return true;
		});
		textEdit->setStringToValueFunction([](UTF8StringPtr txt, float &result, CTextEdit* textEdit)
		{
			sscanf(txt, "%f", &result);
			LOG("to value: %f\n", result);
			return true;
		});*/
	}
}

void CompressorController::configDefulatShow(CControl* control)
{
	CTextEdit* textEdit = dynamic_cast<CTextEdit*>(control);
	if (textEdit)
	{
		/*TChar* units = getParameterObject(control->getTag())->getInfo().units;
		textEdit->setValueToStringFunction([units](float value, char* utf8String, CParamDisplay* display)
		{
			sprintf(utf8String, "%.2f %ws\0", value, units);
			LOG("to string: %s\n", utf8String);
			return true;
		});
		textEdit->setStringToValueFunction([&](UTF8StringPtr txt, float &result, CTextEdit* textEdit)
		{
			sscanf(txt, "%f", &result);
			LOG("to value: %f\n", result);
			return true;
		});*/
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
