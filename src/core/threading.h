#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

#include "math/include.h"

class worker;
class integrator;

struct job_desc
{
	vec2i x_range;
	vec2i y_range;
	uint32_t spp;
	bool submit = true;
};

enum bundle_state
{
	NEED_INPUT = 0,
	HAS_OUTPUT
};

class thread_pool
{
	protected:
	bool done = false;
	uint32_t max_threads = 0;
	std::mutex queue_mutex;
	std::condition_variable mutex_condition;
	std::vector<std::thread> threads = {};
	std::vector<worker*> workers = {};
	std::queue<job_desc> jobs = {};

	std::vector<bundle_state> states = {};
	
	void thread_loop_scalar(worker* w); //defined in the base integrator source file!

	void thread_loop_packet(worker* w); //defined in the base integrator source file!

	void thread_loop_dynamic(worker* w); //defined in the base integrator source file!

	public:
	thread_pool() {done = false;};

	//spawn threads and add to queue
	void start(int32_t do_packet);

	void queue(const job_desc &j)
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			jobs.push(j);
		}
		mutex_condition.notify_one();
	};
	
	void stop();

	bool busy()
	{
		bool pool_busy;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			pool_busy = jobs.empty();
		}
		return pool_busy;
	};

	size_t job_size()
	{
		return jobs.size();
	};

	bool is_done() const {return done;};

	void wait_for_null();

	job_desc get_job();
};