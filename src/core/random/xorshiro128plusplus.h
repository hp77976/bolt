#pragma once
#include <stdint.h>

struct xorshiro128_plusplus
{
	inline uint64_t rotl(const uint64_t x, int k)
	{
		return (x << k) | (x >> (64 - k));
	};

	union
	{
		mutable uint64_t s[2];
		mutable uint32_t q[4];
	};
	
	inline uint64_t next(void)
	{
		const uint64_t s0 = s[0];
		uint64_t s1 = s[1];
		const uint64_t result = rotl(s0 + s1, 17) + s0;

		s1 ^= s0;
		s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
		s[1] = rotl(s1, 28); // c

		return result;
	};

	inline void seed(int seeds[4])
	{
		q[0] = seeds[0];
		q[1] = seeds[1];
		q[2] = seeds[2];
		q[3] = seeds[3];
	};

	inline void jump(void)
	{
		static const uint64_t JUMP[] = { 0x2bd7a6a6e99c2ddc, 0x0992ccaf6a6fca05 };

		uint64_t s0 = 0;
		uint64_t s1 = 0;
		for(uint64_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
			for(uint64_t b = 0; b < 64; b++) {
				if (JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
	};

	inline void long_jump(void)
	{
		static const uint64_t LONG_JUMP[] = { 0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3 };

		uint64_t s0 = 0;
		uint64_t s1 = 0;
		for(uint64_t i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
			for(uint64_t b = 0; b < 64; b++) {
				if (LONG_JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
	};

	inline float get_float()
	{
		uint64_t val = next();
		uint64_t bits = 32;
		uint64_t shift = 7;

		float num = val >> shift;
		float den = (1<<(bits-shift))-1;
		float fval = num/den;
		return fval;
	};
};
