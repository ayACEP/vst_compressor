#pragma once

class RangeMapping
{
public:
	RangeMapping();
	RangeMapping(double min, double max);
	~RangeMapping() {};

	void setRange(double min, double max);
	double toNormalizeValue(double usefulValue);
	double toUsefulValue(double normalizeValue);

private:
	double min1;
	double max1;
	double min2;
	double max2;
};
