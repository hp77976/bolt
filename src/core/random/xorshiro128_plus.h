#pragma once
#include <stdint.h>

struct alignas(32) xorshiro128_plus //aigned to get SSE working (hopefully)
{
	inline uint32_t rotl(const uint32_t x, int k)
	{
		return (x << k) | (x >> (32 - k));
	};

	mutable uint32_t s[4];

	inline uint32_t next(void)
	{
		uint32_t result = s[0] + s[3];
		const uint32_t t = s[1] << 9;

		s[2] ^= s[0]; s[3] ^= s[1];
		s[1] ^= s[2]; s[0] ^= s[3];

		s[2] ^= t;

		s[3] = rotl(s[3],11);

		return result;
	};

	inline void seed(uint32_t seeds[4])
	{
		for(uint32_t i = 0; i < 4; i++)
			s[i] = seeds[i];
	};

	inline void jump(void)
	{
		static const uint32_t JUMP[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};

		uint32_t s0 = 0; uint32_t s1 = 0;
		uint32_t s2 = 0; uint32_t s3 = 0;

		for(int i = 0; i < (int)(sizeof JUMP / sizeof *JUMP); i++)
		{
			for(int b = 0; b < 32; b++)
			{
				if(JUMP[i] & UINT32_C(1) << b)
				{
					s0 ^= s[0]; s1 ^= s[1];
					s2 ^= s[2]; s3 ^= s[3];
				}
				next();
			}
		}

		s[0] = s0; s[1] = s1;
		s[2] = s2; s[3] = s3;
	};

	inline void long_jump(void)
	{
		static const uint32_t LONG_JUMP[] = {0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662};

		uint32_t s0 = 0; uint32_t s1 = 0;
		uint32_t s2 = 0; uint32_t s3 = 0;
		
		for(int i = 0; i < (int)(sizeof LONG_JUMP / sizeof *LONG_JUMP); i++)
		{
			for(int b = 0; b < 32; b++)
			{
				if(LONG_JUMP[i] & UINT32_C(1) << b)
				{
					s0 ^= s[0]; s1 ^= s[1];
					s2 ^= s[2]; s3 ^= s[3];
				}
				next();
			}
		}

		s0 ^= s[0]; s1 ^= s[1];
		s2 ^= s[2]; s3 ^= s[3];
	};

	inline float ufloat(uint32_t v, uint32_t b, uint32_t s)
	{
		float n = v >> s;
		float d = (1<<(b-s))-1;
		float f = n/d;
		return f;
	};

	inline float sfloat(uint32_t v, uint32_t b, uint32_t s)
	{
		uint32_t i = v >> (b-1);
		uint32_t u = v & (UINT32_MAX>>1);
		float f = ufloat(u,b-1,s-1);
		return i ? -f : f;
	};

	inline float get_float()
	{
		uint32_t val = next();
		uint32_t bits = 32;
		uint32_t shift = 7;

		float num = val >> shift;
		float den = (1<<(bits-shift))-1;
		float fval = num/den;
		return fval;
	};

	inline float get_sfloat()
	{
		uint32_t val = next();
		uint32_t bits = 32;
		uint32_t shift = 7;

		return sfloat(val,bits,shift);
	};

	inline float get_ufloat()
	{
		uint32_t val = next();
		uint32_t bits = 31;
		uint32_t shift = 7;

		return ufloat(val,bits,shift);
	};
};
