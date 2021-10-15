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
#include "etl/vector.h"
#include "etl/mean.h"
#include "etl/circular_buffer.h"
#include "ox_data_structure.hpp"
#include <math.h>

namespace algo::utils{

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

		for (size_t i=SIGNAL_SIZE - WINDOW_SIZE; i < SIGNAL_SIZE; i++){
			signal[i] = signal[SIGNAL_SIZE - WINDOW_SIZE - 1];
		};
	}

	template<typename T, size_t SIGNAL_SIZE>
	void gradient(etl::array<T, SIGNAL_SIZE>& signal, const etl::array<T, SIGNAL_SIZE>& signal_time){
		static const constexpr uint32_t MAX_GRAD_VAL = 12000;
		for (size_t i{0}; i < SIGNAL_SIZE - 1; i++){
			float t_diff_sec = signal_time[i+1] - signal_time[i];
			t_diff_sec /= 1000;
			const auto v_diff = signal[i+1] - signal[i];
			const auto sig = static_cast<T>(v_diff / t_diff_sec);
			signal[i] =  abs(sig) < MAX_GRAD_VAL ? sig : 0;
		}
	}

	template<typename T, size_t SIGNAL_SIZE>
	T welfords_algorithm(const etl::array<T, SIGNAL_SIZE>& signal){
		float mean{0}, M2{0};
		T variance{0};
		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			const auto sample = signal[i];
			float delta = sample - mean;
			mean = mean + delta / (i + 1);
			M2 = M2 + delta * (sample - mean);
		}
		variance = static_cast<T>(M2 / SIGNAL_SIZE);
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
	float hr_calculator(const etl::array<T, SIGNAL_SIZE>& signal, const etl::array<T, SIGNAL_SIZE>& signal_time, const T& std_dev){
		etl::vector<float, 30> true_peak_times{};
		float temp[30]{};
		uint32_t peak_time{0}, samples_in_peak{0};

		for (size_t i{0}; i < SIGNAL_SIZE; i++){
			if (signal[i] < - std_dev){
				peak_time += signal_time[i];
				samples_in_peak++;
			} else if (samples_in_peak != 0){
				if (samples_in_peak > 4) true_peak_times.push_back((peak_time / samples_in_peak) / 1000);
				peak_time = 0;
				samples_in_peak = 0;
			}
		}

		if (true_peak_times.size() <= 1) return 0.0;

	    for (int i{0}; i < true_peak_times.size(); i++){
	    	temp[i] = true_peak_times[i];
	    }

	    for(auto it = true_peak_times.begin(); it != true_peak_times.end() - 1; it++ ){
	    	*it = *(it+1) - *it;
	    }
	    true_peak_times.pop_back();

	    for (int i{0}; i < true_peak_times.size(); i++){
	    	temp[i] = true_peak_times[i];
	    }

	    const auto mean = etl::mean<float>(true_peak_times.begin(), true_peak_times.end()).get_mean();
		return mean == 0.0 ? 0.0 : 60.0 / mean;
	}
}

#endif /* INC_MAX30102_HEARTRATE_H_ */
