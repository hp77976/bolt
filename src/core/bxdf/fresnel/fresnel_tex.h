#pragma once
#include "../../math/include.h"
#include "../../spectrum.h"

inline spectrum fr_full(
	const float cosi, const spectrum &cost,
	const spectrum &eta, const spectrum &k
)
{
	spectrum tmp = (eta * eta + k * k) + (cosi * cosi) + (cost * cost);
	spectrum rparl2 = (tmp - (2.0f * cosi * cost) * eta) / (tmp + (2.0f * cosi * cost) * eta);
	spectrum tmp_f = (eta* eta + k * k) * (cost * cost) + cosi * cosi;
	spectrum rperp2 = (tmp_f - (2.0f * cosi * cost) * eta) / (tmp_f + (2.0f * cosi * cost) * eta);
	return (rparl2 + rperp2) * 0.5f;
};

inline spectrum eval_fresnel_tex(const spectrum &eta, const spectrum &k, const float cosi)
{
	spectrum sint2 = std::max(0.0f,1.0f - cosi * cosi);
	if(cosi > 0.0f)
		sint2 = sint2 / (eta * eta);
	else
		sint2 = sint2 * (eta * eta);

	sint2 = clamp(sint2);

	spectrum cost2 = spectrum(1.0f) - sint2;

	if(cosi > 0.0f)
	{
		spectrum a = 2.0f * k * k * sint2;
		return fr_full(cosi,sqrt((cost2+sqrt(cost2*cost2+a*a))/2.0f),eta,k);
	}
	else
	{
		spectrum a = 2.0f * k * k * sint2;
		spectrum d2 = eta * eta + k * k;
		return fr_full(-cosi,sqrt((cost2+sqrt(cost2*cost2+a*a))/2.0f),eta/d2,-k/d2);
	}
};