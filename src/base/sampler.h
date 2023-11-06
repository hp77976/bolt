#pragma once
#include "../core/math/include.h"
#include "../core/random/rng.h"

#ifndef SAMPLER_DEPTH
#define SAMPLER_DEPTH 256
#endif

struct sampler_state
{
	rng_state rs;
	uint32_t indices[4];
};

class sampler
{
	protected:
	random_source* rng = nullptr;
	alignas(32) std::array<float,SAMPLER_DEPTH> f_array;
	alignas(32) std::array<float8,SAMPLER_DEPTH> f8_array; //2KB
	mutable uint32_t sample_index = 0;
	mutable uint32_t sample8_index = 0;
	/*std::array<float,SAMPLER_DEPTH> f1_array;
	std::array<vec2f,SAMPLER_DEPTH> f2_array;
	std::array<vec3f,SAMPLER_DEPTH> f3_array;
	std::array<vec4f,SAMPLER_DEPTH> f4_array;*/
	/*mutable uint32_t f1_index = 0;
	mutable uint32_t f2_index = 0;
	mutable uint32_t f3_index = 0;
	mutable uint32_t f4_index = 0;
	mutable uint32_t total_index = 0;*/

	public:
	sampler(int offset) {rng = new random_source(offset);};

	virtual sampler* clone(uint32_t offset) const = 0;

	virtual sampler_state save_state() const
	{
		sampler_state s;
		s.rs = rng->save_state();
		s.indices[0] = sample_index;
		/*s.indices[0] = f1_index;
		s.indices[1] = f2_index;
		s.indices[2] = f3_index;
		s.indices[3] = f4_index;*/
		return s;
	};

	virtual void load_state(const sampler_state &s)
	{
		rng->load_state(s.rs);
		sample_index = s.indices[0];
		/*f1_index = s.indices[0];
		f2_index = s.indices[1];
		f3_index = s.indices[2];
		f4_index = s.indices[3];*/
	};

	virtual ~sampler() {delete rng;};

	virtual inline float get_float() const = 0;

	virtual inline vec2f get_vec2f() const = 0;

	virtual inline vec3f get_vec3f() const = 0;

	virtual inline vec4f get_vec4f() const = 0;

	virtual inline float8 get_float8() const = 0;

	virtual inline vec2f8 get_vec2f8() const = 0;

	virtual inline vec3f8 get_vec3f8() const = 0;

	virtual inline vec4f8 get_vec4f8() const = 0;

	virtual void generate(const vec2f &p, uint32_t index) = 0;

	virtual void generate8(uint32_t index) = 0;

	inline uint32_t get_index() const {return sample_index;};

	inline void set_index(uint32_t i) {sample_index = i;};
};