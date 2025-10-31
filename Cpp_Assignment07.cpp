#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <future>
#include <thread>

#define PRIME_START 100*1000*1000
#define PRIME_END 200*1000*1000

/**
 * a. What was the most difficult aspect to grasp regarding the topic of threaded pools?
 *      During lecture, the difficult part for me to understand was the future. Reading through the documentation
 *      and giving it an attempt, that part of the project actually ended up fairly easy.
 * 
 *      During the lab implementation, I think the hardest part of the lab was the multi-threading. I'm trying
 *      wrapping my head around multi-threading, and more crucially the syntax and best practices. In the destructor,
 *      I forgot to call the notify_all to the other threads and I also forgot to wrap mutex in a separate closure.
 * 
 * b. What improvements to the design of the ThreadedPool would you consider if you were implementing one for your game?
 *      The first improvement would be to make sure that the tasks being given to the ThreadedPool were tasks
 *      worth multithreading. It was unfortunate that the enqueue function was actually slower than the is_prime function.
 * 
 *      Another idea I had was that you could implement some sort of priority queue. So instead of first-in, first-out,
 *      tasks would be removed by order of importance.
 * 
 * Time taken in single thread: 20217
 * Found primes using thread pool: 5317482
 * Time taken in thread pool: 181308
 * Destroyed thread pool successfully
 * Press ENTER to exit...
 * 
 */


struct ThreadPool {
	ThreadPool(std::size_t numThreads) : stopFlag(false) {
        threads.reserve(numThreads);
        tasks = std::queue<std::function<void()>>();
	
        for(std::size_t t = 0; t < numThreads; ++t) {
            threads.emplace_back([this]() {
                workerThread();
            });
        }
	}

	~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }

        condition.notify_all();

        for (int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
	}

	template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        {
            std::lock_guard lock(this->queueMutex);
            if(this->stopFlag) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            this->tasks.emplace([task]() {
                (*task)();
            });
        }

        this->condition.notify_one();
        return task->get_future();
	}
    
	protected:
		// May be modified if you decide to use std::stop_token for your threads
        void workerThread() {
			while(true) {
                std::function<void()> task;
                
                {
                    // Acquires mutex
                    std::unique_lock lock(this->queueMutex);

                    // Checks condition while holding the lock, if condition is not satisfied, releases lock and waits
                    condition.wait(lock, [this]() {
                        return stopFlag || !tasks.empty();
                    });

                    // Locks gets reacquired each time condition perform its check after a notify
                    if (stopFlag && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();		// Executes task!
            }
		}

    private:
        std::queue<std::function<void()>> tasks;
        std::vector<std::jthread> threads;
        bool stopFlag;
        std::condition_variable condition;
        std::mutex queueMutex;
};

bool is_prime(std::size_t n) {
	if ((n % 2 == 0) && (n != 2)) { 
        return false;
    }

	std::size_t upper = sqrt(static_cast<double>(n));

	for (std::size_t i = 3; i <= upper; i += 2) {
		if ((n % i) == 0) {
            return false;
        }
	}

	return true;
}

int main() {
	std::size_t primeCountSingle = 0, primeCountPool = 0, threadCount = 4;
    auto start = std::chrono::high_resolution_clock::now();

    for(std::size_t i = PRIME_START; i < PRIME_END; ++i) {
		if (is_prime(i)) {
            ++primeCountSingle;
        }
	}

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Found primes in single thread: " << primeCountSingle << "\nTime taken in single thread: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	
    {
		start = std::chrono::high_resolution_clock::now();
	    // TODO: Adds tasks to queue such that it scans all numbers in the range [PRIME_START, PRIME_END] using your ThreadPool implementation
	    // Count the total number of prime numbers present in the range, it should match the single threaded execution
	    // NOTE: You may either use a reference to primeCountPool or may receive the std::futures of your calls to handle the count sequentially
	    // If you decide to use reference, remember the risk of race conditions between threads
	    ThreadPool tp(threadCount);
	    std::vector<std::future<bool>> results;
        results.reserve(PRIME_END - PRIME_START);

        for(std::size_t i = PRIME_START; i < PRIME_END; ++i) {
            results.emplace_back(tp.enqueue(is_prime, i));
        }

	    for(auto& future : results) {
            if (future.get()) {
                ++primeCountPool;
            }
        }
	    
	    end = std::chrono::high_resolution_clock::now();
	    std::cout << "Found primes using thread pool: " << primeCountPool << "\nTime taken in thread pool: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}

	std::cout << "Destroyed thread pool successfully" << std::endl;
    std::cout << "Press ENTER to exit...";
    fgetc(stdin);

    return 0;
}