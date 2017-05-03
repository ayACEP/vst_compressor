#include "compressorcontroller.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "ParamTag.h"
#include "ViewController.h"
#include "utils.h"
#include "ParamUtils.h"

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
	setKnobMode(KnobModes::kLinearMode);
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
	case ParamTag::kThresholdId: 
	{
		ParamValue value = normalizedValue2dBFS(valueNormalized);
		swprintf(string, L"%.2f %ws", value, units);
	} break;
	case ParamTag::kRatioId: 
	{
		ParamValue value = ParamUtils::get_ratio_range().toUsefulValue(valueNormalized);
		swprintf(string, L"%.1f:1", value);
	} break;
	case ParamTag::kGainId: 
	{
		ParamValue value = ParamUtils::get_gain_range().toUsefulValue(valueNormalized);
		swprintf(string, L"%.2f %ws", value, units);
	} break;
	case ParamTag::kAttackId: 
	{
		ParamValue value = ParamUtils::get_attack_range().toUsefulValue(valueNormalized);
		swprintf(string, L"%.1f %ws", value, units);
	} break;
	case ParamTag::kReleaseId: 
	{
		ParamValue value = ParamUtils::get_release_range().toUsefulValue(valueNormalized);
		swprintf(string, L"%.0f %ws", value, units);
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
	ParamValue tempValue = 0.0;

	switch (tag)
	{
	case ParamTag::kThresholdId:
	{
		swscanf(string, L"%lf", &tempValue);
		valueNormalized = dBFS2NormalizedValue(tempValue);
	} break;
	case ParamTag::kRatioId:
	{
		swscanf(string, L"%lf", &tempValue);
		valueNormalized = ParamUtils::get_ratio_range().toNormalizeValue(tempValue);
	} break;
	case ParamTag::kGainId:
	{
		swscanf(string, L"%lf", &tempValue);
		valueNormalized = ParamUtils::get_gain_range().toNormalizeValue(tempValue);
	} break;
	case ParamTag::kAttackId:
	{
		swscanf(string, L"%lf", &tempValue);
		valueNormalized = ParamUtils::get_attack_range().toNormalizeValue(tempValue);
	} break;
	case ParamTag::kReleaseId:
	{
		swscanf(string, L"%lf", &tempValue);
		valueNormalized = ParamUtils::get_release_range().toNormalizeValue(tempValue);
	} break;
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
