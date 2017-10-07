#pragma once

#include <algorithm>
#include <atomic>

#include <string>
#include <vector>

#include <atomic>
#include <thread>
#include <future>

template <class F>
void par_for(int begin, int end, F fn, int num_cpus = 0) {
	std::atomic<int> idx;
	idx = begin;

	// Adding manual option to modify the number of threads
	if (num_cpus == 0)
		num_cpus = std::thread::hardware_concurrency();

	std::vector<std::future<void>> futures(num_cpus);

	for (int cpu = 0; cpu != num_cpus; ++cpu) {
		futures[cpu] = std::async(
			std::launch::async,
			[cpu, &idx, end, &fn]() {
			for (;;) {
				int i = idx++;
				if (i >= end) break;
				fn(i, cpu);
			}
		}
		);
	}
	for (int cpu = 0; cpu != num_cpus; ++cpu) {
		futures[cpu].get();
	}
}