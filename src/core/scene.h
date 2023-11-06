#pragma once
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include "records.h"
#include "spectrum.h"
#include "common.h"

enum mat_type
{
	MAT_DIFFUSE = 0,
	MAT_GLOSSY = 1,
	MAT_GLASS = 2,
	MAT_METAL = 3,
	MAT_MIX = 4,
	MAT_LIGHT = 5,
	MAT_PAPER = 6,
};

enum light_type
{
	LI_POINT = 0,
	LI_SPOT = 1,
	LI_HEMI = 2,
	LI_LASER = 3,
	LI_AREA = 4,
};

class sampler;
class integrator;
class accelerator;
class material;
class emitter;
class light;
class camera;
class film;
struct uv_map;
struct tri;
class bitmap;
class mipmap;
class texture;
class filter;

class scene
{
	const char* SECTION_CAMERA		= "camera";
	const char* SECTION_MESHES		= "meshes";
	const char* SECTION_MATERIALS	= "materials";
	const char* SECTION_LIGHTS		= "lights";
	const char* SECTION_IMAGES		= "images";
	const char* SECTION_UV_MAPS		= "uv_maps";
	const char* SECTION_INTEGRATOR  = "integrator";
	const char* SECTION_ACCELERATOR = "accelerator";

	struct header
	{
		uint32_t version;

		uint32_t res_x;
		uint32_t res_y;

		struct
		{
			uint32_t samples;
			uint32_t per_pixel;
			uint32_t tile_size;
			uint32_t job_x;
			uint32_t job_y;
			uint32_t job_z;
			uint32_t submit_x;
			uint32_t submit_y;
			uint32_t submit_z;
			uint32_t tile_x;
			uint32_t tile_y;
			float refill_threshold;
			uint32_t packet_requested = 0;
			uint32_t pixel_sampler = 0;
		} samples;

		//uint32_t dir_path_length;
	};

	struct m_params_s
	{
		uint32_t samples = 0;
		uint32_t spp = 1;
		uint32_t tile_size = 4;
		uint32_t job_x;
		uint32_t job_y;
		uint32_t job_z;
		uint32_t submit_x;
		uint32_t submit_y;
		uint32_t submit_z;
		uint32_t tile_x;
		uint32_t tile_y;
		float refill_threshold;
		uint32_t packet_requested = 0;
		uint32_t pixel_sampler = 0;
	} m_params;

	struct //get data by mesh name
	{
		//get uvs by name, returns index and pointer
		umap<std::string,umap<std::string,std::pair<uint32_t,uv_map*>>> uv_name_idx_ptr = {};
		//get uvs by index, returns index_index and pointer 
		umap<std::string,umap<uint32_t,std::pair<uint32_t,uv_map*>>> uv_idx_idx_ptr = {};
		//gets materials by name, returns index and pointer
		umap<std::string,umap<std::string,std::pair<uint32_t,material*>>> mat_name_idx_ptr = {};

		//needed for setting the tri indices to match uvs
		umap<std::string,uint32_t> tri_idx = {};
	} mesh_name_s;

	film* m_film = nullptr;
	sampler* m_sampler = nullptr;
	integrator* m_integrator = nullptr;
	accelerator* m_accelerator = nullptr;
	camera* m_camera = nullptr;
	std::shared_ptr<std::vector<tri>> m_tris;
	std::shared_ptr<std::vector<material*>> m_mats;
	std::shared_ptr<std::vector<uv_map*>> m_uv_maps;
	std::shared_ptr<std::vector<light*>> m_lights;
	std::vector<bitmap*> m_bitmaps = {};
	std::vector<mipmap*> m_mipmaps = {};
	std::vector<texture*> m_textures = {};
	umap<std::string,mipmap*> mipmap_name_ptr = {};

	std::thread control_thread;

	void import_mesh_header(FILE* f);

	void import_mesh_binary(FILE* f);

	void import_image(FILE* f);

	void import_light(FILE* f);

	material* import_material(FILE* f);
	
	filter* import_filter(FILE* f);

	camera* import_camera(FILE* f);

	accelerator* import_accelerator(FILE* f);

	integrator* import_integrator(FILE* f);

	sampler* import_sampler(FILE* f);

	public:
	scene(const char* scene_file_path);

	void render();
	
	void complete();

	inline film* get_film() const {return m_film;};

	inline sampler* get_sampler() const {return m_sampler;};
	
	inline integrator* get_integrator() const {return m_integrator;};
	
	inline accelerator* get_accelerator() const {return m_accelerator;};

	inline void set_accelerator(accelerator* a) {m_accelerator = a;};
	
	inline camera* get_camera() const {return m_camera;};

	inline std::shared_ptr<std::vector<light*>> get_lights() const {return m_lights;};

	inline std::shared_ptr<std::vector<tri>> get_tris() const {return m_tris;};
	
	inline std::shared_ptr<std::vector<material*>> get_mats() const {return m_mats;};
	
	inline std::shared_ptr<std::vector<uv_map*>> get_uvs() const {return m_uv_maps;};

	light* get_random_light(float *l_pdf, float sample) const;

	spectrum sample_light_direct(
		pd_record* const d_rec, const vec3f &rng, bool test_vis
	) const;

	spectrum8a sample_light_direct8(
		pd_record8* const d_rec, const vec3f8 &rng, int32_t* valid, float8 &mask
	) const;

	inline m_params_s get_params() const {return m_params;};
};