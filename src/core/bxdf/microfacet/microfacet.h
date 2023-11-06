#pragma once
#include "../../math/include.h"
#include "../../spectrum.h"

class microfacet
{
	vec2f alpha; //roughness
	vec2f exponent;

	public:
	inline microfacet(vec2f alpha_) {this->alpha = alpha_;};

	inline void scale_alpha(float v) {this->alpha *= v;};

	inline vec3f sample(const vec3f &li, const vec2f &sample, float* const pdf_) const
	{
		//stretch li
		vec3f sli = normalize(vec3f(
			this->alpha.u * li.x,
			this->alpha.v * li.y,
			li.z)
		);

		//get polar coords
		float theta = 0.0f, phi = 0.0f;
		if(sli.z < 0.99999f)
		{
			theta = std::acos(sli.z);
			phi = std::atan2(sli.y,sli.x);
		}

		float sin_phi, cos_phi;
		sincosf(phi,&sin_phi,&cos_phi);

		//simulate p22_wi(slope.x,slope.y,1,1)
		vec2f slope = sample_vis11(theta,sample);

		//rotate
		slope = vec2f(
			cos_phi * slope.x - sin_phi * slope.y,
			sin_phi * slope.x + cos_phi * slope.y
		);

		//unstretch
		slope *= this->alpha;

		//compute normal
		float norm = 1.0f / std::sqrt(slope.x*slope.x+slope.y*slope.y+1.0f);

		vec3f ret = vec3f(-slope.x * norm, -slope.y * norm, norm);

		*pdf_ = pdf(li,ret);

		return ret;
	};

	//for now only GGX is supported
	inline vec2f sample_vis11(float theta_i, vec2f sample) const
	{
		vec2f slope;

		//special case for normal incidence
		if(theta_i < 1e-4f)
		{
			float sin_phi, cos_phi;
			float r = safe_sqrt(sample.x/(1.0f - sample.x));
			sincosf(2.0f * float(M_PI) * sample.y,&sin_phi,&cos_phi);
			return vec2f(r*cos_phi,r*sin_phi);
		}

		//precomputations
		float tan_theta_i = std::tan(theta_i);
		float a = 1.0f / tan_theta_i;
		float g1 = 2.0f / (1.0f + safe_sqrt(1.0f + 1.0f / (a*a)));
		
		//simulate x component
		float a_ = 2.0f * sample.x / g1 - 1.0f;
		if(std::abs(a_) == 1.0f)
			a_ -= copysignf(float(1.0f),a_) * 1e-4f;
		float tmp = 1.0f / (a_ * a_ - 1.0f);
		float b = tan_theta_i;
		float d = safe_sqrt(b*b*tmp*tmp - (a_*a_ - b*b) * tmp);
		float slope_x_1 = b * tmp - d;
		float slope_x_2 = b * tmp + d;
		slope.x = (a_ < 0.0f || slope_x_2 > 1.0f / tan_theta_i) ? slope_x_1 : slope_x_2;

		//simulate y component
		float s;
		if(sample.y > 0.5f)
		{
			s = 1.0f;
			sample.y = 2.0f * (sample.y - 0.5f);
		}
		else
		{
			s = -1.0f;
			sample.y = 2.0f * (0.5f - sample.y);
		}

		//improved fit
		float z = 
		(
			sample.y *
			(
				sample.y * 
				(
					sample.y * (-(float) 0.365728915865723) + (float) 0.790235037209296
				) - (float) 0.424965825137544
			) + (float) 0.000152998850436920
		) / (
			sample.y * 
			(
				sample.y *
				(
					sample.y *
					(
						sample.y * (float) 0.169507819808272 - (float) 0.397203533833404
					) - (float) 0.232500544458471
				) + (float) 1
			) - (float) 0.539825872510702
		);
		
		slope.y = s * z * std::sqrt(1.0f + slope.x * slope.x);

		return slope;
	};

	inline float project_roughness(const vec3f &v) const
	{
		float inv_st2 = 1.0f / sin_theta2(v);

		//currently always isotropic
		//TODO: add roughness texture support
		return alpha.u;
	};

	inline float smith_g1(const vec3f &v, const vec3f &m) const
	{
		//ensure consistent orientation (can't see back from front)
		if(dot(v,m) * cos_theta(v) <= 0.0f)
			return 0.0f;
		
		//perpendicular incidence
		float tan_theta_ = std::abs(tan_theta(v));
		if(tan_theta_ == 0.0f)
			return 1.0f;

		float alpha_ = project_roughness(v);

		//GGX is the only mode supported right now
		float root = alpha_ * tan_theta_;
		return 2.0f / (1.0f + hypot2(1.0f,root));
	};

	inline float eval(const vec3f &m) const 
	{
		if(cos_theta(m) <= 0.0f)
			return 0.0f;

		float cos_theta2 = cos_theta(m) * cos_theta(m);
		float beckman_exp = ((m.x*m.x)/(alpha.u * alpha.u) + (m.y*m.y)/(alpha.v*alpha.v)) / cos_theta2;

		float root = (1.0f + beckman_exp) * cos_theta2;
		float result = 1.0f / (M_1_PI * alpha.u * alpha.v * root * root);

		if(result * cos_theta(m) < 1e-20f)
			result = 0.0f;
		
		return result;
	};

	//pdf visible
	inline float pdf(const vec3f &li, const vec3f &m) const
	{
		if(cos_theta(li) == 0.0f)
			return 0.0f;
		
		return smith_g1(li,m) * std::abs(dot(li,m)) * eval(m) / std::abs(cos_theta(li));
	};

	inline float g(const vec3f &li, const vec3f &lo, const vec3f &m) const 
	{
		return smith_g1(li,m) * smith_g1(lo,m);
	};
};