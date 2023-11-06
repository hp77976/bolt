#pragma once
#include "vectors/include.h"

inline void coord_sys(const vec3f &a, vec3f* const b, vec3f* const c)
{
	if(std::abs(a.x) > std::abs(a.y))
	{
		float inv_len = 1.0f / std::sqrt(a.x * a.x + a.z * a.z);
		*b = vec3f(-a.z * inv_len, 0.0f, a.x * inv_len);
	}
	else
	{
		float inv_len = 1.0f / std::sqrt(a.y * a.y + a.z * a.z);
		*b = vec3f(0.0f, a.z * inv_len, -a.y * inv_len);
	}

	*c = cross(a,*b);
};

//branchless simd
template <typename F>
inline void coord_sys(const vec3<F> &a, vec3<F>* const b, vec3<F>* const c)
{
	//TODO: this could be much better with shuffling/permutation

	F condition = (abs(a.x) > abs(a.y));

	F az2 = (a.z * a.z);

	F inv_len_a = rsqrt((a.x * a.x) + az2);
	vec3<F> b_a = vec3<F>(a.z * inv_len_a, F(0.0f), -a.x * inv_len_a);

	F inv_len_b = rsqrt((a.y * a.y) + az2);
	vec3<F> b_b = vec3<F>(F(0.0f), a.z * inv_len_b, -a.y * inv_len_b);

	//vec3<F> inv_len = select(condition,inv_len_a,inv_len_b);
	*b = select(condition,b_a,b_b);
	*c = cross(a,*b);
};

template <typename T = float>
struct frame
{
	public:
	vec3<T> s = T(0.0f); //??? tangent?
	vec3<T> t = T(0.0f); //tangent??? bitangent?
	vec3<T> n = T(0.0f); //normal

	inline frame() {};

	inline frame(const vec3<T> &n_) : n(n_) {coord_sys<T>(n,&s,&t);};

	inline frame(const vec3<T> &s_, const vec3<T> &t_, const vec3<T> &n_) : s(s_), t(t_), n(n_) {};

	inline vec3<T> to_local(const vec3<T> &v) const {return vec3<T>(dot(v,s),dot(v,t),dot(v,n));};

	inline vec3<T> to_world(const vec3<T> &v) const {return s * v.x + t * v.y + n * v.z;};
};

/*template <typename V, typename F>
struct frame_v
{
	public:
	V s = V(0.0f); //tangent
	V t = V(0.0f); //bitangent
	V n = V(0.0f); //normal

	inline frame_v() {};

	inline frame_v(const V &n_) : n(n_) {coord_sys<V,F>(n,&s,&t);};

	inline frame_v(const V &s_, const V &t_, const V &n_) : s(s_), t(t_), n(n_) {};

	inline V to_local(const V &v) const {return V(dot(v,s),dot(v,t),dot(v,n));};

	inline V to_world(const V &v) const {return s * v.x + t * v.y + n * v.z;};
};*/