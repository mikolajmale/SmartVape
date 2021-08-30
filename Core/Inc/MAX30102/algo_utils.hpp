/*
 * algo_utils.h
 *
 *  Created on: Jul 12, 2021
 *      Author: maskopol
 */

#ifndef INC_ALGO_UTILS_H_
#define INC_ALGO_UTILS_H_

#include <stdint.h>
#include "etl/array.h"
#include "etl/circular_buffer.h"
#include "ox_data_structure.hpp"
#include <math.h>

namespace algo::utils{

	static etl::array<uint32_t, 20> true_peak_times{};

	template <typename T, const size_t WINDOW_SIZE, const size_t SIGNAL_SIZE>
	void convolution(const etl::array<T, WINDOW_SIZE>& smoothing_window, etl::array<T, SIGNAL_SIZE>& signal){

		for (size_t i=0; i < SIGNAL_SIZE - WINDOW_SIZE; i++){
			for (size_t j = 0; j < WINDOW_SIZE; j++){
				if (j == 0){
					signal[i] = signal[i + j] * smoothing_window[WINDOW_SIZE - 1 - j];
				} else {
					signal[i] = signal[i] + signal[i + j] * smoothing_window[WINDOW_SIZE - 1 - j];
				}
			}
			signal[i] = signal[i] / WINDOW_SIZE;
		};
	}

	template<typename T, size_t SIGNAL_SIZE>
	void gradient(etl::array<T, SIGNAL_SIZE>& signal, etl::array<uint32_t, SIGNAL_SIZE>& signal_time){
		for (size_t i{0}; i < SIGNAL_SIZE - 1; i++){
			signal[i] = (signal[i+1] - signal[i]) / (signal_time[i+1] - signal_time[i]);
		}
	}

	template<typename T, size_t SIGNAL_SIZE>
	T welfords_algorithm(const etl::array<T, SIGNAL_SIZE>& signal){
		T mean{0}, M2{0}, variance{0};
		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			auto sample = signal[i];
			auto delta = sample - mean;
			mean = mean + delta / (i + 1);
			M2 = M2 + delta * (sample - mean);
		}
		variance = M2 / SIGNAL_SIZE;
		return sqrt(variance);
	}

	template<typename T, size_t SIZE>
	float mean(const etl::array<T, SIZE>& data, const uint32_t exclude_from_end = 0){
		uint32_t sum{0};
		const uint32_t size{SIZE - exclude_from_end};

		for (uint32_t i{0}; i < size; i++){
			sum += data[i];
		}
		return sum / size;
	}

	template<typename T, size_t SIZE>
	void diff(etl::array<T, SIZE>& data){
		for (uint32_t i{0}; i < SIZE - 1; i++){
			data[i] = data[i+1] - data[i];
		}
	}

	template<typename T, size_t SIGNAL_SIZE>
	float hr_calculator(const etl::array<T, SIGNAL_SIZE>& signal, const etl::array<uint32_t, SIGNAL_SIZE>& signal_time, const T& std_dev){
		uint32_t peak_time{0}, samples_in_peak{0};
		uint8_t peak_ctr{0};
		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			if (signal[i] < - std_dev){
				peak_time += signal_time[i];
				samples_in_peak++;
			} else if (samples_in_peak != 0){
				true_peak_times[peak_ctr++] = peak_time / samples_in_peak;
				peak_time = 0;
				samples_in_peak = 0;
			}
		}

		diff(true_peak_times);
		return 60 / mean(true_peak_times, 1);
	}
}

#endif /* INC_MAX30102_HEARTRATE_H_ */
