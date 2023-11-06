#pragma once
#include "../core/math/include.h"
#include "../core/geometry/include.h"
#include "../core/ray.h"
#include "../core/spectrum.h"
#include "../core/records.h"
#include <array>

//#define ROOT 32
/*#define JOB_X ROOT * ROOT
#define JOB_Y 8
#define JOB_Z 2
#define JOB_SIZE JOB_X * JOB_Y * JOB_Z*/

//SETS has something to do with threads array size and is super buggy
#define SETS 2 //higher gets a bit faster

class scene;

struct ray_bundle
{
	spectrum* li;
	spectrum* throughput;
	spectrum* weight;
	ray* r;
	hit_record* h_rec;
	vec2f* px_pos;
	int32_t* bounces;
	bool* valid;
	bool* mask;
	bool* terminated;
	bool* committed;
	bool initialized = false;
	int32_t job_size = 0;

	ray_bundle() {};

	void init(int32_t job_size_)
	{
		job_size = job_size_;
		
		li = new spectrum[job_size];
		throughput = new spectrum[job_size];
		weight = new spectrum[job_size];
		r = new ray[job_size];
		h_rec = new hit_record[job_size];
		px_pos = new vec2f[job_size];
		bounces = new int32_t[job_size];
		valid = new bool[job_size];
		mask = new bool[job_size];
		terminated = new bool[job_size];
		committed = new bool[job_size];

		initialized = true;
	};

	void reset(int32_t i)
	{
		li[i] = 0.0f;
		throughput[i] = 1.0f;
		weight[i] = 0.0f;
		bounces[i] = 0;
		valid[i] = true;
		mask[i] = true;
		terminated[i] = false;
		committed[i] = false;
	};

	void destroy()
	{
		delete[] li;
		delete[] throughput;
		delete[] weight;
		delete[] r;
		delete[] h_rec;
		delete[] px_pos;
		delete[] bounces;
		delete[] valid;
		delete[] mask;
		delete[] terminated;
		delete[] committed;
		
		initialized = false;
	};

	~ray_bundle() {};
};

class accelerator
{
	protected:
	mutable std::shared_ptr<std::vector<tri>> m_tris = {};
	mutable std::shared_ptr<std::vector<uv_map*>> m_uv_maps;
	mutable std::shared_ptr<std::vector<material*>> m_mats = {};
	mutable uint64_t total_hits = 0;

	public:
	accelerator(scene* s);

	virtual bool intersect(ray &r, hit_record &h_rec) const = 0;

	virtual float8 intersect8(ray8 &r, hit_record8 &h_rec, int32_t* valid) const = 0;

	virtual bool occluded(const ray &r, const vec3f &pos) const = 0;

	virtual float8 occluded8(const ray8 &r, const vec3f8 &pos, int32_t* valid) const = 0;
	
	virtual void intersect_s(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};
	
	virtual void intersect_n(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};

	virtual void push_pos(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};

	virtual void push_dir(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};

	virtual void reset_cmd_buff(int32_t id, int32_t index)
	{throw std::runtime_error("do not call\n");};

	virtual void pull_dir(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};

	virtual void pull_pos(int32_t id, int32_t index, ray_bundle &bundle)
	{throw std::runtime_error("do not call\n");};

	virtual void submit_t(int32_t id) {throw std::runtime_error("do not call\n");};

	virtual vec3i get_job_dims() const {throw std::runtime_error("do not call\n");};

	virtual int32_t get_job_size() const {return 0;};

	uint64_t get_total_hits() const {return total_hits;};
};