#pragma once
#include "../math/include.h"

struct vertex
{
	vec3f p; //position
	vec3f n; //normal

	inline vertex() {};

	inline vertex(const vec3f &p_, const vec3f &n_) : p(p_), n(n_) {};
};

struct tri
{
	vertex a, b, c; //vertices
	int64_t i; //self index
	int32_t m; //material index
	
	inline tri() {};

	inline tri(
		const vertex &a_, const vertex &b_, const vertex &c_, int64_t i_, int32_t m_
	) : a(a_), b(b_), c(c_), i(i_), m(m_) {};

	inline vertex& operator[](uint8_t i) 
	{
		switch(i)
		{
			default:
			case(0): return a; break;
			case(1): return b; break;
			case(2): return c; break;
		}
	};

	inline vertex operator[](uint8_t i) const
	{
		switch(i)
		{
			default:
			case(0): return a; break;
			case(1): return b; break;
			case(2): return c; break;
		}
	};
};

struct vertex8
{
	vec3f8 p;
	vec3f8 n;

	inline vertex8() : p(0.0f), n(0.0f) {};

	inline vertex8(const vec3f8 &p_, const vec3f8 &n_) : p(p_), n(n_) {};

	inline vertex8
	(
		vertex v0, vertex v1, vertex v2, vertex v3,
		vertex v4, vertex v5, vertex v6, vertex v7
	)
	{
		p = vec3f8(
			float8(v0.p.x,v1.p.x,v2.p.x,v3.p.x,v4.p.x,v5.p.x,v6.p.x,v7.p.x),
			float8(v0.p.y,v1.p.y,v2.p.y,v3.p.y,v4.p.y,v5.p.y,v6.p.y,v7.p.y),
			float8(v0.p.z,v1.p.z,v2.p.z,v3.p.z,v4.p.z,v5.p.z,v6.p.z,v7.p.z)
		);

		n = vec3f8(
			float8(v0.n.x,v1.n.x,v2.n.x,v3.n.x,v4.n.x,v5.n.x,v6.n.x,v7.n.x),
			float8(v0.n.y,v1.n.y,v2.n.y,v3.n.y,v4.n.y,v5.n.y,v6.n.y,v7.n.y),
			float8(v0.n.z,v1.n.z,v2.n.z,v3.n.z,v4.n.z,v5.n.z,v6.n.z,v7.n.z)
		);
	};
};