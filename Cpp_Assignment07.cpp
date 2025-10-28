#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <future>
#include <thread>

#define PRIME_START 100*1000*1000
#define PRIME_END 200*1000*1000

// --------------- MODIFY

struct ThreadPool {
	ThreadPool(std::size_t numThreads) {
		// TODO
	}
	~ThreadPool() {
		// TODO
	}
	template <typename F, typename... Args> auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
		// TODO
	}
	protected:
		// May be modified if you decide to use std::stop_token for your threads
        void workerThread() {
			// TODO
		}
};

// --------------- END OF MODIFY

bool is_prime(std::size_t n) {
	if((n % 2 == 0) && (n != 2)) { return false; }
	std::size_t upper = sqrt(static_cast<double>(n));
	for(std::size_t i = 3; i <= upper; i += 2) {
		if((n % i) == 0){ return false; }
	}
	return true;
}

int main() {
	std::size_t primeCountSingle = 0, primeCountPool = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for(std::size_t i = PRIME_START; i < PRIME_END; ++i) {
		if (is_prime(i)) { ++primeCountSingle; }
	}
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Found primes in single thread: " << primeCountSingle << "\nTime taken in single thread: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	{
		start = std::chrono::high_resolution_clock::now();
	    // TODO: Adds tasks to queue such that it scans all numbers in the range [PRIME_START, PRIME_END[ using your ThreadPool implementation
	    // Count the total number of prime numbers present in the range, it should match the single threaded execution
	    // NOTE: You may either use a referene to primeCountPool or may receive the std::futures of your calls to handle the count sequentially
	    // If you decide to use reference, remember the risk of race conditions between threads
	    ThreadPool tp(4);
	    
	    
	    
	    end = std::chrono::high_resolution_clock::now();
	    std::cout << "Found primes using thread pool: " << primeCountPool << "\nTime taken in thread pool: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
	}
	std::cout << "Destroyed thread pool successfully" << std::endl;
    std::cout << "Press ENTER to exit...";
    fgetc(stdin);
    return 0;
}