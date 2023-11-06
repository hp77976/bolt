#pragma once
#include "../simd/include.h"
#include "../templates.h"
#include <stdint.h>
#include <stdio.h> //needed for debugging stuff
#include <cmath>

template <typename T>
struct vec3
{
	union
	{
		struct
		{
			union{T x,r,u;};
			union{T y,g,v;};
			union{T z,b,w;};
		};
		
		T __q[3];
	};
	
	inline vec3() {};
	inline vec3(const T &x_) : x(T(x_)), y(T(x_)), z(T(x_)) {};
	inline vec3(const T &x_, const T &y_, const T &z_) : x(x_), y(y_), z(z_) {};
	inline vec3(const T x_[3]) : x(x_[0]), y(x_[1]), z(x_[2]) {};
	
	inline vec3 operator+(const vec3 &a) const {return vec3(x + a.x, y + a.y, z + a.z);};
	inline vec3 operator-(const vec3 &a) const {return vec3(x - a.x, y - a.y, z - a.z);};
	inline vec3 operator*(const vec3 &a) const {return vec3(x * a.x, y * a.y, z * a.z);};
	inline vec3 operator/(const vec3 &a) const {return vec3(x / a.x, y / a.y, z / a.z);};
	inline vec3 operator%(const vec3 &a) const {return vec3(y*a.z-z*a.y,z*a.x-x*a.z,x*a.y-y*a.x);};

	inline vec3& operator+=(const vec3 &a) {x += a.x; y += a.y; z += a.z; return *this;};
	inline vec3& operator-=(const vec3 &a) {x -= a.x; y -= a.y; z -= a.z; return *this;};
	inline vec3& operator*=(const vec3 &a) {x *= a.x; y *= a.y; z *= a.z; return *this;};
	inline vec3& operator/=(const vec3 &a) {x /= a.x; y /= a.y; z /= a.z; return *this;};

	inline vec3 operator+(const T &a) const {return vec3(x + a, y + a, z + a);};
	inline vec3 operator-(const T &a) const {return vec3(x - a, y - a, z - a);};
	inline vec3 operator*(const T &a) const {return vec3(x * a, y * a, z * a);};
	inline vec3 operator/(const T &a) const {return vec3(x / a, y / a, z / a);};

	inline vec3& operator+=(const T &a) {x += a; y += a; z += a; return *this;};
	inline vec3& operator-=(const T &a) {x -= a; y -= a; z -= a; return *this;};
	inline vec3& operator*=(const T &a) {x *= a; y *= a; z *= a; return *this;};
	inline vec3& operator/=(const T &a) {x /= a; y /= a; z /= a; return *this;};

	inline vec3 operator-(void) const {return vec3(-x,-y,-z);};
	inline bool operator==(const vec3 &a) const {return x==a.x&&y==a.y&&z==a.z;};
	inline bool operator!=(const vec3 &a) const {return x!=a.x&&y!=a.y&&z!=a.z;};

	inline T operator[](int i) const {return __q[i];};

	inline T& operator[](int i) {return __q[i];};
};

using vec3f  = vec3<float>;
using vec3f4 = vec3<float4>;
using vec3f8 = vec3<float8>;
using vec3d  = vec3<double>;
using vec3d4 = vec3<double4>;
using vec3i  = vec3<int32_t>;
using vec3i4 = vec3<int32_4>;
using vec3i8 = vec3<int32_8>;
using vec3h  = vec3<half>;

template <typename T>
inline vec3<T> operator+(const T &a, const vec3<T> &b) {return vec3<T>(a+b.x,a+b.y,a+b.z);};

template <typename T>
inline vec3<T> operator-(const T &a, const vec3<T> &b) {return vec3<T>(a-b.x,a-b.y,a-b.z);};

template <typename T>
inline vec3<T> operator*(const T &a, const vec3<T> &b) {return vec3<T>(a*b.x,a*b.y,a*b.z);};

template <typename T>
inline vec3<T> operator/(const T &a, const vec3<T> &b) {return vec3<T>(a/b.x,a/b.y,a/b.z);};

namespace math
{
	template <typename T>
	inline vec3<T> min(const vec3<T> &a, const vec3<T> &b)
	{
		return vec3<T>(min(a.x,b.x),min(a.y,b.y),min(a.z,b.z));
	};

