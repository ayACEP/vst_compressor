#include "RangeMapping.h"

RangeMapping::RangeMapping()
{
	min1 = 0.0;
	max1 = 1.0;
}

RangeMapping::RangeMapping(double min, double max): RangeMapping()
{
	setRange(min, max);
}

void RangeMapping::setRange(double min, double max)
{
	min2 = min;
	max2 = max;
}

double RangeMapping::toNormalizeValue(double usefulValue)
{
	return (usefulValue - min2) / (max2 - min2) * (max1 - min1) + min1;
}

double RangeMapping::toUsefulValue(double normalizeValue)
{
	return (normalizeValue - min1) / (max1 - min1) * (max2 - min2) + min2;
}
