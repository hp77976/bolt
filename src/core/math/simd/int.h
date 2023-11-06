#pragma once
#include <xmmintrin.h>
#include <immintrin.h>
#include <x86intrin.h>

#include <string>

inline int32_t get(const __m256i &m, const int idx) {return reinterpret_cast<const int32_t*>(&m)[idx];};

inline int64_t get64(const __m256i &m, const int idx) {return reinterpret_cast<const int64_t*>(&m)[idx];};

inline int32_t get(const __m128i &m, const int idx) {return reinterpret_cast<const int32_t*>(&m)[idx];};

struct int32_4
{
	private:
	__m128i x;

	public:
	inline int32_4() {};
	inline int32_4(const __m128i& m) : x(m) {};
	inline int32_4(const int32_t a) : x(_mm_set1_epi32(a)) {};
	inline int32_4(const int32_t f[4]){x = _mm_load_epi32(f);};
	inline int32_4(
		const int32_t a, const int32_t b, const int32_t c, const int32_t d
	){x = _mm_set_epi32(a,b,c,d);};

	inline operator __m128i() const {return this->x;};

	inline int32_4 operator+(const int32_4 &a) const {return _mm_add_epi32(x,a.x);};
	inline int32_4 operator-(const int32_4 &a) const {return _mm_sub_epi32(x,a.x);};
	inline int32_4 operator*(const int32_4 &a) const {return _mm_mullo_epi32(x,a.x);};