	template <typename T>
	inline vec3<T> max(const vec3<T> &a, const vec3<T> &b)
	{
		return vec3<T>(max(a.x,b.x),max(a.y,b.y),max(a.z,b.z));
	};

	template <typename T>
	inline T max(const vec3<T> &a) {return max(max(a.x,a.y),a.z);};

	template <typename T>
	inline T min(const vec3<T> &a) {return min(min(a.x,a.y),a.z);};

	template <typename T>
	inline vec3<T> rcp(const vec3<T> &a) {return vec3<T>(rcp(a.x),rcp(a.y),rcp(a.z));};

	template <typename T>
	inline vec3<T> cross(const vec3<T> &a, const vec3<T> &b)
	{
		T r0 = a.y * b.z;
		T r1 = a.z * b.y;
		T r2 = r0 - r1;

		T r3 = a.z * b.x;
		T r4 = a.x * b.z;
		T r5 = r3 - r4;

		T r6 = a.x * b.y;
		T r7 = a.y * b.x;
		T r8 = r6 - r7;

		return vec3<T>(r2,r5,r8);
	};

	template <typename T>
	inline T dot(const vec3<T> &a, const vec3<T> &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	};

	template <typename T>
	inline vec3<T> abs(const vec3<T> &a)
	{
		return vec3<T>(abs(a.x),abs(a.y),abs(a.z));
	};

	template <typename T>
	inline T len_sqr(const vec3<T> &a)
	{
		T r0 = a.x * a.x;
		T r1 = a.y * a.y;
		T r2 = a.z * a.z;
		return r0 + r1 + r2;
	};

	template <typename T>
	inline T len(const vec3<T> &a) {return sqrt(len_sqr(a));};

	template <typename T>
	inline T distance(const vec3<T> &a, const vec3<T> &b) {return len(a - b);};

	template <typename T>
	inline T mag(vec3<T> a) {return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);};

	template <typename T>
	inline vec3<T> sqrt(const vec3<T> &v)
	{
		return vec3<T>(sqrt(v.x),sqrt(v.y),sqrt(v.z));
	};

	template <typename T>
	inline vec3<T> normalize(const vec3<T> &a)
	{
		return a / len(a);
	};

	template<typename T>
	inline vec3<T> avg(const vec3<T> &a, const vec3<T> &b)
	{
		return ((a + b) * T(0.5));
	};

	template<typename T>
	inline vec3<T> avg(const vec3<T> &a, const vec3<T> &b, const vec3<T> &c)
	{
		return ((a + b + c) * T(0.3333333333333333));
	};

	inline void ons(const vec3f &a, vec3f* b, vec3f* c)
	{
		if (std::abs(a.x) > std::abs(a.y))
		{
			// project to the y = 0 plane and construct a normalized orthogonal vector in this plane
			float invLen = 1.f / std::sqrt(a.x * a.x + a.z * a.z);
			*b = vec3f(-a.z * invLen, 0.0f, a.x * invLen);
		}
		else
		{
			// project to the x = 0 plane and construct a normalized orthogonal vector in this plane
			float invLen = 1.0f / std::sqrt(a.y * a.y + a.z * a.z);
			*b = vec3f(0.0f, a.z * invLen, -a.y * invLen);
		}

		*c = a % *b;
	};

	template <typename T>
	inline vec3<T> face_forward(const vec3<T> &n, const vec3<T> &v)
	{
		return (dot(n,v) < T(0.0f)) ? -n : n;
	};

	template <typename T>
	inline vec3<T> permute(const vec3<T> &a, int p0, int p1, int p2)
	{
		return vec3<T>(a[p0],a[p1],a[p2]);
	};

	template <typename T>
	inline vec3<T> select(const T &cond, const vec3<T> &a, const vec3<T> &b)
	{
		return vec3<T>(select(cond,a.x,b.x),select(cond,a.y,b.y),select(cond,a.z,b.z));
	};

	template <typename T>
	inline vec3<T> shuffle(int i0, int i1, int i2, const vec3<T> &v)
	{
		return vec3<T>(v[i0],v[i1],v[i2]);
	};

	//yes this is leaky. i don't care. its for debugging.
	inline char* print(const vec3f &v)
	{
		char* str = (char*)malloc(32);
		sprintf(str,"%2.3f %2.3f %2.3f",v.x,v.y,v.z);
		return str;
	};
};