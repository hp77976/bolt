#pragma once
#include "../../base/material.h"
#include "../../base/sampler.h"
#include "../textures/import.h"
#include <algorithm>

//mitsuba 0.6 style glass
class glass : public material
{
	color_socket* reflection_color = nullptr;
	color_socket* transmission_color = nullptr;

	//TODO: might need to break this up into two materials?
	//these all need to be textures as well
	float roughness;
	float dispersion;
	float ior;

	void glass_config()
	{
		if(roughness == 0.0f)
		{
			components.push_back(
				EDeltaReflection  |EFrontSide|EBackSide|ESpatiallyVarying
			);
			components.push_back(
				EDeltaTransmission|EFrontSide|EBackSide|ESpatiallyVarying|ENonSymmetric
			);
		}
		else
		{
			components.push_back(
				EGlossyReflection  |EFrontSide|EBackSide|ESpatiallyVarying|EUsesSampler
			);
			components.push_back(
				EGlossyTransmission|EFrontSide|EBackSide|ESpatiallyVarying|EUsesSampler|ENonSymmetric
			);
		}

		config();
	};

	public:
	glass(
		color_socket* rf_color, color_socket* tx_color, float r, float d, float i
	) : reflection_color(rf_color), transmission_color(tx_color), 
		roughness(r), dispersion(d), ior(i)
	{
		glass_config();
	};

	glass(FILE* f, scene* s)
	{
		reflection_color   = read_color_socket(f,s);
		transmission_color = read_color_socket(f,s);
		fread(&roughness,sizeof(float),1,f);
		fread(&dispersion,sizeof(float),1,f);
		fread(&ior,sizeof(float),1,f);
		glass_config();
	};

	spectrum sample(bsdf_record &rec, sampler* const rng) const
	{
		bool do_reflect  = //(rec.mask & EDeltaReflection) &&
			true;	//(rec.component == -1 || rec.component == 0);
		bool do_transmit = //(rec.mask & EDeltaTransmission) && 
			true;	//(rec.component == -1 || rec.component == 1);
			
		float cos_theta_t;
		float f = fr_diel_ext(cos_theta(rec.li),&cos_theta_t,ior);

		if(do_reflect && do_transmit /*true*/)
		{
			if(rng->get_float() <= f)
			{
				rec.sampled_component = 0;
				rec.sampled_type = EDeltaReflection;
				rec.lo = reflect(rec.li);
				rec.pdf = f;
				return reflection_color->eval(rec);
			}
			else
			{
				rec.sampled_component = 1;
				rec.sampled_type = EDeltaTransmission;
				rec.lo = refract(rec.li,cos_theta_t,ior);
				rec.pdf = 1.0f - f;
				float factor = (rec.mode == TM_RADIANCE) ? 
					(cos_theta_t < 0.0f ? (1.0f / ior) : ior) : 1.0f;

				return transmission_color->eval(rec) * (factor * factor);
			}
		}		
	};

	spectrum eval(const bsdf_record &rec, e_measure measure) const 
	{
		bool do_reflect  = measure == E_DISCRETE;
		//(rec.component == -1 || rec.component == 0) && (measure == E_DISCRETE);
		bool do_transmit = measure == E_DISCRETE;
		//(rec.component == -1 || rec.component == 1) && (measure == E_DISCRETE);

		float cos_theta_t = 0.0f;
		float f = fr_diel_ext(cos_theta(rec.li),&cos_theta_t,ior);

		if(cos_theta(rec.li) * cos_theta(rec.lo) >= 0.0f) //reflect
		{
			if(!do_reflect || std::abs(dot(reflect(rec.li),rec.lo)-1.0f) >  1e-3f)
				return 0.0f;

			return reflection_color->eval(rec) * f;
		}
		else //refract
		{
			if(!do_transmit || std::abs(dot(refract(rec.li,cos_theta_t,ior),rec.lo)-1.0f) > 1e-3f)
				return 0.0f;
			
			float factor = cos_theta_t < 0.0f ? 1.0f / ior : ior;

			return transmission_color->eval(rec) * factor * factor * (1.0f - f);
		}
	};

	float pdf(const bsdf_record &rec, const vec2f &rng, e_measure measure) const
	{
		bool do_reflect  = measure == E_DISCRETE;
		//(rec.component == -1 || rec.component == 0) && (measure == E_DISCRETE);
		bool do_transmit = measure == E_DISCRETE;
		//(rec.component == -1 || rec.component == 1) && (measure == E_DISCRETE);

		float cos_theta_t = 0.0f;
		float f = fr_diel_ext(cos_theta(rec.li),&cos_theta_t,ior);

		if(cos_theta(rec.li) * cos_theta(rec.lo) >= 0.0f)
		{
			if(!do_reflect || std::abs(dot(reflect(rec.li),rec.lo)-1.0f) > 1e-3f)
				return 0.0f;

			return do_transmit ? f : 1.0f;
		}
		else
		{
			if(!do_transmit || std::abs(dot(refract(rec.li,cos_theta_t,ior),rec.lo)-1.0f) > 1e-3f)
				return 0.0f;
			
			return do_reflect ? 1.0f - f : 1.0f;
		}
	};

	float get_eta() const
	{
		return ior;
	};
};