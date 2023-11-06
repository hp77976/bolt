#pragma once
#include <xmmintrin.h>
#include <immintrin.h>
#include <x86intrin.h>

#include <string>

inline double get(const __m256d &m, const int idx)
{
	return reinterpret_cast<const double*>(&m)[idx];
};

struct double4
{
	private:
	__m256d x;

	public:
	inline double4() {};
	inline double4(const __m256d &m) : x(m) {};
	inline double4(const double a) : x(_mm256_set1_pd(a)) {};
	inline double4(const double f[4]){x = _mm256_load_pd(f);};
	inline double4(
		const double a, const double b, const double c, const double d
	){x = _mm256_set_pd(a,b,c,d);};

	inline operator __m256d() const {return this->x;};

	inline double4 operator+(const double4 &a) const {return _mm256_add_pd(x,a.x);};
	inline double4 operator-(const double4 &a) const {return _mm256_sub_pd(x,a.x);};
	inline double4 operator*(const double4 &a) const {return _mm256_mul_pd(x,a.x);};
	inline double4 operator/(const double4 &a) const {return _mm256_div_pd(x,a.x);};

	inline void operator+=(const double4 &a) {this->x = _mm256_add_pd(x,a.x);};
	inline void operator-=(const double4 &a) {this->x = _mm256_sub_pd(x,a.x);};
	inline void operator*=(const double4 &a) {this->x = _mm256_mul_pd(x,a.x);};
	inline void operator/=(const double4 &a) {this->x = _mm256_div_pd(x,a.x);};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline double4 operator >(const double4 &a) const {return _mm256_cmp_pd(x,a.x,30);};
	inline double4 operator <(const double4 &a) const {return _mm256_cmp_pd(x,a.x,17);};
	inline double4 operator>=(const double4 &a) const {return _mm256_cmp_pd(x,a.x,29);};
	inline double4 operator<=(const double4 &a) const {return _mm256_cmp_pd(x,a.x,18);};
	inline double4 operator==(const double4 &a) const {return _mm256_cmp_pd(x,a.x,24);};
	inline double4 operator!=(const double4 &a) const {return _mm256_cmp_pd(x,a.x,12);};

	inline double4 operator&&(const double4 &a) const {return _mm256_and_pd(x,a.x);};
	inline double4 operator||(const double4 &a) const {return  _mm256_or_pd(x,a.x);};
	inline double4 operator &(const double4 &a) const {return _mm256_and_pd(x,a.x);};
	inline double4 operator |(const double4 &a) const {return  _mm256_or_pd(x,a.x);};
	inline double4 operator ^(const double4 &a) const {return _mm256_xor_pd(x,a.x);};
	
	//binary bit flip, useful for inverting comparison results
	inline double4 operator~() const {return (__m256d)~(__m256i)x;}; //stupid...

	inline double4 operator-(void) const {return _mm256_sub_pd(_mm256_set1_pd(0.0f),x);};

	//this might be very very slow... so be careful!
	inline float operator[](const unsigned int i) const 
	{
		switch(i)
		{
			default:
			case 0: return get(x,0); break;
			case 1: return get(x,1); break;
			case 2: return get(x,2); break;
			case 3: return get(x,3); break;
		}
	};
};

inline double4  abs(const double4 &a) {return _mm256_andnot_pd(_mm256_set1_pd(-0.0f),a);};
inline double4 sqrt(const double4 &a) {return _mm256_sqrt_pd(a);};

inline double4 select(const double4 &cond, const double4 &a, const double4 &b)
{
	return (double4((__m256d)cond) & a) ^ (~double4((__m256d)cond) & b);
};

inline double4 clamp(const double4 &val, const double4 &low, const double4 &high)
{
	double4 ret = select((val < low),low,val);
	ret = select((ret > high),high,ret);
	return ret;
};

inline std::string vec_to_str(const __m256d &x)
{
	std::string ret;

	for(int i = 0; i < 4; i++)
		ret.append(std::to_string(get(x,i))+" ");

	return ret;
};