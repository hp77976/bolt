#pragma once
#include "../../math/include.h"
#include "../../spectrum.h"

template <typename T>
inline T deil_f(T &pa, T &pe, const T &ei, const T &ci, const T &ct)
{
	pa = fmsub(ei,ci,ct) / fmadd(ei,ci,ct);
	pe = fnmadd(ei,ct,ci) / fmadd(ei,ct,ci);

	pa = (ei * ci - ct) / (ei * ci + ct);
	pe = (ci - ei * ct) / (ei * ct + ci);
};

inline void fr_diel_2(float cosi, const spectrum &cost, const spectrum &eta, spectrum* const f)
{
	spectrum rparl = spectrum(eta * cosi);
	rparl = (cost - rparl) / (cost + rparl);
	spectrum rperp = spectrum(eta * cost);
	rperp = (spectrum(cosi) - rperp) / (spectrum(cosi) + rperp);
	*f = (rparl * rparl + rperp * rperp) * 0.5f;
};

inline float fr_diel_2(float cosi, float cost, float eta)
{
	float rparl = eta * cosi;
	rparl = (cost - rparl) / (cost + rparl);
	float rperp = eta * cost;
	rperp = (cosi - rperp) / (cosi + rperp);
	return (rparl * rparl + rperp * rperp) * 0.5f;
};

inline float fr_diel(float cost, float ior)
{
	float cost_i = std::clamp(cost,-1.0f,1.0f);
	float eta_i = ior;
	if(cost_i < 0.0f)
	{
		eta_i = 1.0f / eta_i;
		cost_i = -cost_i;
	}

	float s2ti = fnmadd(cost_i,cost_i,1.0f);
	float s2tt = s2ti / (eta_i * eta_i);
	if(s2tt >= 1.0f)
		return 1.0f;
	
	float ctt = safe_sqrt(1.0f - s2tt);

	float r_parl = fmsub(eta_i,cost_i,ctt) / fmadd(eta_i,cost_i,ctt);
	float r_perp = fnmadd(eta_i,ctt,cost_i) / fmadd(eta_i,ctt,cost_i);

	return ((r_parl * r_parl) + (r_perp * r_perp)) / 2.0f;
};

inline float fr_diel_ext(float cti, float* ctt, float ior)
{
	if(ior == 1)
	{
		*ctt = -cti;
		return 0.0f;
	}

	//use snell's law, calc sqr sine of angle of normal and ray
	float scale = (cti > 0.0f) ? rcp(ior) : ior;
	//float cos_theta_t2 = 1.0f - (1.0f - cti * cti) * (scale * scale);
	float cos_theta_t2 = fnmadd(scale*scale,fnmadd(cti,cti,1.0f),1.0f);

	//check for total internal reflection
	if(cos_theta_t2 <= 0.0f)
	{
		*ctt = 0.0f;
		return 1.0f;
	}

	//find abs cos of incident/transmitted rays
	float cos_theta_i = std::abs(cti);
	float cos_theta_t = std::sqrt(cos_theta_t2);

	float rs = (cos_theta_i - ior * cos_theta_t) / fmadd(ior,cos_theta_t,cos_theta_i);
	float rp = (ior * cos_theta_i - cos_theta_t) / fmadd(ior,cos_theta_i,cos_theta_t);

	*ctt = (cti > 0.0f) ? -cos_theta_t : cos_theta_t;

	//no polarization, return unpolarized reflectance
	return 0.5f * (rs * rs + rp * rp);
};