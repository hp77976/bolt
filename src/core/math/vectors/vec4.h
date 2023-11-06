#pragma once
#include "../simd/include.h"
#include "../templates.h"
#include <stdint.h>
#include <stdio.h> //needed for debugging stuff
#include <cmath>

template <typename T>
struct vec4
{
	union
	{
		struct
		{
			union{T x,r;};
			union{T y,g;};
			union{T z,b;};
			union{T w,a;};
		};

		T __q[4];
	};

	inline vec4() {};
	inline vec4(const T &x_) : x(x_), y(x_), z(x_), w(x_) {};
	inline vec4(const T &x_, const T &y_, const T &z_) : x(x_), y(y_), z(z_) {};
	inline vec4(const T x_, const T y_, const T z_, const T w_):x(x_),y(y_),z(z_),w(w_) {};
	inline vec4(const T v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {};
	
	inline vec4 operator+(const vec4 &v) const {return vec4(x + v.x, y + v.y, z + v.z, w + v.w);};
	inline vec4 operator-(const vec4 &v) const {return vec4(x - v.x, y - v.y, z - v.z, w - v.w);};
	inline vec4 operator*(const vec4 &v) const {return vec4(x * v.x, y * v.y, z * v.z, w * v.w);};
	inline vec4 operator/(const vec4 &v) const {return vec4(x / v.x, y / v.y, z / v.z, w / v.w);};

	inline void operator+=(const vec4 &v) {x += v.x; y += v.y; z += v.z; w += v.w;};
	inline void operator-=(const vec4 &v) {x -= v.x; y -= v.y; z -= v.z; w -= v.w;};
	inline void operator*=(const vec4 &v) {x *= v.x; y *= v.y; z *= v.z; w *= v.w;};
	inline void operator/=(const vec4 &v) {x /= v.x; y /= v.y; z /= v.z; w /= v.w;};

	inline vec4 operator+(const T &v) const {return vec4(x + v, y + v, z + v, w + v);};
	inline vec4 operator-(const T &v) const {return vec4(x - v, y - v, z - v, w - v);};
	inline vec4 operator*(const T &v) const {return vec4(x * v, y * v, z * v, w * v);};
	inline vec4 operator/(const T &v) const {return vec4(x / v, y / v, z / v, w / v);};

	inline void operator+=(const T &v) {x += v; y += v; z += v; w += v;};
	inline void operator-=(const T &v) {x -= v; y -= v; z -= v; w -= v;};
	inline void operator*=(const T &v) {x *= v; y *= v; z *= v; w *= v;};
	inline void operator/=(const T &v) {x /= v; y /= v; z /= v; w /= v;};

	inline vec4 operator-() const {return vec4(-x,-y,-z,-w);};

	inline bool operator==(const vec4 &v) const {return x==v.x&&y==v.y&&z==v.z&&w==v.w;};

	inline T operator[](const unsigned int i) const {return __q[i];};

	inline T& operator[](const unsigned int i) {return __q[i];};
};

using vec4f  = vec4<float>;
using vec4f4 = vec4<float4>;
using vec4f8 = vec4<float8>;
using vec4d  = vec4<double>;
using vec4d4 = vec4<double4>;
using vec4i  = vec4<int32_t>;
using vec4i4 = vec4<int32_4>;
using vec4i8 = vec4<int32_8>;
using vec4uc = vec4<uint8_t>;

template <typename T>
inline vec4<T> operator+(const T &a, const vec4<T> &b) {return vec4<T>(a+b.x,a+b.y,a+b.z);};

template <typename T>
inline vec4<T> operator-(const T &a, const vec4<T> &b) {return vec4<T>(a-b.x,a-b.y,a-b.z);};

template <typename T>
inline vec4<T> operator*(const T &a, const vec4<T> &b) {return vec4<T>(a*b.x,a*b.y,a*b.z);};

template <typename T>
inline vec4<T> operator/(const T &a, const vec4<T> &b) {return vec4<T>(a/b.x,a/b.y,a/b.z);};

namespace math
{
	template <typename T>
	inline T dot(const vec4<T> &a, const vec4<T> &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	};

	template <typename T>
	inline vec4<T> operator+(const T &a, const vec4<T> &b) {return vec4<T>(a+b.x,a+b.y,a+b.z,a+b.w);};

	template <typename T>
	inline vec4<T> operator-(const T &a, const vec4<T> &b) {return vec4<T>(a-b.x,a-b.y,a-b.z,a-b.w);};

	template <typename T>
	inline vec4<T> operator*(const T &a, const vec4<T> &b) {return vec4<T>(a*b.x,a*b.y,a*b.z,a*b.w);};

	template <typename T>
	inline vec4<T> operator/(const T &a, const vec4<T> &b) {return vec4<T>(a/b.x,a/b.y,a/b.z,a/b.w);};

	template <typename T>
	inline vec4<T> min(const vec4<T> &a, const vec4<T> &b)
	{
		return vec4<T>(min(a.x,b.x),min(a.y,b.y),min(a.z,b.z),min(a.w,b.w));
	};

	template <typename T>
	inline vec4<T> max(const vec4<T> &a, const vec4<T> &b)
	{
		return vec4<T>(max(a.x,b.x),max(a.y,b.y),max(a.z,b.z),max(a.w,b.w));
	};

	template <typename T>
	inline vec4<T> rcp(const vec4<T> &a) {return vec4<T>(rcp(a.x),rcp(a.y),rcp(a.z),rcp(a.w));};

	template <typename T>
	inline vec4<T> shuffle(int i0, int i1, int i2, int i3, const vec4<T> &v)
	{
		return vec4<T>(v[i0],v[i1],v[i2],v[i3]);
	};

	template <typename T>
	inline T len_sqr(const vec4<T> &a) {return a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w;};

	template <typename T>
	inline vec4<T> len(const vec4<T> &a) {return sqrt(len_sqr(a));};

	template <typename T>
	inline vec4<T> normalize(const vec4<T> &a) {return a / len(a);};

	template <typename T>
	inline T distance(const vec4<T> &a, const vec4<T> &b) {return len(a - b);};
};