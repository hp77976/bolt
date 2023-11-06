#include "integrator.h"
#include "accelerator.h"

void thread_pool::start(int32_t do_packet)
{
	max_threads = std::thread::hardware_concurrency();
#ifdef DETERMINISTIC
	max_threads = 1;
#endif
	max_threads = 4;
	
	for(uint32_t i = 0; i < max_threads; i++)
		workers.push_back(((integrator*)this)->spawn_worker(i));
	printf("Threads spawned: %lu\n",workers.size());

	if(do_packet == 0)
		for(uint32_t i = 0; i < max_threads; i++)
			threads.push_back(std::thread(std::bind(
				&thread_pool::thread_loop_scalar,this,workers.at(i)
			)));
	else if(do_packet == 1)
		for(uint32_t i = 0; i < max_threads; i++)
			threads.push_back(std::thread(std::bind(
				&thread_pool::thread_loop_packet,this,workers.at(i)
			)));
	else if(do_packet == 2)
		for(uint32_t i = 0; i < max_threads; i++)
			threads.push_back(std::thread(std::bind(
				&thread_pool::thread_loop_dynamic,this,workers.at(i)
			)));

	if(do_packet)
	{		
		//don't use the busy() check. it doesn't work
		while(true)
		{
			/*bool all_null = true;
			for(int32_t i = 0; i < workers.size(); i++)
				if(workers.at(i) != nullptr)
					all_null = false;*/

			if(done /*|| all_null*/)
			{
				break;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(100));
			}
		}
	}
};

job_desc thread_pool::get_job()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		mutex_condition.wait(lock,[this]{return !jobs.empty() || done;});

		if(done || jobs.empty())
		{
			job_desc jd;
			jd.spp = 0;
			jd.x_range = {0,0};
			jd.y_range = {0,0};
			jd.submit = false;
			return jd;
		}
		else
		{
			printf("returning real job\n");
			job_desc jd = jobs.front();
			jobs.pop();
			jd.submit = true;
			return jd;
		}
	}
};

void thread_pool::wait_for_null()
{
	while(true)
	{
		bool all_null = true;
		for(int32_t i = 0; i < workers.size(); i++)
			if(workers.at(i) != nullptr)
				all_null = false;

		if(all_null)
		{
			break;
		}
	}
};

void thread_pool::stop()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		done = true;
	}
	mutex_condition.notify_all();

	//for(int32_t i = 0; i < workers.size(); i++)
	//	workers.at(i)->kill();

	for(std::thread &active_thread : threads)
		active_thread.join();

	threads.clear();
};

void thread_pool::thread_loop_scalar(worker* w)
{
	while(true)
	{
		job_desc jd;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			mutex_condition.wait(lock,[this]{return !jobs.empty() || done;});

			if(done || jobs.empty())
			{
				delete w; w = nullptr;
				return;
			}

			jd = jobs.front();
			jobs.pop();
		}
		
		w->render(jd);
	}
};

void thread_pool::thread_loop_packet(worker* w)
{
	while(true)
	{
		job_desc jd;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			mutex_condition.wait(lock,[this]{return !jobs.empty() || done;});

			if(done || jobs.empty())
			{
				printf("kill\n");
				delete w; w = nullptr;
				return;
			}

			jd = jobs.front();
			jobs.pop();
		}

		w->render_bundle(jd);
	}
};

void thread_pool::thread_loop_dynamic(worker* w)
{
	w->render_dynamic();
	delete w; w = nullptr;
};