#pragma once

#include "vstgui/plugin-bindings/vst3editor.h"
#include "compressorcontroller.h"

namespace Steinberg {
namespace Vst {

class CompressorController;

class ViewController: public IController
{
private:
	CTextEdit* txt;
	CTextButton* btn;
	CKnob* knob;
	CCheckBox* chk;

	CompressorController* controller;
public:
	ViewController(CompressorController* controller);
	~ViewController();

	void valueChanged(CControl* pControl) override;
	CView* verifyView(CView* view, const UIAttributes& attributes, const IUIDescription* description) override;
	void controlBeginEdit(CControl* pControl) override;
	void controlEndEdit(CControl* pControl) override;
};

enum ControlTag {
	root = 0,
	btn = 1,
	chk = 2,
	knob = 3,
	txt = 4,
};

}
}