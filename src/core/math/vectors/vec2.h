#pragma once
#include "../simd/include.h"
#include <stdint.h>
#include <stdio.h> //needed for debugging stuff
#include <cmath>
#include <half.h>

template <typename T>
struct vec2
{
	union
	{
		struct
		{
			union{T x,u;};
			union{T y,v;};
		};
		
		T __q[2];
	};

	inline vec2() {};
	inline vec2(const T &x_) : x(x_), y(x_) {};
	inline vec2(const T &x_, const T &y_) : x(x_), y(y_) {};
	inline vec2(const T x_[2]) : x(x_[0]), y(x_[1]) {};

	inline vec2 operator+(const vec2 &a) const {return vec2(x + a.x, y + a.y);};
	inline vec2 operator-(const vec2 &a) const {return vec2(x - a.x, y - a.y);};
	inline vec2 operator*(const vec2 &a) const {return vec2(x * a.x, y * a.y);};
	inline vec2 operator/(const vec2 &a) const {return vec2(x / a.x, y / a.y);};

	inline vec2 operator+(const T &a) const {return vec2(x + a, y + a);};
	inline vec2 operator-(const T &a) const {return vec2(x - a, y - a);};
	inline vec2 operator*(const T &a) const {return vec2(x * a, y * a);};
	inline vec2 operator/(const T &a) const {return vec2(x / a, y / a);};

	inline vec2& operator+=(const vec2 &a) {x += a.x; y += a.y; return *this;};
	inline vec2& operator-=(const vec2 &a) {x -= a.x; y -= a.y; return *this;};
	inline vec2& operator*=(const vec2 &a) {x *= a.x; y *= a.y; return *this;};
	inline vec2& operator/=(const vec2 &a) {x /= a.x; y /= a.y; return *this;};

	inline vec2& operator+=(const T &a) {x += a; y += a; return *this;};
	inline vec2& operator-=(const T &a) {x -= a; y -= a; return *this;};
	inline vec2& operator*=(const T &a) {x *= a; y *= a; return *this;};
	inline vec2& operator/=(const T &a) {x /= a; y /= a; return *this;};

	inline T operator[](int i) const {return __q[i];};

	inline T& operator[](int i) {return __q[i];};
};

template <typename T>
inline vec2<T> operator+(const T &a, const vec2<T> &b) {return vec2<T>(a + b.x, a + b.y);};

template <typename T>
inline vec2<T> operator-(const T &a, const vec2<T> &b) {return vec2<T>(a - b.x, a - b.y);};

template <typename T>
inline vec2<T> operator*(const T &a, const vec2<T> &b) {return vec2<T>(a * b.x, a * b.y);};

template <typename T>
inline vec2<T> operator/(const T &a, const vec2<T> &b) {return vec2<T>(a / b.x, a / b.y);};

using vec2f  = vec2<float>;
using vec2i  = vec2<int32_t>;
using vec2u  = vec2<uint32_t>;
using vec2f8 = vec2<float8>;
using vec2h  = vec2<half>;

template <typename T>
inline vec2<T> max(const vec2<T> &a, const vec2<T> &b)
{
	return vec2<T>(std::max(a.x,b.x), std::max(a.y,b.y));
};

template <typename T>
inline vec2<T> min(const vec2<T> &a, const vec2<T> &b)
{
	return vec2<T>(std::min(a.x,b.x), std::min(a.y,b.y));
};

template <>
inline vec2f8 max(const vec2f8 &a, const vec2f8 &b)
{
	return vec2f8(_mm256_max_ps(a.x,b.x),_mm256_max_ps(a.y,b.y));
};

template <>
inline vec2f8 min(const vec2f8 &a, const vec2f8 &b)
{
	return vec2f8(_mm256_min_ps(a.x,b.x),_mm256_min_ps(a.y,b.y));
};

inline vec2f ceil(const vec2f &a) {return vec2f(std::ceil(a.x),std::ceil(a.y));};

inline vec2f floor(const vec2f &a) {return vec2f(std::floor(a.x),std::floor(a.y));};