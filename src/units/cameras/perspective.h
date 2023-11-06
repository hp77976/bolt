#pragma once
#include "../../base/camera.h"

class perspective_camera : public camera
{
	protected:
	float m_xfov;
	transform<float> m_camera_to_sample;
	transform<float> m_sample_to_camera;
	transform<float> m_clip_transform;

	transform<float8> m_camera_to_sample8;
	transform<float8> m_sample_to_camera8;
	transform<float8> m_clip_transform8;

	//image_rect
	float m_normalization;
	vec3f m_dx, m_dy;

	public:
	perspective_camera(
		float near, float far, float dist, const vec2f &res,
		float aspect, const transform<float> &t, film* f, float fov
	) : camera(near,far,dist,res,aspect,t,f)
	{
		init(
			m_near_clip,m_far_clip,m_focus_dist,m_res,
			m_aspect,m_world_transform,f,m_xfov
		);
	};

	perspective_camera(FILE* f, film* _film) : camera(f,_film)
	{
		fread(&m_xfov,sizeof(m_xfov),1,f);
		printf("Aspect: %f\n",m_aspect);
		init(
			m_near_clip,m_far_clip,m_focus_dist,m_res,
			m_aspect,m_world_transform,_film,m_xfov
		);
	};

	private:
	void init(
		float near, float far, float dist, const vec2f &res,
		float aspect, const transform<float> &t, film* f, float fov
	)
	{
		{
			m_type |= EDeltaPosition | EOnSurface;

			set_x_fov(fov);
			m_xfov = fov;

			vec2i &film_size   = m_film->m_size;
			vec2i &crop_size   = m_film->m_crop_size;
			vec2i &crop_offset = m_film->m_crop_offset;

			vec2f rel_size = vec2f(
				(float) crop_size.x / (float) film_size.x,
				(float) crop_size.y / (float) film_size.y
			);

			vec2f rel_offset = vec2f(
				(float) crop_offset.x / (float) film_size.x,
				(float) crop_offset.y / (float) film_size.y
			);

			m_camera_to_sample = 
				scale(vec3f(rcp(rel_size.x),rcp(rel_size.y), 1.0f)) * 
				translate(vec3f(-rel_offset.x, -rel_offset.y, 0.0f)) *
				scale(vec3f(-0.5f, -0.5f*m_aspect, 1.0f)) * 
				translate(vec3f(-1.0f, -1.0f/m_aspect, 0.0f)) *
				perspective(fov,near,far);
			
			m_sample_to_camera = inverse(m_camera_to_sample);

			m_dx = transform_point(m_sample_to_camera,vec3f(m_inv_res.x,0.0f,0.0f));
			m_dx -= transform_point(m_sample_to_camera,vec3f(0.0f));

			m_dy = transform_point(m_sample_to_camera,vec3f(0.0f,m_inv_res.y,0.0f));
			m_dy -= transform_point(m_sample_to_camera,vec3f(0.0f));

			vec3f min_ = transform_point(m_sample_to_camera,vec3f(0.0f,0.0f,0.0f));
			vec3f max_ = transform_point(m_sample_to_camera,vec3f(1.0f,1.0f,0.0f));

			img_rect.reset();
			expand_by(&img_rect,vec2f(min_.x,min_.y)/min_.z);
			expand_by(&img_rect,vec2f(max_.x,max_.y)/max_.z);
			m_normalization = rcp(get_volume(img_rect));
		}

		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				m_camera_to_sample8.mt.m[i][j] = float8(m_camera_to_sample.mt.m[i][j]);
				m_camera_to_sample8.mi.m[i][j] = float8(m_camera_to_sample.mi.m[i][j]);

				m_sample_to_camera8.mt.m[i][j] = float8(m_sample_to_camera.mt.m[i][j]);
				m_sample_to_camera8.mi.m[i][j] = float8(m_sample_to_camera.mi.m[i][j]);

				m_clip_transform8.mt.m[i][j] = float8(m_clip_transform.mt.m[i][j]);
				m_clip_transform8.mi.m[i][j] = float8(m_clip_transform.mi.m[i][j]);
			}
		}

		//skip image rect for now?
		//skip clip transform. no need for ogl support yet
	};

	template <typename T>
	inline T importance(const vec3<T> &d) const
	{
		T ct = cos_theta(d);

		T inv_ct = T(1.0f) / ct;
		vec2<T> p = vec2<T>(d.x * inv_ct, d.y * inv_ct);

		T ret = T(m_normalization) * inv_ct * inv_ct * inv_ct;
		ret = select(ct<=T(0.0f)||!contains(img_rect,p),T(0.0f),ret);
		return ret;
	};
	
	public:
	spectrum sample_ray(ray &r, const vec2f &px, const vec2f &s) const
	{
		vec3f np = transform_point(m_sample_to_camera,vec3f(px.x*m_inv_res.x,px.y*m_inv_res.y,0.0f));
		vec3f d = normalize(np);
		float iz = rcp(d.z);
		r.o = get_position();
		r.d = transform_vector(m_world_transform,d);
		return 1.0f;
	};

	spectrum8a sample_ray8(ray8 &r, const vec2f8 &px, const vec2f &s) const
	{
		vec3f8 np = transform_point(m_sample_to_camera8,vec3f8(px.x*m_inv_res.x,px.y*m_inv_res.y,0.0f));
		vec3f8 d = normalize(np);
		float8 iz = rcp(d.z);
		r.o = transform_point_affine(m_world_transform8,vec3f8(0.0f));
		r.d = transform_vector(m_world_transform8,d);
		return 1.0f;
	};

	spectrum sample_direct(pd_record* d_rec, const vec2f &sample) const
	{
		//localize reference position
		vec3f ref = transform_point_affine(inverse<float>(m_world_transform),d_rec->ref_pos);

		if(ref.z < m_near_clip || ref.z > m_far_clip)
		{
			d_rec->pdf = 0.0f;
			return 0.0f;
		}

		vec3f screen_sample = transform_point(m_camera_to_sample,ref);
		d_rec->uv = vec2f(screen_sample.x,screen_sample.y);
		if(
			d_rec->uv.x < 0 || d_rec->uv.x > 1 ||
			d_rec->uv.y < 0 || d_rec->uv.y > 1
		)
		{
			d_rec->pdf = 0.0f;
			return 0.0f;
		}

		d_rec->uv.x *= m_res.x;
		d_rec->uv.y *= m_res.y;

		vec3f local_dir = ref;
		float dist = len(local_dir);
		float inv_dist = 1.0f / dist;
		local_dir *= inv_dist;

		d_rec->pos = transform_point_affine<float>(m_world_transform,vec3f(0.0f));
		d_rec->dir = (d_rec->pos - d_rec->ref_pos) * inv_dist;
		d_rec->dist = dist;
		d_rec->norm = transform_point<float>(m_world_transform,vec3f(0.0f,0.0f,1.0f));
		d_rec->pdf = 1.0f;
		d_rec->measure = E_DISCRETE;

		return spectrum(importance(local_dir) * inv_dist * inv_dist);
	};

	spectrum8a sample_direct8(pd_record8* d_rec, const vec2f8 &sample) const
	{
		vec3f8 ref = transform_point_affine(inverse<float8>(m_world_transform8),d_rec->ref_pos);

		float8 c0 = (ref.z < float8(m_near_clip) || ref.z > float8(m_far_clip));
		float8 c1 = d_rec->uv.x < float8(0.0f) || d_rec->uv.x > float8(1.0f) ||
					d_rec->uv.y < float8(0.0f) || d_rec->uv.y > float8(1.0f);

		vec3f8 screen_sample = transform_point(m_camera_to_sample8,ref);
		d_rec->uv = vec2f8(screen_sample.x,screen_sample.y);
		d_rec->uv *= vec2f8(float8(m_res.x),float8(m_res.y));
		vec3f8 local_dir = ref;
		float8 dist = len(local_dir);
		float8 inv_dist = rcp(dist);
		local_dir *= inv_dist;

		vec3 temp_pos = transform_point_affine(m_world_transform,vec3f(0.0f));
		vec3 temp_norm = transform_point(m_world_transform,vec3f(0.0f,0.0f,1.0f));
		d_rec->pos = vec3f8(float8(temp_pos.x),float8(temp_pos.y),float8(temp_pos.z));
		d_rec->dir = (d_rec->pos - d_rec->ref_pos) * inv_dist;
		d_rec->dist = dist;
		d_rec->norm = vec3f8(float8(temp_norm.x),float8(temp_norm.y),float8(temp_norm.z));
		d_rec->pdf = float8(1.0f);
		for(int i = 0; i < 8; i++)
			d_rec->measure[i] = E_DISCRETE;
		d_rec->pdf = select(c0||c1,float8(0.0f),d_rec->pdf);
		return spectrum8a(select(c0||c1,float8(0.0f),importance(local_dir) * inv_dist * inv_dist));
	};

	spectrum sample_direction(
		direction_record* d_rec, pd_record* p_rec,
		const vec2f &sample, const vec2f *extra
	) const;

	spectrum8a sample_direction8(
		direction_record8* d_rec, pd_record8* p_rec,
		const vec2f8 &sample, const vec2f8 *extra
	) const;

	spectrum sample_position(
		pd_record* p_rec, const vec2f &sample, const vec2f* extra
	) const;

	spectrum8a sample_position8(
		pd_record8* p_rec, const vec2f8 &sample, const vec2f8* extra
	) const;

	float pdf_position(const pd_record &p_rec) const
	{
		return (p_rec.measure == E_DISCRETE) ? 1.0f : 0.0f;
	};

	float pdf_direct(const pd_record &d_rec) const
	{
		return (d_rec.measure == E_DISCRETE) ? 1.0f : 0.0f;
	};

	float pdf_direction(const direction_record &d_rec, const pd_record &p_rec) const
	{
		if(d_rec.measure != E_SOLID_ANGLE)
			return 0.0f;

		return importance(transform_vector<float>(inverse<float>(m_world_transform),d_rec.dir));
	};

	spectrum eval_direction(const direction_record& d_rec, const pd_record &p_rec) const
	{
		if(d_rec.measure != E_SOLID_ANGLE)
			return 0.0f;

		return importance(transform_vector(inverse<float>(m_world_transform),d_rec.dir));
	};

	spectrum eval_position(const pd_record &p_rec) const
	{
		return (p_rec.measure == E_DISCRETE) ? 1.0f : 0.0f;
	};

	bool get_sample_position(
		const pd_record &p_rec, const direction_record &d_rec, vec2f* px_pos
	) const
	{
		vec3f local = transform_vector(inverse<float>(m_world_transform),d_rec.dir);

		if(local.z <= 0)
			return false;

		vec3f screen_sample = transform_point(m_camera_to_sample,local);

		if(
			screen_sample.x < 0 || screen_sample.x > 1 ||
			screen_sample.y < 0 || screen_sample.y > 1
		)
			return false;
		
		*px_pos = vec2f(screen_sample.x * m_res.x, screen_sample.y * m_res.y);

		return true;
	};

	void get_pdf(
		const ray &r, float dist, float x, float y,	float* const pdf, float* const flux
	) const
	{
		//localize reference position
		vec3f ref = transform_point_affine(inverse<float>(m_world_transform),r.o);
		vec3f local_dir = ref;
		float dist_ = len(local_dir);
		float inv_dist = 1.0f / dist_;
		local_dir *= inv_dist;

		vec3f dir = local_dir;

		float cos_ac = m_normalization;
		if(cos_ac <= 0.0f)
		{
			*flux = 0.0f;
		}
		else
		{
			float cam_pdf = 1.0f / (cos_ac * cos_ac * cos_ac * (m_inv_res.x * m_inv_res.y));

			*flux = cam_pdf / (dist * dist);
		}
	};

	void set_focal_length(float l);

	void set_diagonal_fov(float d);

	void set_x_fov(float x);

	void set_y_fov(float y);

	float pdf() const {return 1.0f;};
};