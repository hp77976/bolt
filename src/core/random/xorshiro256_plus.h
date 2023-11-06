#pragma once
#include <stdint.h>

struct alignas(32) xorshiro256_plus
{
	inline uint64_t rotl(const uint64_t x, int k)
	{
		return (x << k) | (x >> (64 - k));
	};

	mutable uint64_t s[4];

	inline uint64_t next(void)
	{
		uint64_t result = s[0] + s[3];
		const uint64_t t = s[1] << 17;

		s[2] ^= s[0]; s[3] ^= s[1];
		s[1] ^= s[2]; s[0] ^= s[3];

		s[2] ^= t;

		s[3] = rotl(s[3],45);

		return result;
	};

	inline void seed(uint64_t seeds[4])
	{
		for(int i = 0; i < 4; i++)
			s[i] = seeds[i];
	};

	inline void jump(void)
	{
		static const uint64_t JUMP[] = {
			0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
			0xa9582618e03fc9aa, 0x39abdc4529b1661c
		};

		uint64_t s0 = 0; uint64_t s1 = 0;
		uint64_t s2 = 0; uint64_t s3 = 0;

		for(int i = 0; i < (int)(sizeof JUMP / sizeof *JUMP); i++)
		{
			for(int b = 0; b < 64; b++)
			{
				if(JUMP[i] & UINT64_C(1) << b)
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
		static const uint64_t LONG_JUMP[] = {
			0x76e15d3efefdcbbf, 0xc5004e441c522fb3,
			0x77710069854ee241, 0x39109bb02acbe635
		};

		uint64_t s0 = 0; uint64_t s1 = 0;
		uint64_t s2 = 0; uint64_t s3 = 0;
		
		for(int i = 0; i < (int)(sizeof LONG_JUMP / sizeof *LONG_JUMP); i++)
		{
			for(int b = 0; b < 64; b++)
			{
				if(LONG_JUMP[i] & UINT64_C(1) << b)
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

	inline float get_float()
	{
		/*uint64_t val = next();
		uint64_t bits = 64;
		uint64_t shift = 11;

		double num = val >> shift;
		double den = (1<<(bits-shift))-1;
		double fval = num/den;
		return (float)fval;*/

		uint64_t x = next();
		double r = (x >> 11) * 0x1.0p-53;
		return (float)r;
	};

	inline double get_double()
	{
		uint64_t x = next();
		double r = (x >> 11) * 0x1.0p-53;
		return r;
	};
};
