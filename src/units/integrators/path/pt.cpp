#include "pt.h"
#include "../../../base/material.h"
#include "../../../base/light.h"
#include "../../../core/util/checks.h"

void path_worker::render(const job_desc &jd)
{
	for(int32_t hi = jd.y_range.y; hi > jd.y_range.x; hi--)
	{
		for(int32_t wi = jd.x_range.y; wi > jd.x_range.x; wi--)
		{
			for(int32_t si = 0; si < jd.spp; si++)
			{
				m_sampler->generate(vec2f(wi,hi),~0);
				
				spectrum li = 0.0f;
				spectrum throughput = 1.0f;

				ray r;
				vec2f px_pos = vec2f(wi,hi) + (get_vec2f() - 0.5f);
				get_camera()->sample_ray(r,px_pos,0.0f);

				hit_record h_rec;
				if(!get_accelerator()->intersect(r,h_rec))
				{
					//do nothing, the ray missed the scene
				}
				else for(int32_t bi = 0; bi < i()->max_bounces(); bi++)
				{
					h_rec.wi = -r.d;
					material* mat = h_rec.get_material();
					
					//attempt direct light sample
					pd_record d_rec = pd_record(h_rec); d_rec.ref_pos = h_rec.pos;
					spectrum l_value = m_scene->sample_light_direct(&d_rec,get_vec3f(),true);
					if(!is_black(l_value) && mat->get_type() & ESmooth)
					{
						bsdf_record b_rec = bsdf_record(h_rec);
						b_rec.lo = h_rec.sh_frame.to_local(d_rec.dir);
						b_rec.mode = TM_RADIANCE;
						
						spectrum e_weight = mat->eval(b_rec,E_SOLID_ANGLE);
						
						if(!is_black(e_weight))
						{
							float e_pdf = mat->pdf(b_rec,get_vec2f(),E_SOLID_ANGLE);
							if(d_rec.measure != E_SOLID_ANGLE && ((light*)d_rec.object)->is_on_surface())
								e_pdf = 0.0f;
							
							li += throughput * l_value * e_weight * mi_weight(d_rec.pdf,e_pdf);
						}
					}

					//sample material to get new direction
					bsdf_record b_rec = bsdf_record(h_rec);
					spectrum weight = mat->sample(b_rec,m_sampler);
					if(is_black(weight))
						break;

					//fill hit records and intersect
					h_rec.from_bsdf(b_rec);
					r = ray(h_rec.pos,h_rec.wo);
					if(!get_accelerator()->intersect(r,h_rec))
						break;

					throughput *= weight;
				}

				if(jd.submit)
				{
					film* f = get_film();
					//std::unique_lock lock(f->lock);
					f->splat(li,px_pos);
				}
			}
		}
	}
};

