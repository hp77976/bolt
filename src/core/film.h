#pragma once
#include <mutex>
#include "math/include.h"
#include "spectrum.h"

class filter;

class film
{
	public:
	filter* m_filter = nullptr;
	spectrum* c = nullptr; //color
	float* w = nullptr; //weight
	vec2i m_size;
	vec2i m_crop_size;
	vec2i m_crop_offset;
	std::mutex lock;

	film(
		const vec2i &size, const vec2i &crop_size,
		const vec2i &offset, filter* filter_
	) : m_size(size), m_crop_size(crop_size), m_crop_offset(offset)
	{
		c = new spectrum[size.x*size.y];
		w = new float[size.x*size.y];
		m_filter = filter_;
	};

	film(film* ref_film) : film(
		ref_film->m_size,ref_film->m_crop_size,
		ref_film->m_crop_offset,ref_film->m_filter
	) {};

	vec2i size() const {return m_size;};

	void splat(const spectrum &s, const vec2f &pos);

	void normalize()
	{
		for(int i = 0; i < (m_size.x * m_size.y); i++)
			for(int j = 0; j < SPECTRUM_SAMPLES; j++)
				if(c[i].p[j] > 0.0f && w[i] > 0)
					c[i].p[j] *= (1.0f / w[i]);
	};

	void normalize(int samples_)
	{
		for(int i = 0; i < (m_size.x * m_size.y); i++)
			for(int j = 0; j < SPECTRUM_SAMPLES; j++)
				if(c[i].p[j] > 0.0f)
					c[i].p[j] /= samples_;
	};

	void clear()
	{
		for(int i = 0; i < (m_size.x * m_size.y); i++)
		{
			c[i] = 0.0f;
			w[i] = 0.0f;
		}
	};

	inline void copy_from(film* const f2)
	{
		if(f2->m_size.x != m_size.x || f2->m_size.y != m_size.y)
			throw std::runtime_error("You can't copy mismatched films!\n");

		int32_t pxc = m_size.x * m_size.y;
		memcpy(c,f2->c,pxc*sizeof(spectrum));
		memcpy(w,f2->w,pxc*sizeof(float));
	};

	//TODO: this should probably be removed...
	inline void to_rgba(vec4f* dest) const
	{
		if(dest == nullptr)
			return;
		
		for(int i = 0; i < (m_size.x * m_size.y); i++)
		{
			//vec3f rgb = spectrum_to_rgb(c[i]);
			vec3f rgb = spectrum_to_linear_rgb(c[i]);
			dest[i] = {rgb.r,rgb.g,rgb.b,1.0f};
		}
	};

	void write(const char* path, bool write_info);
};

void write_film_stupid(std::string path, std::vector<float> data, uint32_t w, uint32_t h);