#include "../../core/scene.h"
#include "../../base/filter.h"
#include "../filters/gaussian.h"

enum filter_type
{
	FILTER_GAUSSIAN = 0,
	FILTER_LINEAR = 1,
	FILTER_BILINEAR = 2,
	FILTER_TRILINEAR = 3,
};

filter* scene::import_filter(FILE* f)
{
	uint32_t type = 0;
	fread(&type,sizeof(type),1,f);

	filter* f_ = nullptr;

	switch(type)
	{
		case(FILTER_GAUSSIAN):
			f_ = new gaussian_filter(f);
			break;
		default:
			printf("Got filter type: %u\n",type);
			log(LOG_ERROR,"Unsupported filter type!\n");
			break;
	}

	return f_;
};