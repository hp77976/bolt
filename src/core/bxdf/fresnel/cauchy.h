#pragma once
#include "../../math/include.h"
#include "../../spectrum.h"
#include "dielectric.h"

inline void eval_cauchy_fresnel(
	const spectrum &s, spectrum* const f, float cosi, float ior, float cb
)
{
	if(cb != 0.0f)
	{
		spectrum eta = s * s;
		eta = spectrum(ior) + spectrum(cb) / eta;
		spectrum cost = spectrum(std::max(0.0f,1.0f - cosi * cosi));
		if(cosi > 0.0f)
			cost = cost / (eta * eta);
		else
			cost = cost * (eta * eta);
		
		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			cost.p[i] = clamp(cost.p[i]);

		for(int i = 0; i < SPECTRUM_SAMPLES; i++)
			cost.p[i] = std::sqrt(1.0f - cost.p[i]);
		
		fr_diel_2(std::abs(cosi),cost,cosi > 0.0f ? eta : spectrum(1.0f) / eta, f);
	}
	else
	{
		bool entering = cosi > 0.0f;
		float eta = ior;

		if(cb != 0.0f)
		{
			//this isn't possible yet
		}

		float eta2 = eta * eta;
		float sint2 = (entering ? 1.0f / eta2 : eta2) * std::max(0.0f,1.0f - cosi * cosi);

		if(sint2 >= 1.0f)
			*f = spectrum(1.0f);
		else
			fr_diel_2(
				std::abs(cosi),
				spectrum(std::sqrt(std::max(0.0f,1.0f - sint2))),
				entering ? eta : spectrum(1.0f) / eta,
				f
			);
	}
};

inline float eval_cauchy_fresnel2(float ior, float cosi)
{
	bool entering = cosi > 0.0f;
	float eta2 = ior * ior;
	float sint2 = (entering ? 1.0f / eta2 : eta2) * std::max(0.0f,1.0f - cosi * cosi);

	if(sint2 >= 1.0f)
		return 1.0f;
	else
		return fr_diel_2(std::abs(cosi),std::sqrt(std::max(0.0f,1.0f-sint2)),entering?ior:(1.0f/ior));
};