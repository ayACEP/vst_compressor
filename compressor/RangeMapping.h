#pragma once

class RangeMapping
{
public:
	RangeMapping();
	RangeMapping(float min, float max);
	~RangeMapping() {};

	void setRange(float min, float max);
	float toNormalizeValue(float usefulValue);
	float toUsefulValue(float normalizeValue);

private:
	float min1;
	float max1;
	float min2;
	float max2;
};

