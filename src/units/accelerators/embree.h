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
#include "../../../ext/embree/include/embree4/rtcore.h"

inline void errorFunction(void* userPtr, enum RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
};

class scene;

class embree_accelerator : public accelerator
{
	RTCScene rt_scene;
	RTCDevice m_dev;

	//std::string isa = "";

	public:
	embree_accelerator(scene* s) : accelerator(s) //, isa(isa_str)
	{
		printf("Selected Embree Accelerator\n");

		m_dev = rtcNewDevice("isa=avx2"
			//"set_affinity=1"
		);

		if(!m_dev)
			throw std::runtime_error("a\n");

		rtcSetDeviceErrorFunction(m_dev, errorFunction, NULL);
		rt_scene = rtcNewScene(m_dev);
		RTCGeometry geo = rtcNewGeometry(m_dev,RTC_GEOMETRY_TYPE_TRIANGLE);
		rtcSetSceneBuildQuality(rt_scene,RTC_BUILD_QUALITY_HIGH);

		uint64_t tri_count = m_tris->size();

		printf("tri count: %lu\n",tri_count);
		printf("vert count: %lu\n",tri_count*3);
		printf("index count: %lu\n",tri_count*3);

		float* verts = (float*)rtcSetNewGeometryBuffer(
			geo,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,3*sizeof(float),tri_count*3
		);

		unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(
			geo,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,3*sizeof(unsigned),tri_count
		);

		if(!verts)
		{
			throw std::runtime_error("failed to alloc verts\n");
		}

		if(!indices)
		{
			throw std::runtime_error("failed to alloc indices\n");
		}

		printf("allocated embree\n");

		uint32_t x = 0;
		uint32_t n = 0;
		for(uint32_t i = 0; i < m_tris->size(); i++)
		{
			verts[x++] = m_tris->at(i).a.p.x;
			verts[x++] = m_tris->at(i).a.p.y;
			verts[x++] = m_tris->at(i).a.p.z;
			verts[x++] = m_tris->at(i).b.p.x;
			verts[x++] = m_tris->at(i).b.p.y;
			verts[x++] = m_tris->at(i).b.p.z;
			verts[x++] = m_tris->at(i).c.p.x;
			verts[x++] = m_tris->at(i).c.p.y;
			verts[x++] = m_tris->at(i).c.p.z;
			indices[n] = n; n++;
			indices[n] = n; n++;
			indices[n] = n; n++;
		}

		if(x != tri_count * 9)
		{
			printf("x: %u\n",x);
			printf("tc3: %lu\n",tri_count*3);
			throw std::runtime_error("\n");
		}

		printf("copied data in\n");

		rtcCommitGeometry(geo);
		rtcAttachGeometry(rt_scene,geo);
		rtcReleaseGeometry(geo);
		rtcCommitScene(rt_scene);

		printf("Built Embree Accelerator\n");
	};

	~embree_accelerator()
	{
		rtcReleaseScene(rt_scene);
		rtcReleaseDevice(m_dev);
	};

