
#include "scene.h"
#include "../base/accelerator.h"
#include "../base/integrator.h"
#include "../base/light.h"
#include "../base/camera.h"
#include "logging.h"
#include "film.h"
#include "util/strings.h"
#include "util/checks.h"

light* scene::get_random_light(float* light_pdf, float sample) const
{
	//this cannot occur, scenes will not start with no lights!
	//if(m_lights->size() == 0)
	//	return nullptr;
		
	int idx = std::round(sample * (m_lights->size() - 1));

	*light_pdf = 1.0f / m_lights->size();

	return m_lights->at(idx);
};

spectrum scene::sample_light_direct(
	pd_record* const d_rec, const vec3f &rng, bool test_vis
) const
{
	float l_pdf = 1.0f;
	light* lp = get_random_light(&l_pdf,rng.x);
	null_throw(lp,"sample direct light\n");
	d_rec->object = lp;

	spectrum value = lp->sample_direct(d_rec,vec2f(rng.y,rng.z));

	if(d_rec->pdf == 0.0f) //this check is cheaper than occlusion, so rule it out first!
		return 0.0f;

	ray r = ray(d_rec->ref_pos,d_rec->dir);
	r.maxt = d_rec->dist - 1e-7f;
	r.mint += 1e-7f;

	if(test_vis)
		if(m_accelerator->occluded(r,d_rec->pos))
			return 0.0f;

	d_rec->pdf *= l_pdf;
	value /= l_pdf;
	return value;
};

spectrum8a scene::sample_light_direct8(
	pd_record8* const d_rec, const vec3f8 &rng, int32_t* valid, float8 &mask
) const
{
	float l_pdf = 1.0f;
	light* lp = get_random_light(&l_pdf,rng.x[0]);
	for(int i = 0; i < 8; i++)
		d_rec->object[i] = lp;

	spectrum8a value = lp->sample_direct8(d_rec,vec2f8(rng.y,rng.z));
	
	ray8 r = ray8(d_rec->ref_pos,d_rec->dir);
	r.maxt = d_rec->dist - 1e-7f;
	r.mint += 1e-7f;
	mask = m_accelerator->occluded8(r,d_rec->pos,valid);

	d_rec->pdf *= l_pdf;
	value /= l_pdf;
	return value;
};

scene::scene(const char* scene_file_path)
{
	m_tris = std::shared_ptr<std::vector<tri>>(new std::vector<tri>());
	m_mats = std::shared_ptr<std::vector<material*>>(new std::vector<material*>());
	m_uv_maps = std::shared_ptr<std::vector<uv_map*>>(new std::vector<uv_map*>());
	m_lights = std::shared_ptr<std::vector<light*>>(new std::vector<light*>());

	FILE* sf = fopen(scene_file_path,"rb"); //scene file
	if(!sf)
		log(LOG_ERROR,"Failed to open scene file!\n");

	header s_hdr; //scene header
	fread(&s_hdr,sizeof(header),1,sf);

	std::string data_path = read_string(sf);
	std::string mesh_path = read_string(sf);

	log(
		LOG_STATUS,
		"Resolution: " + std::to_string(s_hdr.res_x) + "x" + std::to_string(s_hdr.res_y) + "\n" +
		"Samples per pixel: " + std::to_string(s_hdr.samples.per_pixel) + "\n" + 
		"Data path: " + data_path + "\n" +
		"Mesh path: " + mesh_path + "\n"
	);

	m_params.samples = s_hdr.samples.samples;
	m_params.spp = s_hdr.samples.per_pixel;
	m_params.tile_size = s_hdr.samples.tile_size;
	m_params.packet_requested = s_hdr.samples.packet_requested;
	m_params.pixel_sampler = s_hdr.samples.pixel_sampler;
	m_params.job_x = s_hdr.samples.job_x;
	m_params.job_y = s_hdr.samples.job_y;
	m_params.job_z = s_hdr.samples.job_z;
	m_params.submit_x = s_hdr.samples.submit_x;
	m_params.submit_y = s_hdr.samples.submit_y;
	m_params.submit_z = s_hdr.samples.submit_z;
	m_params.tile_x = s_hdr.samples.tile_x;
	m_params.tile_y = s_hdr.samples.tile_y;
	m_params.refill_threshold = s_hdr.samples.refill_threshold;

	printf("job_x: %i\n",m_params.job_x);
	printf("job_y: %i\n",m_params.job_y);
	printf("job_z: %i\n",m_params.job_z);

	printf("variant: %u\n",m_params.packet_requested);

	m_film = new film(
		vec2i(s_hdr.res_x,s_hdr.res_y),
		vec2i(s_hdr.res_x,s_hdr.res_y),
		vec2i(0,0),import_filter(sf)
	);

	m_film->clear(); //ensure that the values are allocated and zeroed properly

	bool got_accelerator = false;
	bool got_integrator  = false;
	bool got_materials = false;
	bool got_meshes = false;
	bool got_camera = false;
	bool got_images = false;
	bool got_lights = false;
	bool got_uvmaps = false;
	bool got_film = true;

	uint32_t item_count = 0;

	m_sampler = import_sampler(sf);

	//all headers do need to be found, even if they contain nothing in them, otherwise error
	while(!(
		got_meshes&&got_images&&got_lights&&got_materials&&
		got_camera&&got_film&&got_integrator
	))
	{
		uint32_t char_count = 0;
		fread(&char_count,sizeof(char_count),1,sf);
		char* section_name = new char[char_count];
		fread(section_name,sizeof(char),char_count,sf);

		fread(&item_count,sizeof(item_count),1,sf);

		/*if((strncmp(section_name,SECTION_ACCELERATOR,char_count) == 0) && !got_accelerator)
		{
			m_accelerator = import_accelerator(sf);
			got_accelerator = true;
		}
		else*/ if((strncmp(section_name,SECTION_INTEGRATOR,char_count) == 0) && !got_integrator)
		{
			m_integrator = import_integrator(sf);
			got_integrator = true;
		}
		else if((strncmp(section_name,SECTION_CAMERA,char_count) == 0) && !got_camera && got_film)
		{
			m_camera = import_camera(sf);
			got_camera = true;
		}
		else if((strncmp(section_name,SECTION_MESHES,char_count) == 0) && !got_meshes)
		{
			log(LOG_DEBUG,"Mesh count: " + std::to_string(item_count) + "\n");
			for(uint32_t i = 0; i < item_count; i++)
				import_mesh_header(sf);

			got_meshes = true;
		}
		else if((strncmp(section_name,SECTION_IMAGES,char_count) == 0) && !got_images)
		{
			log(LOG_DEBUG,"Image count: " + std::to_string(item_count) + "\n");
			for(uint32_t i = 0; i < item_count; i++)
				import_image(sf);

			got_images = true;
		}
		else if((strncmp(section_name,SECTION_LIGHTS,char_count) == 0) && !got_lights)
		{
			log(LOG_DEBUG,"Light count: " + std::to_string(item_count) + "\n");
			for(uint32_t i = 0; i < item_count; i++)
				import_light(sf);

			got_lights = true;
		}
		else if((strncmp(section_name,SECTION_MATERIALS,char_count) == 0) && !got_materials)
		{
			log(LOG_DEBUG,"Material count: " + std::to_string(item_count) + "\n");
			for(uint32_t i = 0; i < item_count; i++)
				m_mats->push_back(import_material(sf));

			got_materials = true;
		}
		else
		{
			log(LOG_ERROR,"Uknown section type: " + std::string(section_name) + "!\n");
		}

		delete[] section_name;
	}

	FILE* mesh_file = fopen(mesh_path.c_str(),"rb");
	if(!mesh_file)
		throw std::runtime_error("Failed to open mesh file!\n");
	import_mesh_binary(mesh_file);
	fclose(mesh_file);

	//this needs to be done post mesh import due to the mesh binary not existing prior
	{
		uint32_t char_count = 0;
		fread(&char_count,sizeof(char_count),1,sf);
		char* section_name = new char[char_count];
		fread(section_name,sizeof(char),char_count,sf);

		fread(&item_count,sizeof(item_count),1,sf);

		if((strncmp(section_name,SECTION_ACCELERATOR,char_count) == 0) && !got_accelerator)
		{
			m_accelerator = import_accelerator(sf);
			got_accelerator = true;
		}
		else
		{
			throw std::runtime_error("Failed to read accelerator!\n");
		}

		delete[] section_name;
	}

	if(m_accelerator == nullptr)
		throw std::runtime_error("Failed to get accelerator!\n");

	log(LOG_DEBUG,"Completed scene header read\n");
	log(LOG_DEBUG,mesh_path + "\n");
	fclose(sf);

	//a few last checks just in case!
	if(m_lights->size() == 0)
		log(LOG_ERROR,"No lights in scene! Exitting...\n");

	null_throw(m_integrator,"integrator\n");
	null_throw(m_accelerator,"accelerator\n");
	null_throw(m_camera,"camera\n");
	null_throw(m_film,"film\n");
	null_throw(this,"scene\n");
};

