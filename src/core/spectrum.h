#pragma once
#include "math/include.h"
#include <stdint.h>
#include <math.h>
#include <vector>
#include <functional>
#include <stdexcept>

#define Epsilon 1e-4f
#define ShadowEpsilon 1e-3f

#ifndef SPECTRUM_SAMPLES
#	define SPECTRUM_SAMPLES 32
#endif

#ifndef SPECTRUM_START
#	define SPECTRUM_START 360
#	define SPECTRUM_MIN_WAVELENGTH 360
#endif

#ifndef SPECTRUM_END
#	define SPECTRUM_END 830
#	define SPECTRUM_MAX_WAVELENGTH 830
#endif

#ifndef SPECTRUM_RANGE
#	define SPECTRUM_RANGE (SPECTRUM_END - SPECTRUM_START)
#endif

static const float INV_SPECTRUM_SAMPLES = 1.0f / SPECTRUM_SAMPLES;

//align so that vectorization will occur ~20% faster
struct alignas(32) spectrum
{
	union
	{
		float8 x[SPECTRUM_SAMPLES/8];
		float p[SPECTRUM_SAMPLES];// = {0.0f};
	};

	inline spectrum()
	{
		//for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		//	p[i] = 0.0f;
	};
	
	inline spectrum(float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] = a;
	};

	~spectrum() {};

	inline spectrum operator+(const spectrum &a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] + a.p[i];
		return r;
	};

	inline spectrum operator-(const spectrum &a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] - a.p[i];
		return r;
	};

	inline spectrum operator*(const spectrum &a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] * a.p[i];
		return r;
	};

	inline spectrum operator/(const spectrum &a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] / a.p[i];
		return r;
	};

	inline spectrum operator*(float a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] * a;
		return r;
	};

	inline spectrum operator/(float a) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = p[i] / a;
		return r;
	};

	inline spectrum operator-(void) const
	{
		spectrum r;
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = 0.0f - p[i];
		return r;
	};

	inline spectrum& operator+=(const spectrum &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] += a.p[i];
		return *this;
	};

	inline spectrum& operator-=(const spectrum &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] -= a.p[i];
		return *this;
	};

	inline spectrum& operator*=(const spectrum &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] *= a.p[i];
		return *this;
	};

	inline spectrum& operator/=(const spectrum &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] /= a.p[i];
		return *this;
	};

	inline spectrum& operator*=(float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] *= a;
		return *this;
	};

	inline spectrum& operator/=(float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] /= a;
		return *this;
	};

	inline spectrum& operator=(float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] = a;
		return *this;
	};
};

inline spectrum operator*(float a, const spectrum &b)
{
	spectrum r;
	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		r.p[i] = b.p[i] * a;
	return r;
};

inline spectrum operator/(float a, const spectrum &b)
{
	spectrum r;
	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		r.p[i] = b.p[i] / a;
	return r;
};

inline spectrum sqrt(const spectrum &s)
{
	spectrum r;
	
	for(uint32_t i = 0; i < SPECTRUM_SAMPLES; i++)
		r.p[i] = std::sqrt(s.p[i]);

	return r;
};

inline bool is_black(const spectrum &s)
{
	for(uint32_t i = 0; i < SPECTRUM_SAMPLES; i++)
		if(s.p[i] > 0.0f)
			return false;
	
	return true;
};

float luminance(const spectrum &s);

inline bool isnan(const spectrum &s)
{
#ifdef DEBUG_SPECTRUM
	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		if(std::isnan(s.p[i]))
			return true;
#endif
	return false;
};

inline void throw_nan(const spectrum &s, const char* msg)
{
	if(isnan(s))
	{
		printf("%s",msg);
		throw std::runtime_error("NaN spectrum value!\n");
	}
		
};

inline void throw_nan(const spectrum &s)
{
	if(isnan(s))
		throw std::runtime_error("NaN spectrum value!\n");
};

class continous_spectrum
{
	public:
	float average(float lambdaMin, float lambdaMax) const;

	virtual float eval(float lambda) const;
};

class interpolated_spectrum : public continous_spectrum
{
	public:
	std::vector<float> m_wavelengths, m_values;
	float eval(float lambda) const;
	interpolated_spectrum(const float *wavelengths, const float *values, size_t nEntries);
};

class color : public continous_spectrum
{
	public:
	static float m_wavelengths[SPECTRUM_SAMPLES+1];

