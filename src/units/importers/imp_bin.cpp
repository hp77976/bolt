#include "../../core/scene.h"
#include "../../base/accelerator.h"
//#include "../accelerators/embree.h"
#include "../../core/util/strings.h"
#include "../../core/geometry/include.h"

//TODO: clean this mess up

void scene::import_mesh_binary(FILE* f)
{
	uint64_t mesh_count = 0;
	fread(&mesh_count,sizeof(mesh_count),1,f);
	//logger::log(LOG_DEBUG,"Mesh count: " + std::to_string(mesh_count) + "\n");

	uint64_t total_tris = 0;

	for(uint64_t i = 0; i < mesh_count; i++)
	{
		std::string mesh_name = read_string(f);
		//log(LOG_DEBUG,"Mesh name: " + mesh_name + "\n");
		
		uint64_t tri_count = 0;
		fread(&tri_count,sizeof(tri_count),1,f);
		//log(LOG_DEBUG,"Tri count: " + std::to_string(tri_count) + "\n");
		tri* tri_array = new tri[tri_count];
		fread(tri_array,sizeof(tri),tri_count,f);

		//printf("bytes: %lu\n",sizeof(tri)*tri_count);
		//printf("tri bytes: %lu\n",sizeof(tri));
		//printf("vec3f bytes: %lu\n",sizeof(vec3f));
		
		uint64_t mat_count = 0;
		fread(&mat_count,sizeof(mat_count),1,f);
		//log(LOG_DEBUG,"Mat count: " + std::to_string(mat_count) + "\n");

		umap<int32_t,std::string> mat_idx_map = {};
		for(int32_t i = 0; i < (int32_t)mat_count; i++)
		{
			std::string mat_name = read_string(f);
			//log(LOG_DEBUG,"Mat name: " + mat_name + "\n");
			mat_idx_map.insert({i,mat_name});
		}
		
		//set the tri indices and mat indices
		for(uint64_t i = 0; i < tri_count; i++)
		{
			tri_array[i].i = total_tris + i;
			//printf("%i\n",tri_array[i].m);

			std::string midx = mat_idx_map.at(tri_array[i].m);
			tri_array[i].m = mesh_name_s.mat_name_idx_ptr.at(mesh_name).at(midx).first;
			//log(LOG_DEBUG,"midx: " + midx + "\n");
			//log(LOG_DEBUG,"mesh_name: " + mesh_name + "\n");
		}

		uint64_t uv_count = 0;
		fread(&uv_count,sizeof(uv_count),1,f);
		//log(LOG_DEBUG,"UV count: " + std::to_string(uv_count) + "\n");
		//printf("tri_uv size: %lu\n",sizeof(tri_uvs));

		for(uint64_t ui = 0; ui < uv_count; ui++)
		{
			std::pair<uint32_t,uv_map*> new_um = std::pair<uint32_t,uv_map*>(-1,new uv_map());
			m_uv_maps->push_back(new_um.second);
			mesh_name_s.uv_idx_idx_ptr.at(mesh_name).emplace(ui,new_um);

			tri_uvs* uv_sets = new tri_uvs[tri_count];
			fread(uv_sets,sizeof(tri_uvs),tri_count,f);

			uv_map* um = mesh_name_s.uv_idx_idx_ptr.at(mesh_name).at(ui).second;
			um->uv_coords.insert(um->uv_coords.end(),&uv_sets[0],&uv_sets[tri_count]);

			delete[] uv_sets;
		}

		m_tris->insert(m_tris->end(),&tri_array[0],&tri_array[tri_count]);
		total_tris += tri_count;
		mesh_name_s.tri_idx.insert({mesh_name,total_tris});

		delete[] tri_array;
	}
};