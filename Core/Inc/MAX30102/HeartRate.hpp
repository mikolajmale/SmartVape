/*
 * HearRate.h
 *
 *  Created on: Jul 12, 2021
 *      Author: maskopol
 */

#ifndef INC_MAX30102_HEARTRATE_H_
#define INC_MAX30102_HEARTRATE_H_

#include <stdint.h>
#include "ox_data_structure.hpp"
#include "algo_utils.hpp"

template <size_t SIGNAL_SIZE>
class HeartRate {
public:
	HeartRate() : _heart_rate{0} {};
	virtual ~HeartRate(){
		_smoothing_window.fill({0, 1, 1});
	};

	void process(OxWriteData& signal){
		algo::utils::convolution(_smoothing_window, signal);
		algo::utils::gradient(signal);
	};

	uint32_t get_hr(void) {return _heart_rate;};

private:

	uint32_t _heart_rate;
	static size_t const constexpr SMOOTHING_SIZE = 20;
	etl::array<TimestampedOxSample, SMOOTHING_SIZE> _smoothing_window;
};

#endif /* INC_MAX30102_HEARTRATE_H_ */
