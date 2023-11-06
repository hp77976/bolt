
//#include "../units/accelerators/embree.h"
#include "../core/random/rng.h"

/*float8 intersect8(
	RTCScene &rt_scene, ray8 &r, hit_record8 &h_rec, int32_t* valid,
	std::shared_ptr<std::vector<tri>> &m_tris
)
{
	float8 ret = !float8(0.0f);

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

	vec2f8 uv = vec2f8(float8(hit.hit.u),float8(hit.hit.v));

	float8 b1 = uv.x;
	float8 b2 = uv.y;
	float8 b0 = float8(1.0f) - b1 - b2;
	h_rec.bary = vec3f8(b0,b1,b2);

	h_rec.sh_frame.n = normalize(va.n*b0+vb.n*b1+vc.n*b2);
	calc_shading_frame<float8>(h_rec.sh_frame.n,s1,h_rec.sh_frame);

	for(int32_t i = 0; i < 8; i++)
		if(t[i] != nullptr)
			h_rec.tp[i] = t[i];
		else
			h_rec.tp[i] = nullptr;*/

	/*float8 geo_ids = (__m256)int32_8((int32_t*)hit.hit.geomID);*/
	/*return select(geo_ids!=float8(-1.0f),!float8(0.0f),float8(0.0f));*/

	//return float8(hit.ray.tfar) != r.maxt;

	/*for(int32_t i = 0; i < 8; i++)
		if(h_rec.tp[i])
			ret[i] = 0.0f;

	return ret;*/
//};

/*void generate_ray(random_source* rng, ray8* r8)
{
	r8->d = vec3f8(float8(rng->get_float()-0.5f),float8(rng->get_float()-0.5f),float8(-1.0f));
	r8->o = vec3f8(float8(rng->get_float()-0.5f),float8(rng->get_float()-0.5f),float8( 2.0f));
};*/

int main()
{
	/*auto m_tris = std::shared_ptr<std::vector<tri>>(new std::vector<tri>());
	tri t0 = tri();
	t0.a.p = vec3f(-1.0f, 1.0f,0.0f);
	t0.b.p = vec3f( 1.0f,-1.0f,0.0f);
	t0.c.p = vec3f(-1.0f,-1.0f,0.0f);
	tri t1 = tri();
	t1.a.p = vec3f(-1.0f, 1.0f,0.0f);
	t1.b.p = vec3f( 1.0f, 1.0f,0.0f);
	t1.c.p = vec3f( 1.0f,-1.0f,0.0f);
	m_tris->push_back(t0);
	m_tris->push_back(t1);

	RTCDevice dev = rtcNewDevice(nullptr);
	if(!dev)
		throw std::runtime_error("a\n");

	rtcSetDeviceErrorFunction(dev, errorFunction, NULL);
	RTCScene rt_scene = rtcNewScene(dev);
	RTCGeometry geo = rtcNewGeometry(dev,RTC_GEOMETRY_TYPE_TRIANGLE);

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
		throw std::runtime_error("failed to alloc verts\n");

	if(!indices)
		throw std::runtime_error("failed to alloc indices\n");

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
		printf("x: %lu\n",x);
		printf("tc3: %lu\n",tri_count*3);
		throw std::runtime_error("\n");
	}

	printf("copied data in\n");

	rtcCommitGeometry(geo);
	rtcAttachGeometry(rt_scene,geo);
	rtcReleaseGeometry(geo);
	rtcCommitScene(rt_scene);

	printf("Built Embree Accelerator\n");

	random_source* rng = new random_source(0);

	ray8 r8;
	float8 result;
	hit_record8 h_rec;
	int32_t valid[8];
	for(int i = 0; i < 8; i++)
		valid[i] = -1;

	generate_ray(rng,&r8);
	printf("ray:\n");
	printf("x: %s\n",r8.o.x.str().c_str());
	printf("y: %s\n",r8.o.y.str().c_str());
	printf("z: %s\n",r8.o.z.str().c_str());

	result = intersect8(rt_scene,r8,h_rec,valid,m_tris);
	printf("result: %s\n",result.str().c_str());
	printf("x: %s\n",h_rec.pos.x.str().c_str());
	printf("y: %s\n",h_rec.pos.y.str().c_str());
	printf("z: %s\n",h_rec.pos.z.str().c_str());

	union
	{
		int32_t v[8];
		__m256 f;
	};

	f = !float8(0.0f);
	generate_ray(rng,&r8);
	printf("ray:\n");
	printf("x: %s\n",r8.o.x.str().c_str());
	printf("y: %s\n",r8.o.y.str().c_str());
	printf("z: %s\n",r8.o.z.str().c_str());

	result = intersect8(rt_scene,r8,h_rec,v,m_tris);
	printf("result: %s\n",result.str().c_str());
	printf("x: %s\n",h_rec.pos.x.str().c_str());
	printf("y: %s\n",h_rec.pos.y.str().c_str());
	printf("z: %s\n",h_rec.pos.z.str().c_str());

	rtcReleaseScene(rt_scene);
	rtcReleaseDevice(dev);

	return 0;*/
};