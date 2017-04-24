#include "ViewController.h"
#include "ParamTag.h"
#include "utils.h"

namespace Steinberg {
namespace Vst {

ViewController::ViewController(CompressorController *controller)
{
	this->controller = controller;
}

ViewController::~ViewController()
{
}

void ViewController::valueChanged(CControl * pControl)
{
	LOG("%s %d %f %f\n", "ViewController::valueChanged", pControl->getTag(), pControl->getValue(), pControl->getValueNormalized());
	switch (pControl->getTag())
	{
	case ControlTag::txt: {
		auto text = txt->getText();
	} break;
	case ControlTag::btn: {

	} break;
	case ControlTag::knob:
		//controller->setParamNormalized(kThresholdId, pControl->getValueNormalized());
		break;
	case ControlTag::chk:
		//controller->setParamNormalized(kBypassId, pControl->getValueNormalized());
		break;
	default:
		break;
	}
}

CView * ViewController::verifyView(CView * view, const UIAttributes & attributes, const IUIDescription * description)
{
	LOG("ViewController::verifyView\n");
	CControl* pControl = dynamic_cast<CControl*>(view);
	if (pControl == nullptr) {
		return view;
	}
	switch (pControl->getTag())
	{
	case ControlTag::txt:
		txt = dynamic_cast<CTextEdit*>(pControl);
		break;
	case ControlTag::btn:
		btn = dynamic_cast<CTextButton*>(pControl);
		break;
	case ControlTag::knob:
		knob = dynamic_cast<CKnob*>(pControl);
		break;
	case ControlTag::chk:
		chk = dynamic_cast<CCheckBox*>(pControl);
		break;
	default:
		break;
	}
	return view;
}

void ViewController::controlBeginEdit(CControl * pControl)
{
	LOG("ViewController::controlBeginEdit\n");
}

void ViewController::controlEndEdit(CControl * pControl)
{
	LOG("ViewController::controlEndEdit\n");
}

}
}