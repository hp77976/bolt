#pragma once
#include "../core/spectrum.h"
#include "../core/spectrum.h"
#include "../core/records.h"

enum EEmitterType
{
	/// Emission profile contains a Dirac delta term with respect to direction
	EDeltaDirection = 0x01,

	/// Emission profile contains a Dirac delta term with respect to position
	EDeltaPosition  = 0x02,

	/// Is the emitter associated with a surface in the scene?
	EOnSurface      = 0x04
};

class emitter
{
	protected:
	transform<float> m_world_transform;
	transform<float8> m_world_transform8;
	vec3f pos;
	vec3f8 pos8;
	uint32_t m_type = 0;

	private:
	void init()
	{
		for(int i = 0; i < 8; i++)
		{
			for(int j = 0; j < 3; j++)
				pos8[j][i] = pos[j];

			for(int j = 0; j < 4; j++)
			{
				for(int k = 0; k < 4; k++)
				{
					m_world_transform8.mt.m[j][k][i] = m_world_transform.mt.m[j][k];
					m_world_transform8.mi.m[j][k][i] = m_world_transform.mi.m[j][k];
				}
			}
		}
	};

	public:
	emitter(const transform<float> &w_t) : m_world_transform(w_t)
	{
		pos = transform_point(m_world_transform,vec3f(0.0f));
		init();
	};

	emitter(FILE* f)
	{
		float t[9];
		fread(t,sizeof(float),9,f);
		m_world_transform = look_at<float>({t[0],t[1],t[2]},{t[3],t[4],t[5]},{t[6],t[7],t[8]});
		pos = transform_point(m_world_transform,vec3f(0.0f));
		init();
	};

	uint32_t type() const {return m_type;};

	virtual spectrum sample_direct(pd_record* const rec, const vec2f &rng) const = 0;

	virtual spectrum8a sample_direct8(pd_record8* const rec, const vec2f8 &rng) const = 0;

	virtual spectrum sample_direction(
		direction_record* d_rec, pd_record* p_rec,
		const vec2f &sample, const vec2f *extra
	) const = 0;

	virtual spectrum8a sample_direction8(
		direction_record8* d_rec, pd_record8* p_rec,
		const vec2f8 &sample, const vec2f8 *extra
	) const = 0;

	virtual spectrum sample_position(
		pd_record* p_rec, const vec2f &sample, const vec2f* extra
	) const = 0;

	virtual spectrum8a sample_position8(
		pd_record8* p_rec, const vec2f8 &sample, const vec2f8* extra
	) const = 0;

	virtual float pdf_direct(const pd_record &d_rec) const = 0;

	virtual float pdf_direction(const direction_record &d_rec, const pd_record &p_rec) const = 0;

	virtual float pdf_position(const pd_record &d_rec) const = 0;

	virtual float pdf() const = 0;

	virtual spectrum eval_direction(const direction_record &d_rec, const pd_record &p_rec) const = 0;

	virtual spectrum eval_position(const pd_record &p_rec) const = 0;

	inline vec3f get_position() const {return pos;};
	
	inline bool is_on_surface() const {return (m_type & EOnSurface);};

	inline e_measure get_direct_measure() const
	{
		return needs_direct_sample() ? E_SOLID_ANGLE : E_DISCRETE;
	};

	inline bool needs_direct_sample() const
	{
		return needs_position_sample() && needs_direction_sample();
	};

	inline bool needs_position_sample() const {return !(m_type & EDeltaPosition);};

	inline bool needs_direction_sample() const {return !(m_type & EDeltaDirection);};
};