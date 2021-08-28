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

namespace algo::utils{

	template <typename T=OxSample, const size_t WINDOW_SIZE, const size_t SIGNAL_SIZE>
	void convolution(const etl::array<T, WINDOW_SIZE>& smoothing_window,
									etl::array<T, SIGNAL_SIZE>& signal){

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

	template<typename T=OxSample, size_t SIGNAL_SIZE>
	void gradient(etl::array<T, SIGNAL_SIZE>& signal){
		for (size_t i{0}; i < SIGNAL_SIZE - 1; i++){
			signal[i] = (signal[i+1] - signal[i]) / (signal[i+1].ts - signal[i].ts);
		}
	}
}

#endif /* INC_MAX30102_HEARTRATE_H_ */