	static color CIE_X;
	static color CIE_Y;
	static color CIE_Z;
	static float CIE_normalization;

	static color rgbRefl2SpecWhite;
	static color rgbRefl2SpecCyan;
	static color rgbRefl2SpecMagenta;
	static color rgbRefl2SpecYellow;
	static color rgbRefl2SpecRed;
	static color rgbRefl2SpecGreen;
	static color rgbRefl2SpecBlue;
	static color rgbIllum2SpecWhite;
	static color rgbIllum2SpecCyan;
	static color rgbIllum2SpecMagenta;
	static color rgbIllum2SpecYellow;
	static color rgbIllum2SpecRed;
	static color rgbIllum2SpecGreen;
	static color rgbIllum2SpecBlue;

	static color CIE_D65;

	float s[SPECTRUM_SAMPLES];

	void init();

	void fromContinuousSpectrum(const continous_spectrum &smooth);

	float eval(float lambda) const;

	inline spectrum operator*(const float a) const
	{
		spectrum r;

		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.p[i] = this->s[i] * a;
		
		return r;
	};

	inline void operator/=(const float &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			this->s[i] /= a;
	};

	inline float& operator[](int entry) {return s[entry];};

	inline float operator[](int entry) const {return s[entry];};

	inline float getLuminance() const;
};

inline float filter_spectrum(const spectrum &s)
{
	float res = 0.0f;
	for(uint32_t i = 0; i < SPECTRUM_SAMPLES; i++)
		res += std::max(s.p[i],0.0f);
	
	return res * INV_SPECTRUM_SAMPLES;
};

inline spectrum clamp(const spectrum &s)
{
	spectrum ret;

	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
#ifdef DEBUG_SPECTRUM
		if(std::isnan(s.p[i]) || std::isinf(s.p[i]))
			ret.p[i] = 0.0f;
		else
#endif
			ret.p[i] = std::max(0.0f,s.p[i]);

	return ret;
};

inline spectrum operator*(const float a, const color &c)
{
	spectrum r;

	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		r.p[i] = c.s[i] * a;
	
	return r;
};

spectrum from_linear_rgb(const vec3f &rgb, bool reflectance);

inline float from_srgb_component(const float value)
{
	if (value <= (float) 0.04045)
		return value * (float) (1.0 / 12.92);
	return std::pow((value + (float) 0.055)
		* (float) (1.0 / 1.055), (float) 2.4);
};

inline spectrum from_srgb(const vec3f rgb, bool reflectance)
{
	vec3f rgb_ = rgb;
	rgb_.r = from_srgb_component(rgb.r);
	rgb_.g = from_srgb_component(rgb.g);
	rgb_.b = from_srgb_component(rgb.b);

	return from_linear_rgb(rgb_,reflectance);
};

inline spectrum rgb_to_spectrum(const vec3f rgb, bool is_light)
{
	return from_srgb(rgb,!is_light);
};

vec3f spectrum_to_linear_rgb(const spectrum &s);

vec3f spectrum_to_xyz(const spectrum &s);

vec3f spectrum_to_rgb(const spectrum &s);

//array of structs version. might be better
struct spectrum8a
{
	spectrum p[8];

	inline spectrum8a() {};

	inline spectrum8a(const float a)
	{
		for(int i = 0; i < 8; i++)
			p[i] = spectrum(a);
	};

