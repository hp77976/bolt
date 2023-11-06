#pragma once
#include "../core/math/include.h"
#include "../core/ray.h"
#include "../core/threading.h"
#include "../core/film.h"
#include "../core/scene.h"
#include "camera.h"
#include "sampler.h"
#include "accelerator.h"

class scene;
class worker;

class integrator : public thread_pool
{
	protected:
	scene* m_scene = nullptr;
	int32_t sample_count = 0;

	public:
	integrator(scene* s, int32_t samples) : m_scene(s), sample_count(samples) {};

	inline scene* get_scene() const {return m_scene;};

	virtual worker* spawn_worker(int32_t id) = 0;

	virtual film* get_preview() = 0;

	virtual bool supports_packet() const = 0;

	virtual bool supports_bundle() const = 0;
};

class worker
{
	protected:
	integrator* m_i = nullptr;
	scene* m_scene = nullptr;
	sampler* m_sampler = nullptr;

	std::mutex wait_lock;
	bool m_waiting = false;

	bool m_waiting_to_push = false;

	int32_t id = -1;

	public:
	worker(integrator* i, int32_t _id) : m_i(i), id(_id)
	{
		m_scene = m_i->get_scene();
		m_sampler = m_scene->get_sampler()->clone(_id);
	};

	virtual void render(const job_desc &jd) = 0;

	virtual void render_packet(const job_desc &jd) = 0;

	virtual void render_bundle(const job_desc &jd) = 0;

	virtual void render_dynamic() {throw std::runtime_error("don't call this!\n");};

	inline float get_float() const {return m_sampler->get_float();};

	inline vec2f get_vec2f() const {return m_sampler->get_vec2f();};

	inline vec3f get_vec3f() const {return m_sampler->get_vec3f();};

	inline vec4f get_vec4f() const {return m_sampler->get_vec4f();};

	inline float8 get_float8() const {return m_sampler->get_float8();};

	inline vec2f8 get_vec2f8() const {return m_sampler->get_vec2f8();};

	inline vec3f8 get_vec3f8() const {return m_sampler->get_vec3f8();};

	inline vec4f8 get_vec4f8() const {return m_sampler->get_vec4f8();};

	inline film* get_film() const {return m_scene->get_film();};

	inline camera* get_camera() const {return m_scene->get_camera();};

	inline accelerator* get_accelerator() const {return m_scene->get_accelerator();};
	
	int32_t get_id() const {return id;};

	bool waiting_for_submit() const {return m_waiting;};

	void clear_submit_wait()
	{
		{
			std::unique_lock<std::mutex> lock(wait_lock);
			m_waiting = false;
		}
	};

	bool waiting_to_push() const {return m_waiting_to_push;};

	void clear_push_wait()
	{
		{
			std::unique_lock<std::mutex> lock(wait_lock);
			m_waiting_to_push = false;
		}
	};
};