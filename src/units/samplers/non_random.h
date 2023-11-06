#include "../../base/sampler.h"

class non_random_sampler : public sampler
{
	public:
	non_random_sampler(int offset) : sampler(offset) {};

	~non_random_sampler() {};

	non_random_sampler* clone(uint32_t offset) const
	{
		return new non_random_sampler(offset);
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

	void get_array(float* f, int size)
	{
		for(int i = 0; i < size; i++)
			f[i] = get_float();
	};

	void generate(const vec2f &p, uint32_t index)
	{
		for(int i = 0; i < SAMPLER_DEPTH; i++)
			f_array[i] = float(i) / float(SAMPLER_DEPTH);

		rng->shuffle(f_array,SAMPLER_DEPTH);
		sample_index = index > SAMPLER_DEPTH ? 0 : index;
	};

	void generate8(uint32_t next_index)
	{

	};
};