#include "../../core/scene.h"
#include "../../base/accelerator.h"
#include "../accelerators/embree.h"
//#include "../accelerators/vk.h"
#include "../accelerators/vk2.h"

accelerator* scene::import_accelerator(FILE* f)
{
	uint32_t accel_type = 0;
	fread(&accel_type,sizeof(accel_type),1,f);
	printf("Accelerator type: %u\n",accel_type);

	uint32_t isa = 0;
	fread(&isa,sizeof(isa),1,f);
	printf("ISA type: %i\n",isa);

	vec3i job_dims = {m_params.job_x,m_params.job_y,m_params.job_z};
	vec3i submit_size = {m_params.submit_x,m_params.submit_y,m_params.submit_z};

	//if(accel_type == 0)
		//return new vk_accelerator(this);
	//else 
	if(accel_type == 2)
		return new vk_accelerator2(job_dims,submit_size,this);

	return new embree_accelerator(this/*,std::string("stuff")*/);
};