	inline spectrum8a(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] = a[i];
	};

	inline spectrum8a(const spectrum &s)
	{
		for(int i = 0; i < 8; i++)
			p[i] = s;
	};

	inline spectrum8a operator+(const spectrum8a& a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] + a.p[i];
		return r;
	};

	inline spectrum8a operator-(const spectrum8a& a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] - a.p[i];
		return r;
	};

	inline spectrum8a operator*(const spectrum8a& a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] * a.p[i];
		return r;
	};

	inline spectrum8a operator/(const spectrum8a& a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] / a.p[i];
		return r;
	};

	inline spectrum8a operator+(const float8 a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] + a[i];
		return r;
	};

	inline spectrum8a operator-(const float8 a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] - a[i];
		return r;
	};

	inline spectrum8a operator*(const float8 a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] * a[i];
		return r;
	};

	inline spectrum8a operator/(const float8 a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] / a[i];
		return r;
	};

	inline spectrum8a operator*(const float a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] * a;
		return r;
	};

	inline spectrum8a operator/(const float a) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = p[i] / a;
		return r;
	};

	inline spectrum8a operator-(void) const
	{
		spectrum8a r;
		for(int i = 0; i < 8; i++)
			r.p[i] = spectrum(0.0f) - p[i];
		return r;
	};

	inline void operator+=(const spectrum8a &a)
	{
		for(int i = 0; i < 8; i++)
			p[i] += a.p[i];
	};

	inline void operator-=(const spectrum8a &a)
	{
		for(int i = 0; i < 8; i++)
			p[i] -= a.p[i];
	};

	inline void operator*=(const spectrum8a &a)
	{
		for(int i = 0; i < 8; i++)
			p[i] *= a.p[i];
	};

	inline void operator/=(const spectrum8a &a)
	{
		for(int i = 0; i < 8; i++)
			p[i] /= a.p[i];
	};

	inline void operator+=(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] += a[i];
	};

	inline void operator-=(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] -= a[i];
	};

	inline void operator*=(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] *= a[i];
	};

	inline void operator/=(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] /= a[i];
	};

	inline void operator=(const float8 a)
	{
		for(int i = 0; i < 8; i++)
			p[i] = a[i];
	};

	inline void operator*=(const float a)
	{
		for(int i = 0; i < 8; i++)
			p[i] *= a;
	};

	inline void operator/=(const float a)
	{
		for(int i = 0; i < 8; i++)
			p[i] /= a;
	};

	inline void operator=(const float a)
	{
		for(int i = 0; i < 8; i++)
			p[i] = a;
	};
};
struct spectrum8
{
	union
	{
		__m256 x[32];
		float p[256];
		float8 f[32];
	};

	inline spectrum8() {};

	inline spectrum8(const float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] = a;
	};

	inline spectrum8(const float8 a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			f[i] = a;
	};

	inline spectrum8(const spectrum &s)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			for(int j = 0; j < 8; j++)
				f[i].set(j,s.p[i]);
	};

	inline spectrum8 operator+(const spectrum8 &a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			r.f[i] = f[i] + a.f[i];
		
		return r;
	};

	inline spectrum8 operator-(const spectrum8 &a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = p[i] - a.p[i];
		
		return r;
	};

	inline spectrum8 operator*(const spectrum8 &a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = p[i] * a.p[i];
		
		return r;
	};

	inline spectrum8 operator/(const spectrum8 &a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = p[i] / a.p[i];
		
		return r;
	};

	inline spectrum8 operator*(const float a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = p[i] * a;
		
		return r;
	};

	inline spectrum8 operator/(const float a) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = p[i] / a;
		
		return r;
	};

	inline spectrum8 operator-(void) const
	{
		spectrum8 r;

		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			r.p[i] = 0.0f - p[i];
		
		return r;
	};

	inline void operator+=(const spectrum8 &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] += a.p[i];
	};

	inline void operator-=(const spectrum8 &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] -= a.p[i];
	};

	inline void operator*=(const spectrum8 &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] *= a.p[i];
	};

	inline void operator/=(const spectrum8 &a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] /= a.p[i];
	};

	inline void operator*=(const float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] *= a;
	};

	inline void operator/=(const float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] /= a;
	};

	inline void operator=(const float a)
	{
		for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
			p[i] = a;
	};

	inline spectrum get(int i) const
	{
		spectrum ret;
		for(int j = 0; j < SPECTRUM_SAMPLES; j++)
			ret.p[j] = f[j][i];
		return ret;
	};

	inline void set(int i, const spectrum &s)
	{
		for(int j = 0; j < SPECTRUM_SAMPLES; j++)
			f[j].set(i,s.p[j]);
	};
};

inline spectrum8 operator*(const float a, const spectrum8 &b)
{
	spectrum8 r;

	for(int i = 0; i < SPECTRUM_SAMPLES * 8; i++)
		r.p[i] = a * b.p[i];

	return r;
};

inline float8 is_black(const spectrum8 &s)
{
	float8 ret = float8(0.0f);
	for(int i = 0; i < SPECTRUM_SAMPLES; i++)
		ret = ret || (s.f[i] > float8(0.0f));
	return ~ret;
};

inline bool all_black(const spectrum8a &s)
{
	for(int i = 0; i < 8; i++)
		if(!is_black(s.p[i]))
			return false;

	return true;
};

struct alignas(32) spectrum_half
{
	half p[SPECTRUM_SAMPLES];

	spectrum_half(spectrum s)
	{
		for(int32_t i = 0; i < SPECTRUM_SAMPLES; i++)
			p[i] = s.p[i];
	};
};