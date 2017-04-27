#pragma once

#include "RangeMapping.h"

class ParamUtils {
public:
	static RangeMapping &get_ratio_range() { return ratioRange; };
	static RangeMapping &get_gain_range() { return gainRange; };
	static RangeMapping &get_attack_range() { return attackRange; };
	static RangeMapping &get_release_range() { return releaseRange; };
private:
	static RangeMapping ratioRange;
	static RangeMapping gainRange;
	static RangeMapping attackRange;
	static RangeMapping releaseRange;
};
