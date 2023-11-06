#pragma once
#include "../../base/light.h"

class point_light : public light
{
	public:
	point_light(const spectrum &s, float g, const transform<float> &t) : light(s,g,t)
	{
		m_type |= EDeltaPosition;
	};

	point_light(FILE* f) : light(f)
	{
		//TODO: does this need size/radius?
		m_type |= EDeltaPosition;
	};

	spectrum sample_direct(pd_record* const rec, const vec2f &rng) const
	{
		rec->pos = get_position();
		rec->pdf = 1.0f;
		rec->measure = E_DISCRETE;
		rec->uv = 0.5f;
		rec->dir = rec->pos - rec->ref_pos;
		rec->dist = len(rec->dir);
		float inv_dist = 1.0f / rec->dist;
		rec->dir *= inv_dist;
		rec->norm = 0.0f;

		return power * (inv_dist * inv_dist);
	};

	spectrum8a sample_direct8(pd_record8* d_rec, const vec2f8 &sample) const
	{
		vec3f p = get_position();
		d_rec->pos = vec3f8(float8(p.x),float8(p.y),float8(p.z));
		d_rec->pdf = float8(1.0f);
		for(int i = 0; i < 8; i++)
			d_rec->measure[i] = E_DISCRETE;
		d_rec->uv = vec2f8(float8(0.5f));
		d_rec->dir = d_rec->pos - d_rec->ref_pos;
		d_rec->dist = len(d_rec->dir);
		float8 inv_dist = rcp(d_rec->dist);
		d_rec->dir *= inv_dist;
		d_rec->norm = float8(0.0f);
		return spectrum8a(power) * (inv_dist * inv_dist);
	};

	spectrum sample_direction(
		direction_record* d_rec, pd_record* p_rec, const vec2f &sample, const vec2f *extra
	) const
	{
		d_rec->dir = square_to_uni_sphere(sample);
		d_rec->pdf = M_4_PI;
		d_rec->measure = E_SOLID_ANGLE;
		return 1.0f;
	};

	spectrum8a sample_direction8(
		direction_record8* d_rec, pd_record8* p_rec, const vec2f8 &sample, const vec2f8 *extra
	) const
	{
		d_rec->dir = square_to_uni_sphere(sample);
		d_rec->pdf = float8(M_4_PI);
		for(int i = 0; i < 8; i++)
			d_rec->measure[i] = E_SOLID_ANGLE;
		return spectrum8a(1.0f);
	};

	spectrum sample_position(
		pd_record* p_rec, const vec2f &sample, const vec2f* extra
	) const
	{
		p_rec->pos = get_position();
		p_rec->norm = 0.0f;
		p_rec->pdf = 1.0f;
		p_rec->measure = E_DISCRETE;
		return power * (4 * M_PI);
	};

	spectrum8a sample_position8(
		pd_record8* p_rec, const vec2f8 &sample, const vec2f8* extra
	) const
	{
		vec3f p = get_position();
		p_rec->pos = vec3f8(float8(p.x),float8(p.y),float8(p.z));
		p_rec->norm = float8(0.0f);
		p_rec->pdf = float8(1.0f);
		for(int i = 0; i < 8; i++)
			p_rec->measure[i] = E_DISCRETE;
		return spectrum8a(power * (4.0f * M_PI));
	};

	float pdf() const
	{
		return 1.0f;
	};

	float pdf_direct(const pd_record &d_rec) const
	{
		return (d_rec.measure == E_DISCRETE) ? 1.0f : 0.0f;
	};	

	float pdf_direction(const direction_record &d_rec, const pd_record &p_rec) const
	{
		return (d_rec.measure == E_SOLID_ANGLE) ? M_4_PI : 0.0f;
	};

	float pdf_position(const pd_record &p_rec) const
	{
		return (p_rec.measure == E_DISCRETE) ? 1.0f : 0.0f;
	};

	spectrum eval_direction(const direction_record &d_rec, const pd_record &p_rec) const
	{
		return (d_rec.measure == E_SOLID_ANGLE) ? M_4_PI : 0.0f;
	};

	spectrum eval_position(const pd_record &p_rec) const
	{
		return (p_rec.measure == E_DISCRETE) ? (power * 4 * M_PI) : 0.0f;
	};
};