	inline void operator+=(const int32_4 &a) {x = _mm_add_epi32(x,a.x);};
	inline void operator-=(const int32_4 &a) {x = _mm_sub_epi32(x,a.x);};
	inline void operator*=(const int32_4 &a) {x = _mm_mullo_epi32(x,a.x);};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline int32_4 operator >(const int32_4 &a)
	const {return  _mm_cmpgt_epi32(x,a.x);};
	inline int32_4 operator <(const int32_4 &a)
	const {return ~((x > a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_4 operator>=(const int32_4 &a)
	const {return  ((x > a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_4 operator<=(const int32_4 &a)
	const {return  ((x < a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_4 operator==(const int32_4 &a)
	const {return  _mm_cmpeq_epi32(x,a.x);};
	inline int32_4 operator!=(const int32_4 &a)
	const {return  ~(x == a.x);}; //reuse eq

	//stupid hacky solution to missing operators
	inline int32_4 operator&&(const int32_4 &a) const
	{return (__m128i)_mm_and_ps((__m128)x,(__m128)a.x);};
	inline int32_4 operator||(const int32_4 &a) const
	{return (__m128i) _mm_or_ps((__m128)x,(__m128)a.x);};
	inline int32_4 operator &(const int32_4 &a) const
	{return (__m128i)_mm_and_ps((__m128)x,(__m128)a.x);};
	inline int32_4 operator |(const int32_4 &a) const
	{return (__m128i) _mm_or_ps((__m128)x,(__m128)a.x);};
	inline int32_4 operator ^(const int32_4 &a) const
	{return (__m128i)_mm_xor_ps((__m128)x,(__m128)a.x);};
	
	//binary bit flip, useful for inverting comparison results
	inline int32_4 operator~() const {return ~x;};

	inline int32_4 operator-(void) const {return _mm_sub_epi32(_mm_set1_epi32(0),x);};

	//this might be very very slow... so be careful!
	inline int32_t operator[](const unsigned int i) const 
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

struct int32_8
{
	private:
	union
	{
		__m256i x;
		int32_t q[8];
	};

	public:
	inline int32_8() {};
	inline int32_8(const __m256i& m) : x(m) {};
	inline int32_8(const int32_t a) : x(_mm256_set1_epi32(a)) {};
	inline int32_8(const int32_t f[8])
	{
		for(int i = 0; i < 8; i++)
			q[i] = f[i];
	};
	inline int32_8(
		const int32_t a, const int32_t b, const int32_t c, const int32_t d,
		const int32_t e, const int32_t f, const int32_t g, const int32_t h
	) //{x = _mm256_set_epi32(a,b,c,d,e,f,g,h);};
	{
		q[0] = a; q[1] = b; q[2] = c; q[3] = d;
		q[4] = e; q[5] = f; q[6] = g; q[7] = h;
	};

	inline operator __m256i() const {return this->x;};

	inline int32_8 operator+(const int32_8 &a) const {return _mm256_add_epi32(x,a.x);};
	inline int32_8 operator-(const int32_8 &a) const {return _mm256_sub_epi32(x,a.x);};
	inline int32_8 operator*(const int32_8 &a) const {return _mm256_mullo_epi32(x,a.x);};

	inline void operator+=(const int32_8 &a) {x = _mm256_add_epi32(x,a.x);};
	inline void operator-=(const int32_8 &a) {x = _mm256_sub_epi32(x,a.x);};
	inline void operator*=(const int32_8 &a) {x = _mm256_mullo_epi32(x,a.x);};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline int32_8 operator >(const int32_8 &a)
	const {return  _mm256_cmpgt_epi32(x,a.x);};
	inline int32_8 operator <(const int32_8 &a)
	const {return ~((x > a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_8 operator>=(const int32_8 &a)
	const {return  ((x > a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_8 operator<=(const int32_8 &a)
	const {return  ((x < a.x) || (x == a.x));}; //this op is *very* slow
	inline int32_8 operator==(const int32_8 &a)
	const {return  _mm256_cmpeq_epi32(x,a.x);};
	inline int32_8 operator!=(const int32_8 &a)
	const {return  ~(x == a.x);}; //reuse eq

	//stupid hacky solution to missing operators
	inline int32_8 operator&&(const int32_8 &a) const
	{return (__m256i)_mm256_and_ps((__m256)x,(__m256)a.x);};
	inline int32_8 operator||(const int32_8 &a) const
	{return (__m256i) _mm256_or_ps((__m256)x,(__m256)a.x);};
	inline int32_8 operator &(const int32_8 &a) const
	{return (__m256i)_mm256_and_ps((__m256)x,(__m256)a.x);};
	inline int32_8 operator |(const int32_8 &a) const
	{return (__m256i) _mm256_or_ps((__m256)x,(__m256)a.x);};
	inline int32_8 operator ^(const int32_8 &a) const
	{return (__m256i)_mm256_xor_ps((__m256)x,(__m256)a.x);};
	
	//binary bit flip, useful for inverting comparison results
	inline int32_8 operator~() const {return ~x;};

	inline int32_8 operator-(void) const {return _mm256_sub_epi32(_mm256_set1_epi32(0),x);};

	inline int32_t operator[](const unsigned int i) const {return q[i];};

	inline void set(int i, float f) {q[i] = f;};
};

struct int64_8
{
	private:
	union
	{
		struct{__m256i x, y;};
		int64_t z[8];
	};
	
	public:
	inline int64_8() {};
	inline int64_8(const __m256i& m, const __m256i& n) : x(m), y(n) {};
	inline int64_8(const int64_t a) : x(_mm256_set1_epi64x(a)), y(_mm256_set1_epi64x(a)) {};
	inline int64_8(const int64_t f[8]){x = _mm256_load_epi64(f);};
	inline int64_8(
		const int64_t a, const int64_t b, const int64_t c, const int64_t d,
		const int64_t e, const int64_t f, const int64_t g, const int64_t h
	){x = _mm256_set_epi32(a,b,c,d,e,f,g,h);};

	inline operator __m256i() const {return x;};

	inline int64_8 operator+(const int64_8 &a) const
	{return int64_8(_mm256_add_epi64(x,a.x),_mm256_add_epi64(y,a.y));};
	inline int64_8 operator-(const int64_8 &a) const
	{return int64_8(_mm256_sub_epi64(x,a.x),_mm256_sub_epi64(y,a.y));};
	
	//have to do mul and div in software
	inline int64_8 operator*(const int64_8 &a) const
	{
		int64_t r[8];
		
		for(int i = 0; i < 4; i++) r[i+0] = get64(x,i) * get64(a.x,i);
		for(int i = 0; i < 4; i++) r[i+4] = get64(y,i) * get64(a.y,i);
		
		return int64_8(r);
	};

	inline int64_8 operator/(const int64_8 &a) const
	{
		int64_t r[8];
		
		for(int i = 0; i < 4; i++) r[i+0] = get64(x,i) / get64(a.x,i);
		for(int i = 0; i < 4; i++) r[i+4] = get64(y,i) / get64(a.y,i);
		
		return int64_8(r);
	};

	inline void operator+=(const int64_8 &a)
	{x = _mm256_add_epi64(x,a.x); y = _mm256_add_epi64(y,a.y);};
	inline void operator-=(const int64_8 &a)
	{x = _mm256_sub_epi64(x,a.x); y = _mm256_sub_epi64(y,a.y);};

	//these don't return std bools. each element is all 0s (false) or all 1s (true)
	inline int64_8 operator >(const int64_8 &a)
	const {return int64_8(_mm256_cmpgt_epi64(x,a.x),_mm256_cmpgt_epi64(y,a.y));};
	inline int64_8 operator <(const int64_8 &a)
	const {return ~((*this > a) || (*this == a));}; //this op is *very* slow
	inline int64_8 operator>=(const int64_8 &a)
	const {return  ((*this > a) || (*this == a));}; //this op is *very* slow
	inline int64_8 operator<=(const int64_8 &a)
	const {return  ((*this < a) || (*this == a));}; //this op is *very* slow
	inline int64_8 operator==(const int64_8 &a)
	const {return int64_8(_mm256_cmpeq_epi64(x,a.x),_mm256_cmpeq_epi64(y,a.y));};
	inline int64_8 operator!=(const int64_8 &a)
	const {return ~(*this == a);}; //reuse eq

	//stupid hacky solution to missing operators
	inline int64_8 operator&&(const int64_8 &a) const
	{return int64_8(x & a.x,y & a.y);};
	inline int64_8 operator||(const int64_8 &a) const
	{return int64_8(x | a.x,y | a.y);};
	inline int64_8 operator &(const int64_8 &a) const
	{return int64_8(x & a.x,y & a.y);};
	inline int64_8 operator |(const int64_8 &a) const
	{return int64_8(x | a.x,y | a.y);};
	inline int64_8 operator ^(const int64_8 &a) const
	{
		return int64_8(
			(__m256i)_mm256_xor_ps((__m256)x,(__m256)a.x),
			(__m256i)_mm256_xor_ps((__m256)y,(__m256)a.y)
		);
	};
	
	//binary bit flip, useful for inverting comparison results
	inline int64_8 operator~() const {return int64_8(~x,~y);};

	inline int64_8 operator-(void) const {return int64_8((int64_t)0) - *this;};

	//this might be very very slow... so be careful!
	inline int64_t operator[](const unsigned int i) const 
	{
		switch(i)
		{
			default:
			case 0: return get64(x,0); break;
			case 1: return get64(x,1); break;
			case 2: return get64(x,2); break;
			case 3: return get64(x,3); break;
			case 4: return get64(y,0); break;
			case 5: return get64(y,1); break;
			case 6: return get64(y,2); break;
			case 7: return get64(y,3); break;
		}
	};
};

inline std::string vec_to_str(const __m256i &x)
{
	std::string ret;

	for(int i = 0; i < 8; i++)
		ret.append(std::to_string(get(x,i))+" ");

	return ret;
};