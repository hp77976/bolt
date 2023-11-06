#include "../../core/scene.h"
#include "../../core/logging.h"
#include "../../core/util/strings.h"
#include "../materials/diffuse.h"
#include "../materials/glass.h"

#include <queue>

material* scene::import_material(FILE* f)
{
	//read header and mesh id for grabbing the correct UVs
	uint64_t mat_id = -1;
	fread(&mat_id,sizeof(mat_id),1,f);
	std::string mat_name = read_string(f);

	//log(LOG_STATUS,"Material name: " + mat_name + "\n");

	uint64_t mesh_id;
	fread(&mesh_id,sizeof(mesh_id),1,f);
	std::string mesh_name = read_string(f);

	//log(LOG_STATUS,"Material mesh name: " + mesh_name + "\n");

	//import node tree
	//std::queue<mix_material*> mix_queue = {};

	uint64_t id;
	uint32_t type;
	material* mat  = nullptr;
	material* temp = nullptr;
	uint64_t needed_nodes = 1;
	uint64_t node_idx = 0;
	
	while(needed_nodes > 0)
	{		
		bool got_mix = false;

		fread(&id,sizeof(uint64_t),1,f);
		fread(&type,sizeof(uint32_t),1,f);

		switch(type)
		{
			case(MAT_DIFFUSE):
				temp = new diffuse(f,this);
				break;
			case(MAT_GLASS):
				temp = new glass(f,this);
				break;
			//TODO: fix metal
			/*case(MAT_METAL):
				temp = new metal(f,this);
				break;*/
			/*case(MAT_MIX):
				temp = new mix_material(f,mipmap_name_ptr,mesh_name_s.uv_name_idx_ptr.at(mesh_name));
				got_mix = true;
				needed_nodes += 2;
				break;*/
			default: //on unkown type, just fall back and use a white diffuse material
				log(LOG_WARN,
					"Node ID " + std::to_string(id) + 
					" of unkown type: " + std::to_string(type) + "\n"
				);
				temp = new diffuse(new const_color(spectrum(0.8),false),1.0f);
				break;
		}

		needed_nodes -= 1;

		if(node_idx == 0) //set mat to be first node
		{
			mat = temp;
			if(needed_nodes == 0)
				break;
		}
		//TODO: fix mix node support and re-add mix nodes
		/*else if(needed_nodes >= 1) //this means a mix was encountered
		{
			while(true)
			{
				//this shouldn't be possible to hit, so error if it does happen
				//because that means the material file is probably messed up
				if(mix_queue.size() == 0)
					log(LOG_ERROR,"Mix node had no inputs. Malformed material file?\n");

				if(mix_queue.back()->needs_material())
				{
					mix_queue.back()->add_material(temp);
					break;
				}
				else
				{
					mix_queue.pop();
				}
			}
		}*/

		/*if(got_mix)
		{
			mix_queue.push((mix_material*)temp);
		}*/

		if(node_idx >= 16)
			log(LOG_ERROR,"Too many connected mix nodes!\n");
	}

	//log the material pointer in a map with the mesh and material name for later lookup
	if(mesh_name_s.mat_name_idx_ptr.find(mesh_name) != mesh_name_s.mat_name_idx_ptr.end())
	{
		mesh_name_s.mat_name_idx_ptr.at(mesh_name).emplace(mat_name,std::pair(m_mats->size(),mat));
	}
	else
	{
		std::unordered_map<std::string,std::pair<uint32_t,material*>> temp_map;
		temp_map.emplace(mat_name,std::pair<uint32_t,material*>(m_mats->size(),mat));
		mesh_name_s.mat_name_idx_ptr.emplace(mesh_name,temp_map);
	}

	return mat;
};