#include "perspective.h"

spectrum perspective_camera::sample_position(
	pd_record* const p_rec, const vec2f &sample, const vec2f* extra
) const
{
	p_rec->pos = get_position();
	p_rec->norm = transform_point(m_world_transform,vec3f(0.0f,0.0f,1.0f));
	p_rec->pdf = 1.0f;
	p_rec->measure = E_DISCRETE;
	
	return 1.0f;
};

spectrum8a perspective_camera::sample_position8(
	pd_record8* const p_rec, const vec2f8 &sample, const vec2f8* extra
) const
{
	p_rec->pos = pos8;
	vec3f n = transform_point(m_world_transform,vec3f(0.0f,0.0f,1.0f));
	p_rec->norm = vec3f8(float8(n.x),float8(n.y),float8(n.z));
	p_rec->pdf = float8(1.0f);
	for(int i = 0; i < 8; i++)
		p_rec->measure[i] = E_DISCRETE;
	
	return 1.0f;
};

spectrum perspective_camera::sample_direction(
	direction_record* d_rec, pd_record* p_rec, const vec2f &sample, const vec2f* extra
) const
{
	vec3f sample_pos_ = vec3f(sample.x,sample.y,0.0f);

	if(extra)
	{
		sample_pos_.x = (extra->x + sample.x) * m_inv_res.x;
		sample_pos_.y = (extra->y + sample.y) * m_inv_res.y;
	}

	p_rec->uv = vec2f(sample_pos_.x * m_res.x, sample_pos_.y * m_res.y);

	vec3f near_p = transform_point(m_sample_to_camera,sample_pos_);

	vec3f d = normalize(near_p);
	d_rec->dir = transform_vector(m_world_transform,d);
	d_rec->measure = E_SOLID_ANGLE;
	d_rec->pdf = m_normalization / (d.z * d.z * d.z);

	return 1.0f;
};

spectrum8a perspective_camera::sample_direction8(
	direction_record8* d_rec, pd_record8* p_rec,
	const vec2f8 &sample, const vec2f8 *extra
) const
{
	vec3f8 sample_pos_ = vec3f8(sample.x,sample.y,float8(0.0f));

	if(extra)
	{
		sample_pos_.x = (extra->x + sample.x) * float8(m_inv_res.x);
		sample_pos_.y = (extra->y + sample.y) * float8(m_inv_res.y);
	}

	p_rec->uv = vec2f8(sample_pos_.x * float8(m_res.x), sample_pos_.y * float8(m_res.y));

	vec3f8 near_p = transform_point(m_sample_to_camera8,sample_pos_);

	vec3f8 d = normalize(near_p);
	d_rec->dir = transform_vector(m_world_transform8,d);
	for(int i = 0; i < 8; i++)
		d_rec->measure[i] = E_SOLID_ANGLE;
	d_rec->pdf = float8(m_normalization) / (d.z * d.z * d.z);

	return 1.0f;
};

void perspective_camera::set_focal_length(float l)
{
	set_diagonal_fov(2.0f*180.0f/M_PI*std::atan(std::sqrt(36.0f*36.0f+24.0f*24.0f)/(2.0f*l)));
};

void perspective_camera::set_diagonal_fov(float d)
{
	float diag = 2.0f * std::tan(0.5f * deg_to_rad(d));
	float width = diag / std::sqrt(1.0f + 1.0f / (m_aspect*m_aspect));
	set_x_fov(rad_to_deg(2.0f * std::atan(width*0.5f)));
};

void perspective_camera::set_x_fov(float x) {m_xfov = x;};

void perspective_camera::set_y_fov(float y)
{
	set_x_fov(rad_to_deg(2.0f*std::atan(std::tan(0.5f*deg_to_rad(y))*m_aspect)));
};