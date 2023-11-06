#pragma once
#include <stdexcept>
#include <math.h>
#include "../core/logging.h"

class filter
{
	public:
	enum boundary_condtion
	{
		EClamp = 0,
		ERepeat,
		EMirror,
		EZero,
		EOne
	};

	protected:
	alignas(32) float m_values[32];
	float m_radius;
	float m_scale_factor;
	int m_border_size;

	public:
	inline filter(float radius, float scale_factor, int border_size) :
		m_radius(radius), m_scale_factor(scale_factor), m_border_size(border_size) {};
	
	inline filter(FILE* f)
	{
		fread(&m_radius,sizeof(m_radius),1,f);
		fread(&m_scale_factor,sizeof(m_scale_factor),1,f);
		fread(&m_border_size,sizeof(m_border_size),1,f);
	};

	virtual float eval(float x) const = 0;

	inline float eval_discretized(float x) const
	{
		return m_values[std::min((int)std::abs(x*m_scale_factor),31)];
	};

	inline float get_radius() const {return m_radius;};

	inline int get_border_size() const {return m_border_size;};

	protected:
	inline void config()
	{
		if(m_radius < 0.0f)
			log(LOG_WARN,"Filter radius was invalid! Setting to 0.0\n");
		
		float sum = 0.0f;
		for(int i = 0; i < 31; ++i)
		{
			float val = eval((m_radius*i)/31);
			m_values[i] = val;
			sum += val;
		}

		m_values[31] = 0.0f;
		m_scale_factor = 31 / m_radius;
		m_border_size = (int)std::ceil(m_radius - 0.5f);
		
		sum *= 2.0f * m_radius / 31;
		float normalization = 1.0f / sum;

		for(int i = 0; i < 31; ++i)
			m_values[i] *= normalization;
	};
};