#pragma once
#include "emitter.h"

class light : public emitter
{
	protected:
	spectrum power; //TODO: this needs to be a color socket
	float em_pdf; //TODO: precalculated during load time?

	light(const spectrum &s, float gain_, const transform<float> &t) : emitter(t) {this->power = s * gain_;};

	light(FILE* f) : emitter(f)
	{
		float rgb[3];
		fread(&rgb,sizeof(float),3,f);
		
		float power_ = 0.0f;
		fread(&power_,sizeof(float),1,f);
		float efficiency = 0.0f;
		fread(&efficiency,sizeof(efficiency),1,f);
		
		power = rgb_to_spectrum(vec3f(rgb[0],rgb[1],rgb[2]),true) * power_ * efficiency;
	};

	public:
	matrix4x4<float> get_matrix() const {return m_world_transform.mt;};
};