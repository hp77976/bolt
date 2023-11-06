#pragma once
#include <stdint.h>
#include <stdio.h>
#include "ray.h"
#include "math/include.h"

class material;
class emitter;
class camera;
class pd_record;

class shape
{
	protected:
	camera*   m_camera   = nullptr;
	material* m_material = nullptr;
	emitter*  m_emitter  = nullptr;

	bool has_camera   = false;
	bool has_emitter  = false;
	bool has_material = false;

	public:
	bool is_emitter() const {return has_emitter;};
	
	bool is_sensor() const {return has_camera;};

	bool is_material() const {return has_material;};

	camera* get_camera() const {return has_emitter ? m_camera : nullptr;};

	material* get_material() const {return has_material ? m_material : nullptr;};

	emitter* get_emitter() const {return has_emitter ? m_emitter : nullptr;};

	void set_camera(camera* c) {m_camera = c; has_camera = true;};

	void set_material(material* m) {m_material = m; has_material = true;};

	void set_emitter(emitter* e) {m_emitter = e; has_emitter = true;};

	bool ray_intersect(const ray &r) const {return 0.0f;};

	void sample_position(pd_record &p_rec, const vec2f &s) const {};

	float pdf_position(const pd_record &p_rec) const {return 0.0f;};

	void sample_direct(pd_record &d_rec, const vec2f &s) const {};

	float pdf_direct(const pd_record &d_rec) const {return 0.0f;};
};

class shape8
{
	camera*   m_camera   = nullptr;
	material* m_material = nullptr;
	emitter*  m_emitter  = nullptr;

	bool has_camera   = false;
	bool has_emitter  = false;
	bool has_material = false;
};