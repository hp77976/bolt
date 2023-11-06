#include "vk.h"
#include "../../../base/material.h"
#include "../../../base/light.h"

void vk_worker::render(const job_desc &jd)
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