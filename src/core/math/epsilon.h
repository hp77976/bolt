#pragma once
#include <math.h>
#include <algorithm>
#include <stdint.h>
#include "vectors/include.h"

#define DEFAULT_EPSILON_MIN 1e-9f
#define DEFAULT_EPSILON_MAX 1e-1f
#define DEFAULT_EPSILON_STATIC 1e-5f

// An epsilon that can be used as threshold for cos(theta). For instance:
// if (Dot(N, LightDir) < DEFAULT_COS_EPSILON_STATIC) return Spectrum();
#define DEFAULT_COS_EPSILON_STATIC 1e-4f

// This is about 1e-5f for values near 1.f
#define DEFAULT_EPSILON_DISTANCE_FROM_VALUE 0x80u

class epsilon
{
	private:
	union machine_float
	{
		float f;
		uint32_t i;
	};
	
	static constexpr float min_epsilon = DEFAULT_EPSILON_MIN;
	static constexpr float max_epsilon = DEFAULT_EPSILON_MAX;

	static inline float float_adv(const float val)
	{
		machine_float mf;
		mf.f = val;
		mf.i += DEFAULT_EPSILON_DISTANCE_FROM_VALUE;
		return mf.f;
	};

	/*
	NOTE: can't use actual intrinsics safely for these as they don't allow for
	unsigned 32 bit int addition. using signed addition could result in weird
	behavior. doing this "manually" avoids that possibility.
	*/

	public:
	static inline float e(const float value)
	{
		float e_ = std::fabs(float_adv(value)-value);
		return std::clamp(e_,min_epsilon,max_epsilon);
	};

	static inline float e(const vec3f v) {return std::max(e(v.x),std::max(e(v.y),e(v.z)));};
};