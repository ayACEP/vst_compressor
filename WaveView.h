#pragma once
#include <vstgui\lib\controls\ccontrol.h>

namespace VSTGUI {

class CWaveView : public CControl
{
public:
	CWaveView(const CRect& size, IControlListener* listener = 0, int32_t tag = 0, CBitmap* pBackground = 0);
	~CWaveView();
};

}