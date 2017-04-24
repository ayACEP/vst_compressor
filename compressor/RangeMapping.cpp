#include "RangeMapping.h"

RangeMapping::RangeMapping()
{
	min1 = 0;
	max1 = 1;
}

RangeMapping::RangeMapping(float min, float max)
{
	RangeMapping();
	setRange(min, max);
}

void RangeMapping::setRange(float min, float max)
{
	min2 = min;
	max2 = max;
}

float RangeMapping::toNormalizeValue(float usefulValue)
{
	return (usefulValue - min2) / (max2 - min2) * (max1 - min1) + min1;
}

float RangeMapping::toUsefulValue(float normalizeValue)
{
	return (normalizeValue - min1) / (max1 - min1) * (max2 - min2) + min2;
}