void scene::render()
{
	int32_t block_size = m_params.tile_size;

	//this simple tiling system tremendously reduces noise and slightly improves sample speed
	//divide the film into mostly equally sized chunks
	int32_t sx = get_film()->m_size.x;
	int32_t sy = get_film()->m_size.y;

	std::vector<job_desc> tile_jobs;

	if(m_params.packet_requested != 1)
	{
		for(int32_t xi = 0; xi < sx;)
		{
			int32_t xa = xi + block_size < sx ? block_size : sx - xi;
			for(int32_t yi = 0; yi < sy;)
			{
				int32_t ya = yi + block_size < sy ? block_size : sy - yi;
				job_desc jd;
				jd.x_range = vec2i(xi,xi+xa);
				jd.y_range = vec2i(yi,yi+ya);
				jd.spp = std::max((uint32_t)1,m_params.spp);
				jd.submit = true;
				tile_jobs.push_back(jd);
				yi += ya;
			}
			xi += xa;
		}
	}
	else
	{
		for(int32_t xi = 0; xi < sx; xi += m_params.tile_x)
		{
			for(int32_t yi = 0; yi < sy; yi += m_params.tile_y)
			{
				job_desc jd;
				jd.x_range.x = xi;
				jd.y_range.x = yi;
				jd.spp = std::max((uint32_t)1,m_params.spp);
				jd.submit = true;
				tile_jobs.push_back(jd);
			}
		}
	}

	int32_t tile_count = tile_jobs.size();
	for(int i = 0; i < m_params.samples; i++)
		for(int t = 0; t < tile_count; t++)
			m_integrator->queue(tile_jobs.at(t));

	printf("Film size: %i\n",sx*sy);
	printf("Queued %lu tiles\n",m_integrator->job_size());

	if(!m_integrator->supports_packet() && m_params.packet_requested)
		log(LOG_STATUS,"Integrator does not support packeting, falling back to scalar!\n");

	//m_integrator->start(m_integrator->supports_packet() && (m_params.packet_requested == 1));

	//control thread needs to run on its own and loop
	//in order to manage vulkan command queueing

	control_thread = std::thread(std::bind(
		&integrator::start,m_integrator,
		//m_integrator->supports_packet() &&
		(m_params.packet_requested /*> 0*/)
	));
};

void scene::complete()
{
	get_integrator()->stop();
	control_thread.join();
};