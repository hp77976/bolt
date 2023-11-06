#pragma once
#include "../../base/material.h"
#include "../../base/sampler.h"
#include "../textures/import.h"

//mitsuba 0.6 style diffuse
class diffuse : public material
{
	private:
	color_socket* color = nullptr;
	float sigma;
	float a;
	float b;

	void diffuse_config()
	{
		float sigma2 = this->sigma * this->sigma;

		this->a = 1.0f - (sigma2 / 2.0f * (sigma2 + 0.33f));
		this->b = 0.45f * sigma2 / (sigma2 + 0.09f);

		components.push_back(EDiffuseReflection|EFrontSide|ESpatiallyVarying);

		config();
	};

	public:
	diffuse(color_socket* color_, const float sigma_) : color(color_), sigma(sigma_)
	{
		diffuse_config();
	};

	diffuse(FILE* f, scene* s)
	{
		color = read_color_socket(f,s);
		fread(&sigma,sizeof(float),1,f);
		diffuse_config();
	};

	//for easier python binding
	diffuse(const vec3f &rgb, float sigma_) : sigma(sigma_)
	{
		color = new const_color(from_linear_rgb(rgb,true));
		diffuse_config();
	};

	spectrum sample(bsdf_record &b_rec, sampler* const s) const
	{
		if(cos_theta(b_rec.li) <= 0.0f)
			return 0.0f;

		b_rec.sampled_component = 0;
		b_rec.sampled_type = EDiffuseReflection;

		b_rec.lo = square_to_cos_hemi(s->get_vec2f());
		b_rec.pdf = square_to_cos_hemi_pdf(b_rec.lo);

		return color->eval(b_rec);
	};
	
	spectrum eval(const bsdf_record &b_rec, e_measure measure) const 
	{
		if(measure != E_SOLID_ANGLE)
			return 0.0f;

		if(cos_theta(b_rec.li) <= 0.0f || cos_theta(b_rec.lo) <= 0.0f)
			return 0.0f;

		return color->eval(b_rec) * (M_1_PI * cos_theta(b_rec.lo));
	};

	float pdf(const bsdf_record &b_rec, const vec2f &rng, e_measure measure) const
	{
		if(measure != E_SOLID_ANGLE)
			return 0.0f;

		if(cos_theta(b_rec.li) <= 0.0f || cos_theta(b_rec.lo) <= 0.0f)
			return 0.0f;

		return square_to_cos_hemi_pdf(b_rec.lo);
	};
};