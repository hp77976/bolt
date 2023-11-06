#include "../../core/scene.h"
#include "../../core/logging.h"
#include "../../core/util/strings.h"
#include "../../core/geometry/include.h"

void scene::import_mesh_header(FILE* f)
{
	uint64_t mesh_id;
	fread(&mesh_id,sizeof(mesh_id),1,f);
	std::string mesh_name = read_string(f);

	//log(LOG_DEBUG,"Mesh name: " + mesh_name + "\n");

	uint32_t uv_count;
	fread(&uv_count,sizeof(uv_count),1,f);

	if(uv_count > 8)
		log(LOG_ERROR,"Too many UV maps in mesh!\n");

	for(uint32_t i = 0; i < uv_count; i++)
	{
		uint64_t uv_id;
		fread(&uv_id,sizeof(uv_id),1,f);
		std::string uv_name = read_string(f);
		
		uv_map* um = new uv_map;
		um->id = uv_id;

		if(i == 0)
		{
			umap<std::string,std::pair<uint32_t,uv_map*>> temp_map;
			temp_map.emplace(uv_name,std::pair(m_uv_maps->size(),um));
			mesh_name_s.uv_name_idx_ptr.emplace(mesh_name,temp_map);

			umap<uint32_t,std::pair<uint32_t,uv_map*>> idx_idx;
			idx_idx.emplace(i,std::pair(m_uv_maps->size(),um));
			mesh_name_s.uv_idx_idx_ptr.emplace(mesh_name,idx_idx);
		}
		else
		{
			mesh_name_s.uv_name_idx_ptr.at(mesh_name).emplace(uv_name,std::pair(m_uv_maps->size(),um));
			mesh_name_s.uv_idx_idx_ptr.at(mesh_name).emplace(i,std::pair(m_uv_maps->size(),um));
		}

		m_uv_maps->push_back(um);
	}

	uint32_t mat_count;
	fread(&mat_count,sizeof(mat_count),1,f);

	if(mat_count > 8)
		log(LOG_ERROR,"Too many Materials per mesh!\n");

	for(uint32_t i = 0; i < mat_count; i++)
	{
		uint64_t mat_id;
		uint32_t mat_name_len;

		fread(&mat_id,sizeof(mat_id),1,f);
		std::string mat_name = read_string(f);
	}
};