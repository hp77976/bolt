#pragma once
#include "../math/include.h"

//intended to be paired to a specific tri
//separated from tris as a tri can have multiple UVs
struct tri_uvs
{
	vec2f a, b, c;

	inline tri_uvs() : a(0.0f), b(0.0f), c(0.0f) {};

	inline tri_uvs(const vec2f &a_, const vec2f &b_, const vec2f &c_) : a(a_), b(b_), c(c_) {};

	inline vec2f operator[](int i) const 
	{
		switch(i)
		{
			default:
			case 0: return a;
			case 1: return b;
			case 2: return c;
		}
	};

	inline vec2f& operator[](int i)
	{
		switch(i)
		{
			default:
			case 0: return a;
			case 1: return b;
			case 2: return c;
		}
	};
};

struct uv_map
{
	uint64_t id;
	uint32_t offset; //tri index offset
	std::vector<tri_uvs> uv_coords;
};