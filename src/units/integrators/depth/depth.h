#pragma once
#include "../../../base/integrator.h"
#include "../../../base/accelerator.h"
#include "../../../base/camera.h"
#include "../../../core/scene.h"
#include "../../../core/film.h"

class depth_integrator : public integrator
{
	int32_t bounces = 0;

	public:
	depth_integrator(scene* s, int samples, int bounces_) : integrator(s,samples), bounces(bounces_) {};

	worker* spawn_worker(int32_t _id);

	film* get_preview()
	{
		film* temp_c_film = new film(m_scene->get_film());
		temp_c_film->copy_from(m_scene->get_film());
		temp_c_film->normalize();

		return temp_c_film;
	};

	bool supports_packet() const {return true;};
	bool supports_bundle() const {return true;};

	//custom functions
	int32_t max_bounces() const {return bounces;};
};

class depth_worker : public worker
{
	public:
	depth_worker(depth_integrator* i_, int _id) : worker(i_,_id)
	{
		//bb.li = new spectrum[JOB_SIZE];
		//bb.throughput = new spectrum[JOB_SIZE];
		//bb.weight = new spectrum[JOB_SIZE];
	};

	~depth_worker()
	{
		/*delete[] bb.li;
		delete[] bb.throughput;
		delete[] bb.weight;*/
	};

	depth_integrator* i() const {return (depth_integrator*)m_i;};

	void render(const job_desc &jd);

	void render_packet(const job_desc &jd);

	void render_bundle(const job_desc &jd);
};

inline worker* depth_integrator::spawn_worker(int32_t _id)
{
	return new depth_worker(this,_id);
};