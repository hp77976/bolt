#include "../cameras/perspective.h"
#include "../../core/scene.h"
#include "../../core/logging.h"

enum camera_type
{
	CAM_PERSPECTIVE = 0,
	CAM_FISHEYE = 1,
};

camera* scene::import_camera(FILE* f)
{
	if(m_film == nullptr)
		log(LOG_ERROR,"Film not imported but needed for camera!\n");

	uint32_t type = 0;
	fread(&type,sizeof(type),1,f);
	camera* c = nullptr;

	printf("type: %u\n",type);

	switch(type)
	{
		case(CAM_PERSPECTIVE):
			c = new perspective_camera(f,m_film);
			break;
		default:
			log(LOG_ERROR,"Unknown camera type!\n");
			break;
	}

	return c;
};