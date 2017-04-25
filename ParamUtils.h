#pragma once

#include "RangeMapping.h"

class ParamUtils {
public:
	static RangeMapping &get_ratio_range() { return ratioRange; };
	static RangeMapping &get_gain_range() { return gainRange; };
private:
	static RangeMapping ratioRange;
	static RangeMapping gainRange;
};
