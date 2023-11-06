#pragma once
#include "emitter.h"
#include "../core/film.h"

struct box2d
{
	vec2f min =  INF;
	vec2f max = -INF;

	inline void reset()
	{
		min =  INF;
		max = -INF;
	};
};

template <typename T>
inline T contains(const box2d &b, const vec2<T> &p)
{

};

template <>
inline float contains(const box2d &b, const vec2<float> &p)
{
	for(int i = 0; i < 2; i++)
		if(p[i] < b.min[i] || p[i] > b.max[i])
			return false;
	
	return true;
};

template <>
inline float4 contains(const box2d &b, const vec2<float4> &p)
{
	float4 ret = float4(0.0f);
	for(int i = 0; i < 2; i++)
		ret = select(p[i] < float4(b.min[i]) || p[i] > float4(b.max[i]),ret,float4(1.0f));

	return ret;
};

template <>
inline float8 contains(const box2d &b, const vec2<float8> &p)
{
	float8 ret = float8(0.0f);
	for(int i = 0; i < 2; i++)
		ret = select(p[i] < float8(b.min[i]) || p[i] > float8(b.max[i]),ret,float8(1.0f));

	return ret;
};

inline void expand_by(box2d* const b, const vec2f &p)
{
	for(int i = 0; i < 2; i++)
	{
		b->min[i] = std::min(b->min[i],p[i]);
		b->max[i] = std::max(b->max[i],p[i]);
	}
};

inline float get_volume(const box2d &b)
{
	vec2f diff = b.max - b.min;
	float res = diff[0];
	
	for(int i = 1; i < 2; i++)
		res *= diff[i];
	
	return res;
};

class camera : public emitter
{
	protected:
	film* m_film = nullptr;

	float m_near_clip  = 0.0f;
	float m_far_clip   = 0.0f;
	float m_focus_dist = 0.0f;
	float m_aspect     = 0.0f;
	
	vec2f m_res;
	vec2f m_inv_res;
	box2d img_rect;

	public:
	inline camera(
		float near, float far, float dist, const vec2f &res,
		float aspect, const transform<float> &t, film* f
	) : emitter(t), m_film(f), m_near_clip(near), m_far_clip(far), m_focus_dist(dist),
		m_aspect(aspect), m_res(res), m_inv_res(1.0f/res) {};

	inline camera(FILE* f, film* _film) : emitter(f)
	{
		m_film = _film;
		fread(&m_near_clip,sizeof(m_near_clip),1,f);
		fread(&m_far_clip,sizeof(m_far_clip),1,f);
		fread(&m_focus_dist,sizeof(m_focus_dist),1,f);
		fread(&m_aspect,sizeof(m_aspect),1,f);
		int r0;
		fread(&r0,sizeof(r0),1,f);
		m_res.x = r0;
		fread(&r0,sizeof(r0),1,f);
		m_res.y = r0;
		m_inv_res = 1.0f / m_res;
	};

	virtual spectrum sample_ray(ray &r, const vec2f &px, const vec2f &s) const = 0;

	virtual spectrum8a sample_ray8(ray8 &r, const vec2f8 &px, const vec2f &s) const = 0;

	virtual bool get_sample_position(
		const pd_record &p_rec, const direction_record &d_rec, vec2f* px_pos
	) const = 0;

	virtual void get_pdf(
		const ray &r, float dist, float x, float y,
		float* const pdf, float* const flux
	) const = 0;
};