void path_worker::render_packet(const job_desc &jd)
{
	for(int32_t hi = jd.y_range.y; hi > jd.y_range.x; hi--)
	{
		for(int32_t wi = jd.x_range.y; wi > jd.x_range.x; wi--)
		{
			for(int32_t si = 0; si < jd.spp; si++)
			{
				//m_sampler->generate8(m_sampler->get_index());
				//m_sampler->generate(m_sampler->get_index()+1);

				spectrum8a throughput = spectrum8a(1.0f);
				spectrum8a li = spectrum8a(0.0f);

				ray8 r;
				vec2f8 px_pos = vec2f8(wi,hi) + (get_vec2f8() - float8(0.5f));
				get_camera()->sample_ray8(r,px_pos,0.0f);

				union
				{
					int32_t valid[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
					__m256 mask;
				};

				hit_record8 h_rec;
				mask = get_accelerator()->intersect8(r,h_rec,valid);

				if(!any_true(float8(mask)))
				{
					//all rays missed the scene
				}
				else for(int32_t bi = 0; bi < i()->max_bounces(); bi++)
				{
					h_rec.wi = -r.d;
					material* mat[8] = {
						nullptr,nullptr,nullptr,nullptr,
						nullptr,nullptr,nullptr,nullptr
					};
					for(int i = 0; i < 8; i++)
						if(h_rec.tp[i] != nullptr)
							mat[i] = h_rec.get(i).get_material();
					
					//attempt direct light sample
					float8 di_mask;
					pd_record8 d_rec = pd_record8(h_rec); d_rec.ref_pos = h_rec.pos;
					spectrum8a l_value = m_scene->sample_light_direct8(&d_rec,get_vec3f8(),valid,di_mask);

					if(!all_black(l_value))
					{
						bsdf_record8 b_rec = bsdf_record8(h_rec);
						b_rec.lo = h_rec.sh_frame.to_local(d_rec.dir);
						b_rec.mode = int32_8((int32_t)TM_RADIANCE);
						
						spectrum8a e_weight;
						for(int i = 0; i < 8; i++)
							if(mat[i] != nullptr)
								e_weight.p[i] = mat[i]->eval(b_rec.get(i),E_SOLID_ANGLE);

						//if(!all_black(e_weight))
						{
							float8 e_pdf;
							for(int i = 0; i < 8; i++)
								if(mat[i] != nullptr && mat[i]->get_type() & ESmooth)
									e_pdf.set(i,mat[i]->pdf(b_rec.get(i),get_vec2f(),E_SOLID_ANGLE));
							
							for(int i = 0; i < 8; i++)
								if(mat[i] != nullptr)
									if(
										d_rec.get(i).measure != E_SOLID_ANGLE && 
										((light*)d_rec.get(i).object)->is_on_surface()
									)
										e_pdf.set(i,0.0f);

							float8 mis = mi_weight(d_rec.pdf,e_pdf);
							mis = !di_mask && mis;
							li += throughput * l_value * e_weight * mis;
						}
					}

					bsdf_record8 b_rec = bsdf_record8(h_rec);
					spectrum8a weight;
					
					//reuse the same sample values to help the rays stay correlated
					auto si = m_sampler->save_state();
					for(int i = 0; i < 8; i++)
					{
						if(mat[i] != nullptr)
						{
							//only reload state if needed
							m_sampler->load_state(si);
							weight.p[i] = mat[i]->sample8(b_rec,m_sampler,i);
						}
					}
						
					if(all_black(weight))
						break;

					h_rec.from_bsdf(b_rec);
					r = ray8(h_rec.pos,h_rec.wo);
					mask = get_accelerator()->intersect8(r,h_rec,valid);

					if(_mm256_movemask_ps(mask) == 0x0)
						break;

					throughput *= weight * (float8(mask) && float8(1.0f));
				}
		
				for(int i = 0; i < 8; i++)
					get_film()->splat(li.p[i],vec2f(px_pos.x[i],px_pos.y[i]));
			}
		}
	}
};

bool path_worker::accumulate(int32_t ji, int32_t r)
{
	if(rb[r].valid[ji] && rb[r].mask[ji])
		rb[r].throughput[ji] *= rb[r].weight[ji];
	else if(!rb[r].valid[ji] && rb[r].mask[ji])
		rb[r].mask[ji] = false;

	return rb[r].mask[ji];
};

bool path_worker::evaluate(int32_t ji, int32_t r)
{
	spectrum &li = rb[r].li[ji];
	spectrum &throughput = rb[r].throughput[ji];
	spectrum &weight = rb[r].weight[ji];
	ray &ra = rb[r].r[ji];
	hit_record &h_rec = rb[r].h_rec[ji];

	if(rb[r].mask[ji] && rb[r].valid[ji]) //evaluation of results
	{
		h_rec.wi = -ra.d;
		material* mat = h_rec.get_material();
		null_throw(mat,"material\n");

		//attempt direct light sample
		pd_record d_rec = pd_record(h_rec); d_rec.ref_pos = h_rec.pos;
		spectrum l_value = m_scene->sample_light_direct(&d_rec,get_vec3f(),false);
		if(!is_black(l_value) && mat->get_type() & ESmooth && h_rec.hit_light)
		{
			bsdf_record b_rec = bsdf_record(h_rec);
			b_rec.lo = h_rec.sh_frame.to_local(d_rec.dir);
			b_rec.mode = TM_RADIANCE;
			
			spectrum e_weight = mat->eval(b_rec,E_SOLID_ANGLE);
			
			if(!is_black(e_weight))
			{
				float e_pdf = mat->pdf(b_rec,get_vec2f(),E_SOLID_ANGLE);
				if(d_rec.measure != E_SOLID_ANGLE && ((light*)d_rec.object)->is_on_surface())
					e_pdf = 0.0f;
				
				li += throughput * l_value * e_weight * mi_weight(d_rec.pdf,e_pdf);
			}
		}

		//sample material to get new direction
		bsdf_record b_rec = bsdf_record(h_rec);
		weight = mat->sample(b_rec,m_sampler);
		if(is_black(weight))
		{
			rb[r].mask[ji] = false;
			return false;
		}

		//fill hit records and intersect
		h_rec.from_bsdf(b_rec);
		ra = ray(h_rec.pos,h_rec.wo);

		return true;
	}
	else
	{
		return false;
	}
};

void path_worker::render_bundle(const job_desc &jd)
{
	int32_t job_size = get_accelerator()->get_job_size();
	vec3i dims = get_accelerator()->get_job_dims();
	
	for(int r = 0; r < SETS; r++)
	{
		uint32_t jsi = 0;
		for(int32_t xi = 0; xi < dims.x; xi++)
			for(int32_t yi = 0; yi < dims.y; yi++)
				for(int32_t zi = 0; zi < dims.z; zi++)
				{
					rb[r].px_pos[jsi] = vec2f(xi+jd.x_range.x,yi+jd.y_range.x);
					jsi++;
				}
	}	

	for(int32_t r = 0; r < SETS; r++)
	for(int32_t i = 0; i < rb[r].job_size; i++)
	{
		rb[r].li[i] = 0.0f;
		rb[r].throughput[i] = 1.0f;
		rb[r].mask[i] = true;
		rb[r].valid[i] = true;
	}

	m_sampler->generate(0.0f,~0);

	for(int32_t r = 0; r < SETS; r++)
		for(int32_t i = 0; i < job_size; i++)
			rb[r].px_pos[i] += get_vec2f() - 0.5f;

	for(int32_t r = 0; r < SETS; r++)
		for(int32_t i = 0; i < job_size; i++)
			get_camera()->sample_ray(rb[r].r[i],rb[r].px_pos[i],0.0f);

	for(int32_t r = 0; r < SETS; r++)
	{
		get_accelerator()->reset_cmd_buff(id,r);
		get_accelerator()->push_pos(id,r,rb[r]);
		get_accelerator()->push_dir(id,r,rb[r]);
	}

	get_accelerator()->submit_t(id);

	for(int32_t bi = 0; bi < i()->max_bounces(); bi++)
	{
		int32_t dead_sets = 0;
		for(int32_t r = 0; r < SETS; r++)
		{
			get_accelerator()->intersect_n(id,r,rb[r]);
			
			if(bi != 0)
				for(int32_t ji = 0; ji < job_size; ji++)
					accumulate(ji,r);

			int32_t dead_jobs = 0;
			for(int32_t ji = 0; ji < job_size; ji++)
				if(!evaluate(ji,r))
					dead_jobs++;
			
			if(dead_jobs == job_size)
				dead_sets++;
			
			get_accelerator()->reset_cmd_buff(id,r);
			get_accelerator()->push_dir(id,r,rb[r]);
		}

		if(dead_sets == SETS)
			break;

		get_accelerator()->submit_t(id);
	}

	//need to run one final intersect and accumulate
	for(int32_t r = 0; r < SETS; r++)
	{
		get_accelerator()->intersect_n(id,r,rb[r]);
		for(int32_t ji = 0; ji < job_size; ji++)
		{
			accumulate(ji,r);
			get_film()->splat(rb[r].li[ji],rb[r].px_pos[ji]);
		}
	}
};

void path_worker::render_dynamic()
{
	int32_t job_size = get_accelerator()->get_job_size();
	vec3i dims = get_accelerator()->get_job_dims();
	printf("job_size: %i\n",job_size);

	std::queue<vec2f> px_pos = {};

	{ //fill the pixel queue
		job_desc jd = i()->get_job();

		if(jd.submit)
			for(int32_t r = 0; r < SETS; r++)
				for(int32_t x = jd.x_range.x; x < jd.x_range.y; x++)
					for(int32_t y = jd.y_range.x; y < jd.y_range.y; y++)
						for(int32_t s = 0; s < jd.spp; s++)
							px_pos.push(vec2f(x,y));
		else
			return;
	}

	for(int32_t r = 0; r < SETS; r++)
	{
		for(int32_t ji = 0; ji < job_size; ji++)
		{
			rb[r].px_pos[ji] = px_pos.front() - 0.5f + get_vec2f();
			px_pos.pop();
			get_camera()->sample_ray(rb[r].r[ji],rb[r].px_pos[ji],0.0f);
		}

		get_accelerator()->reset_cmd_buff(id,r);
		get_accelerator()->push_pos(id,r,rb[r]);
		get_accelerator()->push_dir(id,r,rb[r]);
	}

	for(int32_t r = 0; r < SETS; r++)
		for(int32_t i = 0; i < rb[r].job_size; i++)
			rb[r].reset(i);

	m_sampler->generate(0.0f,~0);

	float threshold = m_scene->get_params().refill_threshold;
	bool jobs_over = false;

	get_accelerator()->submit_t(id);

	while(true)
	{
		int32_t dead_rays = 0;

		for(int32_t r = 0; r < SETS; r++)
			get_accelerator()->intersect_n(id,r,rb[r]);

		for(int32_t r = 0; r < SETS; r++)
		{
			for(int32_t ji = 0; ji < job_size; ji++)
			{
				if(rb[r].bounces[ji] >= i()->max_bounces() || !rb[r].valid[ji] || !rb[r].mask[ji])
				{
					//valid ray has too many bounces mark for reset
					rb[r].terminated[ji] = true;
					dead_rays++;
				}
				
				if(!rb[r].terminated[ji])
				{
					evaluate(ji,r);
					accumulate(ji,r);

					rb[r].bounces[ji]++;
				}
				else if(rb[r].terminated[ji])
				{
					accumulate(ji,r);
					get_film()->splat(rb[r].li[ji],rb[r].px_pos[ji]);
					rb[r].committed[ji] = true;
				}
			}
		}

		for(int32_t r = 0; r < SETS; r++)
			get_accelerator()->reset_cmd_buff(id,r);

		while(px_pos.size() <= dead_rays)
		{
			job_desc jd = i()->get_job();

			if(jd.submit)
			{
				for(int32_t r = 0; r < SETS; r++)
					for(int32_t x = jd.x_range.x; x < jd.x_range.y; x++)
						for(int32_t y = jd.y_range.x; y < jd.y_range.y; y++)
							for(int32_t s = 0; s < jd.spp; s++)
								px_pos.push(vec2f(x,y));
			}
			else
			{
				jobs_over = true;
				break;
			}
		}

		if(jobs_over && dead_rays == job_size * SETS)
			break;

		if(((float)dead_rays/((float)job_size*SETS)) >= threshold && !px_pos.empty())
		{
			for(int32_t r = 0; r < SETS; r++)
				get_accelerator()->pull_pos(id,r,rb[r]);

			for(int32_t r = 0; r < SETS; r++)
			{
				for(int32_t ji = 0; ji < job_size; ji++)
				{
					if(px_pos.empty())
						break;
					
					if(rb[r].committed[ji])
					{
						rb[r].reset(ji);
						rb[r].px_pos[ji] = px_pos.front() - 0.5f + get_vec2f();
						px_pos.pop();
						get_camera()->sample_ray(rb[r].r[ji],rb[r].px_pos[ji],0.0f);
					}
				}
			}

			for(int32_t r = 0; r < SETS; r++)
				get_accelerator()->push_pos(id,r,rb[r]);
		}

		for(int32_t r = 0; r < SETS; r++)
			get_accelerator()->push_dir(id,r,rb[r]);

		get_accelerator()->submit_t(id);
	}

	/*int32_t job_size = get_accelerator()->get_job_size();

	for(int32_t r = 0; r < SETS; r++)
	for(int32_t i = 0; i < rb[r].job_size; i++)
	{
		rb[r].li[i] = 0.0f;
		rb[r].throughput[i] = 1.0f;
		rb[r].mask[i] = true;
		rb[r].valid[i] = true;
		rb[r].bounces[i] = 0;
	}

	m_sampler->generate(0.0f,~0);


	std::queue<vec2f> px_pos = {};

	float efficiency = 1.0f; //active jobs / total job size
	float threshold = 0.5f;
	bool jobs_left = true;

	job_desc jd = i()->get_job();
	for(int32_t x = jd.x_range.x; x < jd.x_range.y; x++)
		for(int32_t y = jd.y_range.x; y < jd.y_range.y; y++)
			px_pos.push(vec2f(x,y) + get_vec2f() - 0.5f);
	
	for(int32_t i = 0; i < job_size && i < px_pos.size(); i++)
	{
		get_camera()->sample_ray(rb[0].r[i],px_pos.front(),0.0f);
		rb[0].px_pos[i] = px_pos.front();
		px_pos.pop();
		rb[0].mask[i] = true;
		rb[0].bounces[i] = 0;
	}

	get_accelerator()->reset_cmd_buff(id,0);
	get_accelerator()->push_pos(id,0,rb[0]);
	get_accelerator()->push_dir(id,0,rb[0]);
	get_accelerator()->submit_t(id);

	bool run = true;
	while(run)
	{
		get_accelerator()->intersect_n(id,0,rb[0]);
		
		for(int32_t ji = 0; ji < job_size; ji++)
			if(rb[0].bounces[ji] != 0)
				accumulate(ji,0);

		for(int32_t ji = 0; ji < job_size; ji++)
			evaluate(ji,0);

		get_accelerator()->reset_cmd_buff(id,0);
		get_accelerator()->push_dir(id,0,rb[0]);

		for(int32_t ji = 0; ji < job_size; ji++)
			if(rb[0].mask[ji])
				rb[0].bounces[ji]++;

		//run final accumulate and splat to film if bounces exceeded
		for(int32_t ji = 0; ji < job_size; ji++)
		{
			if(rb[0].bounces[ji] >= i()->max_bounces() && rb[0].mask[ji])
			{
				accumulate(ji,0);
				get_film()->splat(rb[0].li[ji],rb[0].px_pos[ji]);
				rb[0].mask[ji] = false;
			}
		}

		//re calc efficiency
		int32_t active_jobs = job_size;
		for(int32_t i = 0; i < job_size; i++)
			if(!rb[0].mask[i])
				active_jobs--;
		
		efficiency = ((float)active_jobs) / ((float)job_size);

		if(active_jobs == 0 && jobs_left == false)
		{
			run = false;
			break;
		}
		
		//refill dead job positions with new pixels
		if(efficiency <= threshold && jobs_left)
		{
			printf("requested job due to efficieny: %f\n",efficiency);
			job_desc jd = i()->get_job();

			if(jd.submit)
				for(int32_t x = jd.x_range.x; x < jd.x_range.y; x++)
					for(int32_t y = jd.y_range.x; y < jd.y_range.y; y++)
						px_pos.push(vec2f(x,y) + get_vec2f() - 0.5f);

			jobs_left = px_pos.size() > 0;

			if(jobs_left)
			{
				//copy position data from device to host
				get_accelerator()->pull_pos(id,0,rb[0]);

				//update position data with new ray positions
				for(int32_t i = 0; i < job_size && px_pos.size() > 0; i++)
				{
					if(!rb[0].mask[i])
					{
						get_camera()->sample_ray(rb[0].r[i],px_pos.front(),0.0f);
						rb[0].px_pos[i] = px_pos.front();
						px_pos.pop();
						rb[0].mask[i] = true;
						rb[0].bounces[i] = 0;
						if(px_pos.size() == 0)
							break;
					}
				}

				//push data back to device
				get_accelerator()->push_pos(id,0,rb[0]);
			}
			printf("px_pos.size(): %lu\n",px_pos.size());
		}

		get_accelerator()->submit_t(id);
	}*/
};