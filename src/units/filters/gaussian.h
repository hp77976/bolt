#pragma once
#include "../../base/filter.h"

class gaussian_filter : public filter
{
	public:
	gaussian_filter(float radius, float scale_factor, int border_size) : 
		filter(radius, scale_factor, border_size)
	{
		config();
	};

	gaussian_filter(FILE* f) : filter(f)
	{
		config();
	};

	float eval(float x) const
	{
		float alpha = -1.0f / (2.0f * (0.5f * 0.5f));
		return std::max(exp((double)alpha * x * x),exp((double)alpha));
	};
};