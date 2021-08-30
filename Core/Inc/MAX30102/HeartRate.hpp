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

class HeartRate {
public:
	HeartRate() : _heart_rate{0} {};
	virtual ~HeartRate(){};

	void process(OxStream& signal){
		algo::utils::convolution(_smoothing_window, signal.get_ir());
		algo::utils::gradient(signal.get_ir(), signal.get_time());
		auto std_dev = algo::utils::welfords_algorithm(signal.get_ir());

		_heart_rate = algo::utils::hr_calculator(signal.get_ir(), signal.get_time(), std_dev);
	};

	uint32_t get_hr(void) {return _heart_rate;};

private:

	uint32_t _heart_rate;
	static size_t const constexpr SMOOTHING_SIZE = 20;
	const etl::array<uint32_t, SMOOTHING_SIZE> _smoothing_window{};
};

#endif /* INC_MAX30102_HEARTRATE_H_ */
