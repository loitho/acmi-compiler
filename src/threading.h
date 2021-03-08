/*
** Name: THREADING.H
** Description:
**		File to handle the multithreadded for loop
**
** History:
**		1-oct-17 
**			File creation by loitho
** 
*/

#pragma once

#include <atomic>
#include <vector>
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
			[&idx, end, &fn]() {
			for (;;) {
				int i = idx++;
				if (i >= end) break;
				fn(i);
			}
		}
		);
	}
	for (int cpu = 0; cpu != num_cpus; ++cpu) {
		futures[cpu].get();
	}
}
