#pragma once
#include <math.h>
#include <stdint.h>
#include <algorithm>

namespace math
{
	template <typename T>
	inline T min(const T &a, const T &b) {return std::min(a,b);};

	template <typename T>
	inline T max(const T &a, const T &b) {return std::max(a,b);};

	template <typename T>
	inline T abs(const T &a) {return std::abs(a);};

	template <typename T>
	inline T sqrt(const T &a) {return std::sqrt(a);};

	template <typename T>
	inline T rsqrt(const T &a) {return T(1) / sqrt(a);};

	template <typename T>
	inline T safe_sqrt(const T &a) {return sqrt(max(T(0),a));};

	template <typename T>
	inline T sin(const T &a) {return std::sin(a);};

	template <typename T>
	inline T cos(const T &a) {return std::cos(a);};

	template <typename T>
	inline T tan(const T &a) {return std::tan(a);};

	template <typename T>
	inline void sincos(const T &x, T *s, T *c) {sincosf(x,s,c);};

	template <typename T>
	inline bool any_true(const T &a) {return a;};

	template <typename T>
	inline bool any_false(const T &a) {return !a;};

	template <typename T>
	inline T select(bool a, const T &b, const T &c) {return a ? b : c;};

	template <typename T>
	inline T select(const T &a, const T &b, const T &c) {return a ? b : c;};

	template <typename T>
	inline T rcp(const T &a) {return T(1) / a;};

	template <typename T>
	inline T fmadd(const T &a, const T &b, const T &c) {return a * b + c;};

	template <typename T>
	inline T fmsub(const T &a, const T &b, const T &c) {return a * b - c;};

	template <typename T>
	inline T fnmadd(const T &a, const T &b, const T &c) {return -(a * b) + c;};

	template <typename T>
	inline T fnmsub(const T &a, const T &b, const T &c) {return -(a * b) - c;};
};