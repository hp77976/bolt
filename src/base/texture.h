#pragma once
#include "../core/math/include.h"
#include "../core/spectrum.h"
#include "../core/records.h"

class color_socket
{
	public:
	color_socket() {};

	virtual spectrum eval(const bsdf_record &i_rec, bool filter = true) const = 0;
};

class const_color : public color_socket
{
	protected:
	spectrum m_color;

	public:
	const_color(const spectrum &s, bool is_light = false) : m_color(s) {};

	const_color(FILE* f, bool is_light = false)
	{
		float color[4];
		vec3f rgb;
		fread(color,sizeof(float),4,f);
		rgb.r = color[0]; rgb.g = color[1]; rgb.b = color[2];
		m_color = from_linear_rgb(rgb,!is_light);
	};

	spectrum eval(const bsdf_record &i_rec, bool filter = true) const {return m_color;};
};

class texture : public color_socket
{

};

class texture_2d : public texture
{
	spectrum eval(const bsdf_record &i_rec, bool filter = true) const
	{
		return 0.0f;
	};
};