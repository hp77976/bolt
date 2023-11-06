#include "core/scene.h"
#include "core/spectrum.h"
#include "core/film.h"
#include "core/util/strings.h"
#include "base/integrator.h"

int main(int argc, char** argv)
{
	//seed random with current system time
	srand(time(nullptr));

	//init global color system (do not remove this)
	color co = color();
	co.init();

	log(LOG_STATUS,"INIT\n");

	scene* s = nullptr;

	if(argc > 1) //open scene file from given path
	{
		printf("Got path: %s\n",argv[1]);
		s = new scene(argv[1]);
	}
	else //attempt to open local scene file
	{
		s = new scene("scene.bsc");
	}

	s->render();

	while(s->get_integrator()->job_size() > 0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		printf("jobs: %u\n",s->get_integrator()->job_size());
	}

	printf("hits: %lu\n",s->get_accelerator()->get_total_hits());

	s->get_integrator()->wait_for_null();

	s->get_film()->normalize();
	s->get_film()->write("output.exr",true);

	s->complete();

	delete s;

	return 0;
};