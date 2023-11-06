#pragma once
#include <xmmintrin.h>
#include <immintrin.h>
#include <x86intrin.h>

#include <string>

#include "avx_mathfun.h"
#include "../templates.h"

#ifdef DEBUG
#define DEBUG_SIMD
#endif

struct float4
{
	private:
	union
	{
		__m128 x;
		float __q[4];
	};

	public:
	inline float4() {};
	inline float4(const __m128& m) : x(m) {};
	inline float4(const float a) : x(_mm_set1_ps(a)) {};
	inline float4(const float f[4]){x = _mm_load_ps(f);};
	inline float4(const float a, const float b, const float c, const float d)
	{x = _mm_set_ps(a,b,c,d);};
	inline float4(bool b){x = b ? (__m128)_mm_set1_epi32(0x7FFFFFFF) : _mm_set1_ps(0.0f);};

	inline operator __m128() const {return this->x;};

	inline float4 operator+(const float4 &a) const {return _mm_add_ps(x,a.x);};
	inline float4 operator-(const float4 &a) const {return _mm_sub_ps(x,a.x);};
	inline float4 operator*(const float4 &a) const {return _mm_mul_ps(x,a.x);};
	inline float4 operator/(const float4 &a) const {return _mm_div_ps(x,a.x);};

	inline void operator+=(const float4 &a) {this->x = _mm_add_ps(x,a.x);};
	inline void operator-=(const float4 &a) {this->x = _mm_sub_ps(x,a.x);};
	inline void operator*=(const float4 &a) {this->x = _mm_mul_ps(x,a.x);};
	inline void operator/=(const float4 &a) {this->x = _mm_div_ps(x,a.x);};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline float4 operator >(const float4 &a) const {return _mm_cmp_ps(x,a.x,30);};
	inline float4 operator <(const float4 &a) const {return _mm_cmp_ps(x,a.x,17);};
	inline float4 operator>=(const float4 &a) const {return _mm_cmp_ps(x,a.x,29);};
	inline float4 operator<=(const float4 &a) const {return _mm_cmp_ps(x,a.x,18);};
	inline float4 operator==(const float4 &a) const {return _mm_cmp_ps(x,a.x,24);};
	inline float4 operator!=(const float4 &a) const {return _mm_cmp_ps(x,a.x,12);};

	inline float4 operator&&(const float4 &a) const {return _mm_and_ps(x,a.x);};
	inline float4 operator||(const float4 &a) const {return _mm_or_ps(x,a.x);};
	inline float4 operator &(const float4 &a) const {return _mm_and_ps(x,a.x);};
	inline float4 operator |(const float4 &a) const {return _mm_or_ps(x,a.x);};
	inline float4 operator ^(const float4 &a) const {return _mm_xor_ps(x,a.x);};
	
	//binary bit flip, useful for inverting comparison results
	inline float4 operator~() const {return (__m128)~(__m128i)x;}; //stupid...

	inline float4 operator-(void) const {return _mm_sub_ps(_mm_set1_ps(0.0f),x);};

	//this might be very very slow... so be careful!
	inline float operator[](const unsigned int i) const 
	{
		return __q[i];
	};
};

namespace math
{
	template <>
	inline float4 min(const float4 &a, const float4 &b) {return _mm_min_ps(a,b);};

	template <>
	inline float4 max(const float4 &a, const float4 &b) {return _mm_max_ps(a,b);};

	template <>
	inline float4 abs(const float4 &a) {return _mm_and_ps(a,_mm_set1_ps(-0.0f));};

	template <>
	inline float4 sqrt(const float4 &a) {return _mm_sqrt_ps(a);};

	template <>
	inline float4 rsqrt(const float4 &a) {return _mm_rsqrt_ps(a);};

	template <>
	inline bool any_true(const float4 &a) {return _mm_movemask_ps(a);};

	template <>
	inline bool any_false(const float4 &a) {return _mm_movemask_ps(a) != 0xFF;};

	template <>
	inline float4 select(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_blendv_ps(c,b,a);
	};

	template <>
	inline float4 rcp(const float4 &a) {return _mm_rcp_ps(a);};

	template <>
	inline float4 fmadd(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_fmadd_ps(a,b,c);
	};

	template <>
	inline float4 fmsub(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_fmsub_ps(a,b,c);
	};

	template <>
	inline float4 fnmadd(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_fnmadd_ps(a,b,c);
	};

	template <>
	inline float4 fnmsub(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_fnmsub_ps(a,b,c);
	};

	inline float4 blend(const float4 &a, const float4 &b, const float4 &c)
	{
		return _mm_blendv_ps(a,b,c);
	};
};

struct float8
{
	private:
	union
	{
		__m256 x;
		float q[8];
	};

	public:
	inline float8() {};
	
	inline float8(const __m256 &m) : x(m) {};
	
	inline float8(const float a)
	{
		for(int i = 0; i < 8; i++)
			q[i] = a;
	};
	
	inline float8(const int a)
	{
		for(int i = 0; i < 8; i++)
			q[i] = (float)a;
	};
	
	inline float8(const double a)
	{
		for(int i = 0; i < 8; i++)
			q[i] = (float)a;
	};
	
	inline float8(float* f)
	{
		for(int i = 0; i < 8; i++)
			q[i] = f[i];
	};
	
	inline float8(
		const float a, const float b, const float c, const float d,
		const float e, const float f, const float g, const float h
	)
	{
		q[0] = a; q[1] = b; q[2] = c; q[3] = d;
		q[4] = e; q[5] = f; q[6] = g; q[7] = h;
	};
	
	inline operator __m256() const {return this->x;};

