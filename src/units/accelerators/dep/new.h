#pragma once
#include "../../base/accelerator.h"
#include "../../core/records.h"

#include <malloc.h>
#include <xmmintrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <numeric>
#include <memory>

#include "../../../ext/vk-bootstrap/src/VkBootstrap.h"
#include <ios>
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string>
#include <string.h>
#include <cstring>

class scene;

void allocate_bind_buffer(
	VkDevice &dev, VkPhysicalDevice &pdev, VkBuffer &buff, VkDeviceMemory &mem,
	uint32_t mem_size, VkBufferUsageFlags use_flags, VkMemoryPropertyFlags mem_flags
);

int do_the_entire_thing_for_one_intersection(vec3f &a, vec3f &b);

class vk_accelerator2 : public accelerator
{
	public:
	vk_accelerator2(scene* s) : accelerator(s) {};

	~vk_accelerator2() {};

	bool intersect(ray &r, hit_record &h_rec) const
	{
		vec3f ubo = r.o;
		vec3f ubo2 = r.d;

		printf("r.o: %2.2f, %2.2f, %2.2f\n",r.o.x,r.o.y,r.o.z);
		printf("r.d: %2.2f, %2.2f, %2.2f\n",r.d.x,r.d.y,r.d.z);

		do_the_entire_thing_for_one_intersection(ubo,ubo2);

		printf("r.o: %2.2f, %2.2f, %2.2f\n",r.o.x,r.o.y,r.o.z);
		printf("r.d: %2.2f, %2.2f, %2.2f\n",r.d.x,r.d.y,r.d.z);

		throw std::runtime_error("Stop here.\n");

		if(
			ubo.x != -1.0f && ubo.y != -1.0f && ubo.z != -1.0f &&
			ubo2.x != -1.0f && ubo2.y != -1.0f && ubo2.z != -1.0f
		)
		{
			tri* t = &m_tris->at(ubo.x);
			vec3f s1 = t->b.p - t->a.p;
			h_rec.m_shape.set_material(m_mats->at(t->m));
			h_rec.tp = t;
			h_rec.wi = r.d;
			h_rec.pos = ubo.y * r.d + r.o;
			h_rec.dist = ubo.y;
			vec2f uv = vec2f(ubo2.x,ubo2.y);

			float b1 = uv.x;
			float b2 = uv.y;
			float b0 = 1.0f - b1 - b2;
			h_rec.bary = vec3f(b0,b1,b2);
			
			h_rec.sh_frame.n = normalize(t->a.n*b0+t->b.n*b1+t->c.n*b2);
			calc_shading_frame<float>(h_rec.sh_frame.n,s1,h_rec.sh_frame);

			return true;
		}
		else
		{
			return false;
		}
	};

	float8 intersect8(ray8 &r, hit_record8 &h_rec, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};

	bool occluded(const ray &r, const vec3f &pos) const
	{
		vec3f ubo = r.o;
		vec3f ubo2 = r.d;

		do_the_entire_thing_for_one_intersection(ubo,ubo2);

		if(
			ubo.x == -1.0f && ubo.y == -1.0f && ubo.z == -1.0f &&
			ubo2.x == -1.0f && ubo2.y == -1.0f && ubo2.z == -1.0f
		)
		{
			return false;
		}

		if(ubo.y > r.mint)
		{
			return false;
		}

		return true;
	};

	//returns true on hits, false on misses
	float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const
	{
		throw std::runtime_error("No implemented yet!\n");
		return 0.0f;
	};
};