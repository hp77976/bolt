#pragma once
#include "../math/include.h"
#include "tri.h"

/*template <typename T>
struct box //axis aligned bounding box
{
	vec3<T> min, max;

	inline box()
	{
		min = T( INF);
		max = T(-INF);
	};

	inline box(const tri &t)
	{
		min = T( INF);
		max = T(-INF);

		for(int32_t i = 0; i < 3; i++) //vertex
		{
			for(int32_t j = 0; j < 3; j++) //axis
			{
				min[j] = std::min(t[i].p[j],min[j]);
				max[j] = std::max(t[i].p[j],max[j]);
			}
		}
	};

	inline box(const vec3f &v0, const vec3f &v1)
	{
		for(int32_t j = 0; j < 3; j++) //axis
		{
			min[j] = std::min(v0[j],v1[j]);
			max[j] = std::max(v0[j],v1[j]);
		}
	};

	inline box(const box &b, const vec3f &v)
	{
		for(int32_t j = 0; j < 3; j++) //axis
		{
			min[j] = std::min(b.min[j],v[j]);
			max[j] = std::max(b.max[j],v[j]);
		}
	};

	inline box(const vec3f &v, const box &b)
	{
		box(b,v);
	};

	inline box(const box &b0, const box &b1)
	{
		for(int32_t j = 0; j < 3; j++) //axis
		{
			min[j] = std::min(b0.min[j],b1.min[j]);
			max[j] = std::max(b0.max[j],b1.max[j]);
		}
	};

	inline vec3f operator[](int i) const
	{
		switch(i)
		{
			default:
			case(0): return min; break;
			case(1): return max; break;
		}
	};

	inline vec3f& operator[](int i)
	{
		switch(i)
		{
			default:
			case(0): return min; break;
			case(1): return max; break;
		}
	};
};

inline bool overlap(const box &a, const box &b)
{
	bool xyz[3];
	
	for(uint32_t ai = 0; ai < 3; ai++) //axis index
		xyz[ai] = (a[1][ai] >= b[0][ai]) && (a[0][ai] <= b[1][ai]);

	return xyz[0] && xyz[1] && xyz[2];
};

inline bool validate(const box &b)
{
	return (
		(b[0][0] <= b[1][0]) &&
		(b[0][1] <= b[1][1]) &&
		(b[0][2] <= b[1][2])
	);
};

inline bool overlap(box &ret, const box &a, const box &b)
{
	if(!overlap(a,b))
		return false;

	for(uint32_t ai = 0; ai < 3; ai++)
	{
		ret[0][ai] = std::max(a[0][ai],b[0][ai]);
		ret[1][ai] = std::min(a[1][ai],b[1][ai]);
	}
	
	return true;
};

inline int32_t max_ext(const box &b)
{
	vec3f d = b[1] - b[0];

	if(d.x > d.y && d.x > d.z)
		return 0;
	else if(d.y > d.z)
		return 1;
	else
		return 2;
	
	return -1;
};

template <typename T>
inline T surf_area(const box<T> &b)
{
	vec3<T> d = b[1] - b[0]; //max - min
	return T(2.0f) * (d.x * d.y + d.y * d.z + d.z * d.x);
};

template <typename T>
struct box_v
{
	union
	{
		struct {T min, max;};
		T c[2];
	};

	inline box_v()
	{
		c[0] = T( INF);
		c[1] = T(-INF);
	};

	inline box_v(const box &b)
	{
		for(uint32_t i = 0; i < 2; i++)
			c[i] = T(b[i]);
	};

	inline box get_box(int32_t i) const
	{
		box b;
		
		for(uint32_t j = 0; j < 3; j++)
		{
			b.max[j] = c[0][j][i];
			b.max[j] = c[1][j][i];
		}

		return b;
	};

	inline void get_box(box &b, int32_t i) const
	{
		for(uint32_t j = 0; j < 3; j++)
		{
			b.max[j] = c[0][j][i];
			b.max[j] = c[1][j][i];
		}
	};

	inline void set_box(const box &b)
	{
		for(uint32_t i = 0; i < 2; i++)
			c[i] = T(b[i]);
	};

	inline T& operator[](int i) {return c[i];};

	inline T operator[](int i) const {return c[i];};
};*/