	inline float8 operator+(const float8 &a) const
	{
		float8 r;
		for(int i = 0; i < 8; i++)
			r.q[i] = q[i] + a.q[i];
		return r;
	};

	inline float8 operator-(const float8 &a) const
	{
		float8 r;
		for(int i = 0; i < 8; i++)
			r.q[i] = q[i] - a.q[i];
		return r;
	};

	inline float8 operator*(const float8 &a) const
	{
		float8 r;
		for(int i = 0; i < 8; i++)
			r.q[i] = q[i] * a.q[i];
		return r;
	};

	inline float8 operator/(const float8 &a) const
	{
		float8 r;
		for(int i = 0; i < 8; i++)
			r.q[i] = q[i] / a.q[i];
		return r;
	};

	inline void operator+=(const float8 &a)
	{
		for(int i = 0; i < 8; i++)
			q[i] += a.q[i];
	};

	inline void operator-=(const float8 &a)
	{
		for(int i = 0; i < 8; i++)
			q[i] -= a.q[i];
	};

	inline void operator*=(const float8 &a)
	{
		for(int i = 0; i < 8; i++)
			q[i] *= a.q[i];
	};

	inline void operator/=(const float8 &a)
	{
		for(int i = 0; i < 8; i++)
			q[i] /= a.q[i];
	};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline float8 operator >(const float8 &a) const {return _mm256_cmp_ps(x,a.x,30);};
	inline float8 operator <(const float8 &a) const {return _mm256_cmp_ps(x,a.x,17);};
	inline float8 operator>=(const float8 &a) const {return _mm256_cmp_ps(x,a.x,29);};
	inline float8 operator<=(const float8 &a) const {return _mm256_cmp_ps(x,a.x,18);};
	inline float8 operator==(const float8 &a) const {return _mm256_cmp_ps(x,a.x,24);};
	inline float8 operator!=(const float8 &a) const {return _mm256_cmp_ps(x,a.x,12);};

	inline float8 operator&&(const float8 &a) const {return _mm256_and_ps(x,a.x);};
	inline float8 operator||(const float8 &a) const {return  _mm256_or_ps(x,a.x);};
	inline float8 operator &(const float8 &a) const {return _mm256_and_ps(x,a.x);};
	inline float8 operator |(const float8 &a) const {return  _mm256_or_ps(x,a.x);};
	inline float8 operator ^(const float8 &a) const {return _mm256_xor_ps(x,a.x);};

	inline float8 operator<<(int i) const {return (__m256)_mm256_slli_epi32((__m256i)x,i);}
	inline float8 operator>>(int i) const {return (__m256)_mm256_srli_epi32((__m256i)x,i);}
	
	//binary bit flip, useful for inverting comparison results
	inline float8 operator~() const {return (__m256)~(__m256i)x;}; //stupid...
	inline float8 operator!() const {return (__m256)~(__m256i)x;}; //stupid...

	//TODO: this could probably be made faster with some bit manipulation, first bit is sign
	inline float8 operator-(void) const
	{
		float8 ret;
		for(int i = 0; i < 8; i++)
			ret.q[i] = 0.0f - q[i];
		return ret;
	};

	//this might be very very slow... so be careful!
	inline float operator[](const unsigned int i) const {return q[i];};

	inline float& operator[](const unsigned int i) {return q[i];};

	inline void set(int i, float f)
	{
		q[i] = f;
	};

	std::string str() const
	{
		union
		{
			float f;
			uint32_t u;
		};

		std::string ret = "";
		for(int i = 0; i < 8; i++)
		{
			f = q[i];
			if(u == UINT32_MAX)
				ret += std::string("F8_TRUE,");
			else
				ret += std::to_string(f) + std::string(",");
		}
		ret.pop_back();
		return ret;
	};
};

namespace math
{
	template <>
	inline float8 min(const float8 &a, const float8 &b) {return _mm256_min_ps(a,b);};

	template <>
	inline float8 max(const float8 &a, const float8 &b) {return _mm256_max_ps(a,b);};

	template <>
	inline float8 abs(const float8 &a) {return _mm256_and_ps(a,_mm256_set1_ps(-0.0f));};

	template <>
	inline float8 sqrt(const float8 &a) {return _mm256_sqrt_ps(a);};

	template <>
	inline float8 rsqrt(const float8 &a) {return _mm256_rsqrt_ps(a);};

	template <>
	inline float8 sin(const float8 &a) {return sin256_ps(a);};

	template <>
	inline float8 cos(const float8 &a) {return cos256_ps(a);};

	template <>
	inline void sincos(const float8 &x, float8* s, float8* c)
	{
		sincos256_ps(x,(__m256*)s,(__m256*)c);
	};

	template <>
	inline bool any_true(const float8 &a) {return _mm256_movemask_ps(a);};

	template <>
	inline bool any_false(const float8 &a) {return _mm256_movemask_ps(a) != 0xFF;};

	template <>
	inline float8 select(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_blendv_ps(c,b,a);
	};

	template <>
	inline float8 rcp(const float8 &a) {return float8(1.0f) / a;};

	template <>
	inline float8 fmadd(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_fmadd_ps(a,b,c);
	};

	template <>
	inline float8 fmsub(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_fmsub_ps(a,b,c);
	};

	template <>
	inline float8 fnmadd(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_fnmadd_ps(a,b,c);
	};

	template <>
	inline float8 fnmsub(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_fnmsub_ps(a,b,c);
	};

	//template <>
	inline float8 blend(const float8 &a, const float8 &b, const float8 &c)
	{
		return _mm256_blendv_ps(a,b,c);
	};
};