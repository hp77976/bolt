#pragma once
#include "../math/include.h"
#include "xorshiro128_plus.h"
#include "xorshiro128plusplus.h"
#include "xorshiro256_plus.h"
#include "xorshiro256plusplus.h"
#include "splitmix64.h"
#include <random>
#include <algorithm>
#include <cstdlib>
#include <memory.h>
#include <memory>
#include <time.h>
#include <string.h>
#include <string>

struct rng_state
{
	uint32_t s32[4];
	uint32_t s64[8];
};

class random_source //provides uniform distribution of floats and uints, not thread safe
{
	mutable xorshiro128_plus xos128p; //32 bit floats
	mutable xorshiro256_plus xos256p; //64 bit floats
	mutable xorshiro256_plusplus xos256pp; //64 bit uints
	
	public:
	random_source(int offset) //offset should utilize thread id to ensure unique seeds
	{
		std::mt19937_64 gen64;
#ifdef DETERMINISTIC
		gen64.seed(0);
#else
		gen64.seed(time(nullptr));
#endif
		gen64.discard(offset);
		uint64_t seed = gen64();
		uint64_t s64[4];
		
		for(int i = 0; i < 4; i++)
			s64[i] = splitmix_next(gen64());
		xos256p.seed(s64);
		xos256p.jump();
	
		for(int i = 0; i < 4; i++)
			s64[i] = splitmix_next(gen64());
		xos256pp.seed(s64);
		xos256pp.jump();

		for(int i = 0; i < 4; i++)
			s64[i] = splitmix_next(gen64());
		uint32_t s32[4];
		memcpy(s32,s64,sizeof(uint32_t)*4);
		xos128p.seed(s32);
		xos128p.jump();
	};

	rng_state save_state() const
	{
		rng_state rs;
		for(int i = 0; i < 4; i++)
			rs.s32[i] = xos128p.s[i];
		for(int i = 0; i < 4; i++)
			rs.s64[i+0] = xos256p.s[i];
		for(int i = 0; i < 4; i++)
			rs.s64[i+4] = xos256pp.s[i];
		return rs;
	};

	void load_state(const rng_state &rs)
	{
		for(int i = 0; i < 4; i++)
			xos128p.s[i] = rs.s32[i];
		for(int i = 0; i < 4; i++)
			xos256p.s[i] = rs.s64[i+0];
		for(int i = 0; i < 4; i++)
			xos256pp.s[i] = rs.s64[i+4];
	};

	template <typename T>
	inline void shuffle(T arr, int size)
	{
		for(int i = 0; i < size; i++)
			std::swap(arr[i],arr[get_u64()%size]);
	};

	/*template <>
	inline void shuffle(float8* arr, int size)
	{
		for(int j = 0; j < 8; j++)
		{
			for(int i = size - 1; i > -1; i--)
			{
				std::swap(arr[i][j],arr[get_u64()%size][j]);
			}
		}
	};*/

#ifndef RNG_USE_DOUBLE
	float get_float() const {return xos128p.get_float();};
#else
	float get_float() const {return xos256p.get_float();};
#endif
	vec2f get_vec2f() const {return vec2f(get_float(),get_float());};

	vec3f get_vec3f() const {return vec3f(get_float(),get_float(),get_float());};

	vec4f get_vec4f() const {return vec4f(get_float(),get_float(),get_float(),get_float());};

	float8 get_float8() const {return float8(
		get_float(),get_float(),get_float(),get_float(),
		get_float(),get_float(),get_float(),get_float()
	);};

	vec2f8 get_vec2f8() const {return vec2f8(get_float8(),get_float8());};

	vec3f8 get_vec3f8() const {return vec3f8(get_float8(),get_float8(),get_float8());};

	vec4f8 get_vec4f8() const {return vec4f8(get_float8(),get_float8(),get_float8(),get_float8());};

	uint64_t get_u64() const {return xos256pp.next();};

	//the following functions are for testing purposes
	float get_uf32() const {return xos128p.get_float();};

	double get_double() const {return xos256p.get_double();};

	float get_sfloat() const {return xos128p.get_sfloat();};

	float get_ufloat() const {return xos128p.get_ufloat();};
};

// Van der Corput radical inverse to fp32
inline float van_der_corput_f32(uint32_t bits, uint32_t scramble)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
	
	bits = (bits >> (32-24)) ^ (scramble & ~-(1<<24));
	return (float) bits / (float) (1U << 24);
};

inline float8 van_der_corput_f32_8(float8 bits, float8 scramble)
{
	union temp
	{
		__m256 f;
		uint32_t b[8];
	};

	uint32_t c[8] = {
		0x00ff00ff,0xff00ff00,0x0f0f0f0f,0xf0f0f0f0,
		0x33333333,0xcccccccc,0x55555555,0xaaaaaaaa
	};

	temp t[8];
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 8; j++)
			t[i].b[j] = c[i];

	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & t[0].f) << 8) | ((bits & t[1].f) >> 8);
	bits = ((bits & t[2].f) << 4) | ((bits & t[3].f) >> 4);
	bits = ((bits & t[4].f) << 2) | ((bits & t[5].f) >> 2);
	bits = ((bits & t[6].f) << 1) | ((bits & t[7].f) >> 1);

	bits = (bits >> (32-24)) ^ (scramble & ~-(1<<24));
	temp s;
	for(int i = 0; i < 8; i++)
		s.b[i] = (1U << 24);
	return bits / float8(s.f);
};