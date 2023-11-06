#include "../../core/scene.h"
#include "../../core/logging.h"
#include "../../base/integrator.h"
#include "../integrators/path/pt.h"
#include "../integrators/depth/depth.h"

integrator* scene::import_integrator(FILE* f)
{
	uint32_t type = 0;
	fread(&type,sizeof(type),1,f);

	integrator* i = nullptr;

	uint32_t sample_count = 0;
	uint32_t eye_bounces = 0;
	uint32_t light_bounces = 0;
	fread(&sample_count,sizeof(sample_count),1,f);
	fread(&eye_bounces,sizeof(eye_bounces),1,f);
	fread(&light_bounces,sizeof(light_bounces),1,f);

	switch(type)
	{
		case(0):
			log(LOG_STATUS,"Path tracer selected\n");
			i = new path_integrator(this,sample_count,eye_bounces);
			//log(LOG_ERROR,"Path is not supported!\n");
			break;
		case(1):
			log(LOG_STATUS,"Depth tracer selected\n");
			i = new depth_integrator(this,sample_count,eye_bounces);
			//log(LOG_ERROR,"Path is not supported!\n");
			break;
		/*case(1):
			logger::log(LOG_STATUS,"BiDir selected\n");
			i = new bdpt_integrator(this,sample_count,eye_bounces,light_bounces);
			break;*/
		/*case(2):
			logger::log(LOG_STATUS,"Light tracer selected\n");
			i = new lt_integrator(this,sample_count,eye_bounces);
			break;*/
		//case(3):
			
		//	break;
		//case(4):
		//	logger::log(LOG_STATUS,"Async integrator selected\n");
		//	i = new async_integrator(this,sample_count,eye_bounces);
		//	break;
		/*case(5):
			log(LOG_STATUS,"Path AVX Integrator selected\n");
			i = new pt_avx_integrator(this,sample_count,eye_bounces);
			break;*/
		default:
			log(LOG_ERROR,"Unknown integrator: " + std::to_string(type) + "!\n");
			break;
	}

	return i;
};