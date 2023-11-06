#include "../../core/scene.h"
#include "../../core/logging.h"
#include "../lights/point.h"

void scene::import_light(FILE* f)
{
	uint32_t type = 0;
	fread(&type,sizeof(type),1,f);

	light* li = nullptr;
	switch(type)
	{
		case(LI_POINT):
			li = new point_light(f);
			break;
		default:
			log(LOG_ERROR,"Uknown light type: " + std::to_string(type) + "!\n");
			break;
	}

	if(li != nullptr)
		m_lights->push_back(li);
};