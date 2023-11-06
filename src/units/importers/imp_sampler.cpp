#include "../../core/scene.h"
#include "../../core/logging.h"
#include "../../core/util/strings.h"
#include "../../base/sampler.h"
#include "../samplers/low_discrepancy.h"
#include "../samplers/partitioned.h"
#include "../samplers/random.h"
#include "../samplers/non_random.h"

sampler* scene::import_sampler(FILE* f)
{
	printf("Got sampler: %u\n",m_params.pixel_sampler);
	switch(m_params.pixel_sampler)
	{
		case(0): return new low_discrepancy_sampler(0);
		case(1): return new partitioned_sampler(0);
		case(2): return new random_sampler(0);
		case(3): return new non_random_sampler(0);
	}
};