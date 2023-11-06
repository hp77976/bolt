#pragma once
#include "../core/math/include.h"
#include "../core/records.h"
#include "../core/spectrum.h"
#include "../core/bxdf/include.h"

class sampler;

class material
{
	protected:
	uint32_t m_type = 0;
	std::vector<uint32_t> components = {};

	inline void config()
	{
		m_type = 0;
		for(uint32_t i = 0; i < components.size(); i++)
			m_type |= components[i];
	};

	public:
	material() {config();};

	virtual spectrum sample(bsdf_record &b_rec, sampler* const s) const = 0;

	virtual spectrum eval(const bsdf_record &b_rec, e_measure measure) const = 0;

	virtual float pdf(const bsdf_record &rec, const vec2f &rng, e_measure measure) const = 0;

	inline uint32_t get_type() const {return m_type;};

	inline bool has_component(e_type_combos etype) const {return (m_type & etype) != 0;};

	inline static bool has_component(e_type_combos tc, uint32_t st) {return (st & tc) != 0;};

	inline static e_measure get_measure(uint32_t component_type)
	{
		if(component_type & ESmooth)
			return E_SOLID_ANGLE;
		else if(component_type & EDelta)
			return E_DISCRETE;
		else if(component_type & EDelta1D)
			return E_LENGTH;
		else
			return E_SOLID_ANGLE;
	};

	//named differently because pybind doesn't like this having the same name as the other sample
	inline spectrum sample8(bsdf_record8 &b_rec, sampler* const s, int i) const
	{
		bsdf_record b = b_rec.get(i);
		spectrum weight = sample(b,s);
		b_rec.set(i,b);
		return weight;
	};
};