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

	template <const size_t WINDOW_SIZE, const size_t SIGNAL_SIZE>
	void convolution(const etl::array<TimestampedOxSample, WINDOW_SIZE>& smoothing_window,
									etl::array<TimestampedOxSample, SIGNAL_SIZE>& signal){

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

	template<size_t SIGNAL_SIZE>
	void gradient(etl::array<TimestampedOxSample, SIGNAL_SIZE>& signal){
		for (size_t i{0}; i < SIGNAL_SIZE - 1; i++){
			signal[i] = (signal[i+1] - signal[i]) / (signal[i+1].ts - signal[i].ts);
		}
	}

	template<size_t SIGNAL_SIZE>
	OxSample welfords_algorithm(const etl::array<TimestampedOxSample, SIGNAL_SIZE>& signal){
		OxSample mean{0, 0}, M2{0, 0}, variance{0, 0};
		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			auto sample = OxSample{signal[i].ir, signal[i].red};
			auto delta = sample - mean;
			mean = mean + delta / (i + 1);
			M2 = M2 + delta * (sample - mean);
		}
		variance = M2 / SIGNAL_SIZE;
		return {sqrt(variance.ir), sqrt(variance.red)};
	}

	template<typename T, size_t SIZE>
	float mean(const etl::array<T, SIZE>& data){
		uint32_t sum{0};
		for (int i{0}; i < SIZE; i++){
			sum += data[i];
		}
		return sum / len(data);
	}

	template<typename T, size_t SIZE>
	void diff(etl::array<T, SIZE>& data){
		for (int i{0}; i < SIZE - 1; i++){
			data[i] = data[i+1] - data[i];
		}
	}

	template<size_t SIGNAL_SIZE>
	OxSample hr_calculator(const etl::array<TimestampedOxSample, SIGNAL_SIZE>& signal,
			const OxSample& std_dev){
		uint32_t peak_time{0}, samples_in_peak{0};
		uint8_t peak_ctr{0};
		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			if (signal[i].ir < - std_dev.ir){
				peak_time += signal[i].ts;
				samples_in_peak++;
			} else if (samples_in_peak != 0){
				true_peak_times[peak_ctr] = peak_time / samples_in_peak;
				peak_time = 0;
				samples_in_peak = 0;
			}
		}

	}
}

#endif /* INC_MAX30102_HEARTRATE_H_ */
