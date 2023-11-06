#pragma once
#include "../math/include.h"
#include "differentials.h"
//#include "../records.h"
#include <stdexcept>

/*class primitive
{
	protected:
	transform obj_to_world;
	float surf_area;

	public:
	primitive(const transform &t) : obj_to_world(t) {};

	virtual bool intersect(const ray &r, float* const t) const
	{
		throw std::runtime_error("Called: primitive.intersect()!\n");
		return false;
	};

	virtual void sample_pos(hit_rec* const rec, const vec2f &rng) const
	{
		throw std::runtime_error("Called: primitive.sample_pos()!\n");
	};

	virtual float pdf_position() const
	{
		throw std::runtime_error("Called: primitive.pdf_position()!\n");
		return 0.0f;
	};

	float surface_area() const {return surf_area;};

	protected:
	virtual void set_surface_area()
	{

	};
};

class rectangle : public primitive
{
	protected:
	transform world_to_obj;
	vec3f dpdu, dpdv;
	frame m_frame;

	public:
	rectangle(const transform &t) : primitive(t)
	{
		world_to_obj = inverse(obj_to_world);

		dpdu = transform_vector(obj_to_world,vec3f(2,0,0));
		dpdv = transform_vector(obj_to_world,vec3f(0,2,0));

		vec3f normal = normalize(transform_normal(obj_to_world,vec3f(0,0,1)));

		m_frame = frame(normalize(dpdu),normalize(dpdv),normal);

		set_surface_area();
	};

	bool intersect(const ray &r, float* const t) const
	{
		ray r_ = transform_ray_affine(world_to_obj,r);
		float hit = -r_.o.z / r_.d.z;

		if(!(hit >= r_.mint && hit <= r_.maxt))
			return false;

		vec3f local = r_.o + r_.d * hit;

		if(std::abs(local.x) <= 1 && std::abs(local.y) <= 1)
		{
			*t = hit;
			return true;
		}

		return false;
	};

	void sample_pos(hit_rec* const rec, const vec2f &rng) const
	{
		rec->pos = transform_point(obj_to_world,vec3f(rng.x * 2 - 1, rng.y * 2 - 1, 0));
		rec->norm = m_frame.n;
		rec->pdf = 1.0f / surf_area;
		//rec->uv = sample;
	};

	float pdf_position() const
	{
		return 1.0f / surf_area;
	};

	protected:
	void set_surface_area()
	{
		surf_area = len(dpdu) * len(dpdv);
	};
};*/