#include "depth.h"
#include "../../../base/material.h"
#include "../../../base/light.h"

void depth_worker::render(const job_desc &jd) {};

void depth_worker::render_packet(const job_desc &jd) {};

void depth_worker::render_bundle(const job_desc &jd)
{
	/*int32_t job_size = get_accelerator()->get_job_size();

	std::vector<vec2f> px_pos = {};
	bool* mask = new bool[job_size];

	for(int32_t xi = 0; xi < std::sqrt(job_size); xi++)
		for(int32_t yi = 0; yi < std::sqrt(job_size); yi++)
			px_pos.push_back(vec2f(xi+jd.x_range.x,yi+jd.y_range.x));

	for(int32_t i = 0; i < job_size; i++)
	{
		bb.li[i] = 0.0f;
		bb.throughput[i] = 1.0f;
		mask[i] = true;
	}

	m_sampler->generate(0.0f,~0);

	for(int32_t i = 0; i < job_size; i++)
		get_camera()->sample_ray(bb.rb.r[i],px_pos[i],0.0f);

	get_accelerator()->record_and_submit_command(0);
	get_accelerator()->intersect_n(bb.rb);

	for(int32_t i = 0; i < job_size; i++)
		bb.li[i] = bb.rb.h_rec[i].dist / 10.f;

	for(int32_t ji = 0; ji < px_pos.size(); ji++)
		get_film()->splat(bb.li[ji],px_pos[ji]);

	delete[] mask;*/
};