	bool intersect(ray &r, hit_record &h_rec) const
	{
		struct RTCRayHit hit;
		hit.ray.org_x = r.o.x;
		hit.ray.org_y = r.o.y;
		hit.ray.org_z = r.o.z;
		hit.ray.dir_x = r.d.x;
		hit.ray.dir_y = r.d.y;
		hit.ray.dir_z = r.d.z;
		hit.ray.tnear = r.mint;
		hit.ray.tfar  = r.maxt;
		hit.ray.mask  = -1;
		hit.ray.flags = 0;
		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

		rtcIntersect1(rt_scene,&hit);

		if(hit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
		{
			tri* t = &m_tris->at(hit.hit.primID);
			vec3f s1 = t->b.p - t->a.p;
			h_rec.m_shape.set_material(m_mats->at(t->m));
			h_rec.tp = t;
			h_rec.wi = r.d;
			h_rec.pos = hit.ray.tfar * r.d + r.o;
			h_rec.dist = hit.ray.tfar;
			vec2f uv = vec2f(hit.hit.u,hit.hit.v);

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
		RTCIntersectArguments iargs;
		rtcInitIntersectArguments(&iargs);
		iargs.feature_mask = RTC_FEATURE_FLAG_TRIANGLE;
		iargs.flags = RTC_RAY_QUERY_FLAG_COHERENT;

		RTCRayHit8 hit;
		int32_8 zero = int32_8(0);
		int32_8 igi = int32_8(RTC_INVALID_GEOMETRY_ID);
		_mm256_store_ps(hit.ray.dir_x,r.d.x);
		_mm256_store_ps(hit.ray.dir_y,r.d.y);
		_mm256_store_ps(hit.ray.dir_z,r.d.z);
		_mm256_store_ps(hit.ray.org_x,r.o.x);
		_mm256_store_ps(hit.ray.org_y,r.o.y);
		_mm256_store_ps(hit.ray.org_z,r.o.z);
		_mm256_store_ps(hit.ray.tnear,r.mint);
		_mm256_store_ps(hit.ray.tfar, r.maxt);
		//_mm256_store_ps((float*)(void*)hit.ray.mask, (__m256)(__m256i)igi);
		//_mm256_store_ps((float*)(void*)hit.ray.flags,(__m256)(__m256i)zero);
		//_mm256_store_ps((float*)(void*)hit.hit.geomID,(__m256)(__m256i)igi);
		//_mm256_store_ps((float*)(void*)hit.hit.instID[0],(__m256)(__m256i)igi);
		//_mm256_store_epi32(hit.hit.geomID,int32_8(0));

		for(int i = 0; i < 8; i++)
		{
			hit.ray.mask[i] = -1;
			hit.ray.flags[i] = 0;
			hit.hit.geomID[i] = RTC_INVALID_GEOMETRY_ID;
			hit.hit.instID[0][i] = RTC_INVALID_GEOMETRY_ID;
		}

		rtcIntersect8(valid,rt_scene,&hit,&iargs);

		tri* t[8];
		for(int i = 0; i < 8; i++)
			t[i] = &m_tris->at(0);

		for(int i = 0; i < 8; i++)
			if(hit.hit.geomID[i] != RTC_INVALID_GEOMETRY_ID)
				if(hit.hit.primID[i] < m_tris->size())
					t[i] = &m_tris->at(hit.hit.primID[i]);

		vec3f8 o = vec3f8(float8(hit.ray.org_x),float8(hit.ray.org_y),float8(hit.ray.org_z));
		vec3f8 d = vec3f8(float8(hit.ray.dir_x),float8(hit.ray.dir_y),float8(hit.ray.dir_z));

		vertex8 va = vertex8(t[0]->a,t[1]->a,t[2]->a,t[3]->a,t[4]->a,t[5]->a,t[6]->a,t[7]->a);
		vertex8 vb = vertex8(t[0]->b,t[1]->b,t[2]->b,t[3]->b,t[4]->b,t[5]->b,t[6]->b,t[7]->b);
		vertex8 vc = vertex8(t[0]->c,t[1]->c,t[2]->c,t[3]->c,t[4]->c,t[5]->c,t[6]->c,t[7]->c);

		vec3f8 s1 = vb.p - va.p;

		h_rec.wi = d;
		h_rec.pos = fmadd(vec3f8(float8(hit.ray.tfar)),d,o);
		h_rec.dist = float8(hit.ray.tfar);

		float8 b1 = float8(hit.hit.u);
		float8 b2 = float8(hit.hit.v);
		float8 b0 = float8(1.0f) - b1 - b2;
		h_rec.bary = vec3f8(b0,b1,b2);

		h_rec.sh_frame.n = normalize(va.n*b0+vb.n*b1+vc.n*b2);
		calc_shading_frame<float8>(h_rec.sh_frame.n,s1,h_rec.sh_frame);

		for(int32_t i = 0; i < 8; i++)
		{
			if(t[i] != nullptr)
			{
				h_rec.tp[i] = t[i];
				h_rec.m_shape[i].set_material(m_mats->at(t[i]->m));
			}
			else
			{
				h_rec.tp[i] = nullptr;
			}
		}

		return float8(hit.ray.tfar) != r.maxt;
	};

	bool occluded(const ray &r, const vec3f &pos) const
	{
		RTCRayHit hit;
		hit.ray.org_x = r.o.x;
		hit.ray.org_y = r.o.y;
		hit.ray.org_z = r.o.z;
		hit.ray.dir_x = r.d.x;
		hit.ray.dir_y = r.d.y;
		hit.ray.dir_z = r.d.z;
		hit.ray.tnear = r.mint;
		hit.ray.tfar  = r.maxt;
		hit.ray.mask  = -1;
		hit.ray.flags = 0;
		hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		hit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

		rtcIntersect1(rt_scene,&hit);

		if(hit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	//returns true on hits, false on misses
	float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const
	{
		RTCRay8 rr;

		_mm256_store_ps(rr.dir_x,r.d.x);
		_mm256_store_ps(rr.dir_y,r.d.y);
		_mm256_store_ps(rr.dir_z,r.d.z);
		_mm256_store_ps(rr.org_x,r.o.x);
		_mm256_store_ps(rr.org_y,r.o.y);
		_mm256_store_ps(rr.org_z,r.o.z);
		_mm256_store_ps(rr.tnear,r.mint);
		_mm256_store_ps(rr.tfar, r.maxt);

		for(int i = 0; i < 8; i++)
		{
			rr.mask[i] = -1;
			rr.flags[i] = 0;
			rr.time[i] = 0.5f;
		}

		RTCOccludedArguments iargs;
		rtcInitOccludedArguments(&iargs);
		iargs.feature_mask = RTC_FEATURE_FLAG_TRIANGLE;
		iargs.flags = RTC_RAY_QUERY_FLAG_COHERENT;

		rtcOccluded8(valid,rt_scene,&rr,&iargs);

		//negative infinity on tfar means there was a hit
		return (float8(rr.tfar) == float8(-INF));

	};

	void intersect_n(ray_bundle* bundle)
	{throw std::runtime_error("e: don't call this\n");};

	void intersect_n_mt(int32_t id, ray_bundle &bundle)
	{throw std::runtime_error("e: don't call this\n");};
};