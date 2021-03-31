#ifndef CPP_DESIGNO_ENGINE_HPP_THREADPOOL
#define CPP_DESIGNO_ENGINE_HPP_THREADPOOL

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

class ThreadPool
{
	std::vector<std::thread> threads;

	std::condition_variable condition;

	//controlled by queue_mutex
	std::queue<std::function<void()>> func_queue;
	
	//controlled by queue_mutex
	std::atomic_bool terminate_pool;

	std::mutex queue_mutex;

	void thread_func()
	{
		//the job we execute
		std::function<void()> job_func;
		while (true)
		{
			job_func = nullptr;
			{
				std::unique_lock<std::mutex> lock{ queue_mutex };

				//wait for availablility of queue
				condition.wait(lock, [this]() { return !func_queue.empty() || terminate_pool.load(); });

				//if there are no more jobs to execute and we're told to quit, quit
				if (func_queue.empty() && terminate_pool.load())
				{
					break;
				}

				//get from queue, and remove from queue
				job_func = func_queue.front();
				func_queue.pop();
			}
			//execute the job we got from the queue, if it exists
			if (job_func != nullptr)
			{
				job_func();
			}
		}
	}

public:
	ThreadPool()
	{
		//don't immediatly terminate all threads
		terminate_pool.store(false);

		//allocate threads, start them off
		for (size_t i = 0; i < (std::thread::hardware_concurrency() - 1); i++)
		{
			threads.push_back(std::thread{ &ThreadPool::thread_func, this });
		}
	}

	~ThreadPool()
	{
		//call for threads to finish once queue is empty
		terminate_pool.store(true);

		condition.notify_all();

		//join all the threads
		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	void add_work(const std::function<void()>& work)
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			func_queue.push(work);
		}
		condition.notify_all();
	}
};

#endif