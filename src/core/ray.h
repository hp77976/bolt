#pragma once
#include "math/epsilon.h" //math/include.h requires ray!
#include <x86intrin.h>
#include <xmmintrin.h>

#ifndef RAY_MIN_DIST
#	define RAY_MIN_DIST 1e-5f
#endif

#ifndef RAY_MAX_DIST
#	define RAY_MAX_DIST 1000000.0f //std::numeric_limits<float>::infinity()
#endif

struct ray
{
	vec3f o; //origin
	float mint = RAY_MIN_DIST;
	vec3f d; //direction
	float maxt = RAY_MAX_DIST;

	ray() : o(0.0f), mint(RAY_MIN_DIST), d(0.0f), maxt(RAY_MAX_DIST) {};

	ray(
		const vec3f &o_, const vec3f &d_,
		float mint_ = RAY_MIN_DIST
	) : o(o_), d(d_)
	{
		this->mint = epsilon::e(this->o);
		this->maxt = RAY_MAX_DIST;
	};
};

template <typename V, typename F>
struct ray_v
{
	vec3<F> o; //origin
	vec3<F> d; //direction
	F mint = RAY_MIN_DIST;
	F maxt;

	ray_v() : o(0.0f), d(0.0f), mint(RAY_MIN_DIST), maxt(RAY_MAX_DIST) {};

	ray_v(const vec3<F> &o_, const vec3<F> &d_, F mint_ = RAY_MIN_DIST) : o(o_), d(d_)
	{
		//mint = epsilon::e(o);
		maxt = (F)RAY_MAX_DIST;
	};

	ray_v(const ray &r)
	{
		o = vec3<F>((F)r.o.x,(F)r.o.y,(F)r.o.z);
		d = vec3<F>((F)r.d.x,(F)r.d.y,(F)r.d.z);
		mint = (F)r.mint;
		maxt = (F)r.maxt;
	};

	inline ray get(int i) const
	{
		return ray(vec3f(o.x[i],o.y[i],o.z[i]),vec3f(d.x[i],d.y[i],d.z[i]));
	};
};

using ray4 = ray_v<vec3f4,float4>;
using ray8 = ray_v<vec3f8,float8>;