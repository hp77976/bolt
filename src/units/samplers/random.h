#include "../../base/sampler.h"

class random_sampler : public sampler
{
	public:
	random_sampler(int offset) : sampler(offset) {};

	~random_sampler() {};

	random_sampler* clone(uint32_t offset) const
	{
		return new random_sampler(offset);
	};

	float get_float() const {return rng->get_float();};

	vec2f get_vec2f() const {return rng->get_vec2f();};

	vec3f get_vec3f() const {return rng->get_vec3f();};

	vec4f get_vec4f() const {return rng->get_vec4f();};

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

	void generate(const vec2f &p, uint32_t index) {};

	void generate8(uint32_t next_index) {};
};