#pragma once
#include "../../base/sampler.h"

class low_discrepancy_sampler : public sampler
{
	public:
	low_discrepancy_sampler(int offset) : sampler(offset) {};

	~low_discrepancy_sampler() {};

	low_discrepancy_sampler* clone(uint32_t offset) const
	{
		return new low_discrepancy_sampler(offset);
	};

	float get_float() const
	{
		sample_index++;
		if(sample_index < SAMPLER_DEPTH)
			return f_array[sample_index-1];

		return rng->get_float();
	};

	vec2f get_vec2f() const
	{
		sample_index += 2;
		if(sample_index < SAMPLER_DEPTH)
			return vec2f(&f_array[sample_index-2]);

		return rng->get_vec2f();
	};

	vec3f get_vec3f() const
	{
		sample_index += 3;
		if(sample_index < SAMPLER_DEPTH)
			return vec3f(&f_array[sample_index-3]);

		return rng->get_vec3f();
	};

	vec4f get_vec4f() const
	{
		sample_index += 4;
		if(sample_index < SAMPLER_DEPTH)
			return vec4f(&f_array[sample_index-4]);

		return rng->get_vec4f();
	};

	float8 get_float8() const {return float8(
		get_float(),get_float(),get_float(),get_float(),
		get_float(),get_float(),get_float(),get_float()
	);};

	vec2f8 get_vec2f8() const {return vec2f8(get_float8(),get_float8());};

	vec3f8 get_vec3f8() const {return vec3f8(get_float8(),get_float8(),get_float8());};

	vec4f8 get_vec4f8() const {return vec4f8(get_float8(),get_float8(),get_float8(),get_float8());};

	/*float8 get_float8() const
	{
		sample8_index++;
		if(sample8_index < SAMPLER_DEPTH)
			return f8_array[sample8_index-1];

		return rng->get_float8();
	};

	vec2f8 get_vec2f8() const
	{
		sample8_index += 2;
		if(sample8_index < SAMPLER_DEPTH)
			return vec2f8(&f8_array[sample8_index-2]);

		return rng->get_vec2f8();
	};

	vec3f8 get_vec3f8() const
	{
		sample8_index += 3;
		if(sample8_index < SAMPLER_DEPTH)
			return vec3f8(&f8_array[sample8_index-3]);

		return rng->get_vec3f8();
	};

	vec4f8 get_vec4f8() const
	{
		if(sample8_index += 4 < SAMPLER_DEPTH)
			return vec4f8(&f8_array[sample8_index-4]);

		return rng->get_vec4f8();
	};*/

	void get_array(float* f, int size)
	{
		for(int i = 0; i < size; i++)
			f[i] = get_float();
	};

	void generate(const vec2f &p, uint32_t next_index)
	{
		uint32_t scromble = rng->get_u64() & 0xFFFFFFFF;
		for(int i = 0; i < SAMPLER_DEPTH; i++)
			f_array[i] = van_der_corput_f32(i,scromble);
		
		rng->shuffle(f_array,SAMPLER_DEPTH);
		sample_index = next_index != ~(uint32_t)0 ? next_index : sample_index;
	};

	void generate8(uint32_t next_index)
	{
		next_index = sample8_index;
		for(int j = 0; j < 8; j++)
		{
			uint32_t scromble = rng->get_u64() & 0xFFFFFFFF;
			for(int i = 0; i < SAMPLER_DEPTH; i++)	
				f8_array[i][j] = van_der_corput_f32(i,scromble);
		}

		rng->shuffle(f8_array.data(),SAMPLER_DEPTH);
		rng->shuffle(f8_array.data(),SAMPLER_DEPTH);
		rng->shuffle(f8_array.data(),SAMPLER_DEPTH);
		rng->shuffle(f8_array.data(),SAMPLER_DEPTH);
		sample8_index = next_index != ~(uint32_t)0 ? next_index : sample8_index;
	};

	/*float get_float() const
	{
		total_index++;
		if(f1_index < SAMPLER_DEPTH)
			return f1_array[f1_index++];
		return rng->get_float();
	};

	vec2f get_vec2f() const
	{
		total_index++;
		if(f2_index < SAMPLER_DEPTH)
			return f2_array[f2_index++];
		return rng->get_vec2f();
	};

	vec3f get_vec3f() const
	{
		total_index++;
		if(f3_index < SAMPLER_DEPTH)
			return f3_array[f3_index++];
		return rng->get_vec3f();
	};

	vec4f get_vec4f() const
	{
		total_index++;
		if(f4_index < SAMPLER_DEPTH)
			return f4_array[f4_index++];
		return rng->get_vec4f();
	};

	void generate(uint32_t index)
	{
		uint32_t scromble = rng->get_u64() & 0xFFFFFFFF;
		for(int i = 0; i < SAMPLER_DEPTH; i++)
		{
			f1_array[i] = van_der_corput_f32(i,scromble);
			f2_array[i] = van_der_corput_f32(i,scromble);
			f3_array[i] = van_der_corput_f32(i,scromble);
			f4_array[i] = van_der_corput_f32(i,scromble);
		}
		
		rng->shuffle(f1_array,SAMPLER_DEPTH);
		f1_index = index != ~(uint32_t)0 ? index : f1_index;

		rng->shuffle(f2_array,SAMPLER_DEPTH);
		f2_index = index != ~(uint32_t)0 ? index : f2_index;

		rng->shuffle(f3_array,SAMPLER_DEPTH);
		f3_index = index != ~(uint32_t)0 ? index : f3_index;

		rng->shuffle(f4_array,SAMPLER_DEPTH);
		f4_index = index != ~(uint32_t)0 ? index : f4_index;

		total_index = 0;
	};*/
};