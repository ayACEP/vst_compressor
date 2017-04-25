#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "RangeMapping.h"

#if MAC
#include <TargetConditionals.h>
#endif

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
class CompressorController : public EditController, public VST3EditorDelegate
{
public:
	//------------------------------------------------------------------------
	// create function required for Plug-in factory,
	// it will be called to create new instances of this controller
	//------------------------------------------------------------------------
	static FUnknown* createInstance (void*)
	{
		return (IEditController*)new CompressorController();
	}

	//---from IPluginBase--------
	tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;

	//---from EditController-----
	IPlugView* PLUGIN_API createView (FIDString name) SMTG_OVERRIDE;
	tresult PLUGIN_API setComponentState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API setParamNormalized(ParamID tag, ParamValue value) override;
	tresult PLUGIN_API getParamStringByValue(ParamID tag, ParamValue valueNormalized, String128 string) override;
	tresult PLUGIN_API getParamValueByString(ParamID tag, TChar* string, ParamValue& valueNormalized) override;

	//-----from VST3EditorDelegate-----//
	CView* verifyView(CView* view, const UIAttributes& attributes, const IUIDescription* description, VST3Editor* editor) override;
	IController* createSubController(UTF8StringPtr name, const IUIDescription* description, VST3Editor* editor) override;

private:
	void configDefulatShow(CControl* control);
	void configRatioShow(CControl* control);

private:
	RangeMapping thresholdRange;
	RangeMapping ratioRange;
};

//------------------------------------------------------------------------
} // namespace Vst
} // namespace Steinberg
