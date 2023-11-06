#pragma once
#include "../../../base/integrator.h"
#include "../../../base/accelerator.h"
#include "../../../base/camera.h"
#include "../../../core/scene.h"
#include "../../../core/film.h"

class path_integrator : public integrator
{
	int32_t bounces = 0;

	public:
	path_integrator(scene* s, int samples, int bounces_) : integrator(s,samples), bounces(bounces_) {};

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

class path_worker : public worker
{
	std::vector<ray_bundle> rb = {};

	public:
	path_worker(path_integrator* i_, int _id) : worker(i_,_id)
	{
		printf("pt worker started\n");
		int32_t js = get_accelerator()->get_job_size();
		printf("js: %i\n",js);

		for(int32_t i = 0; i < SETS; i++)
			rb.push_back(ray_bundle());
	
		for(int32_t i = 0; i < SETS; i++)
			rb[i].init(js);

		printf("rb_size: %lu\n",rb.size());
	};

	~path_worker()
	{
		printf("pt end\n");
	};

	path_integrator* i() const {return (path_integrator*)m_i;};

	void render(const job_desc &jd);

	void render_packet(const job_desc &jd);

	void render_bundle(const job_desc &jd);

	void render_dynamic();

	bool accumulate(int32_t ji, int32_t r);

	bool evaluate(int32_t ji, int32_t r);
};

inline worker* path_integrator::spawn_worker(int32_t _id)
{
	return new path_worker(this,_id);
};