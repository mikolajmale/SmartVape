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
#include "etl/standard_deviation.h"

class HeartRate {
public:
	HeartRate() : _heart_rate{0} {
		_smoothing_window.fill(1);
	};
	virtual ~HeartRate(){};

	void process(OxStream& signal){
		auto& ir = signal.get_ir();
		algo::utils::convolution(_smoothing_window, ir);
		algo::utils::gradient(ir, signal.get_time());

		etl::standard_deviation<etl::standard_deviation_type::Population, int32_t> standard_deviation(ir.begin(), ir.end());
		const int32_t std_dev = static_cast<int32_t>(standard_deviation.get_standard_deviation());
		_heart_rate = algo::utils::hr_calculator(ir, signal.get_time(), std_dev);
		int a;
	};

	uint32_t get_hr(void) {return _heart_rate;};

private:

	uint32_t _heart_rate;
	static size_t const constexpr SMOOTHING_SIZE = 20;
	etl::array<int32_t, SMOOTHING_SIZE> _smoothing_window{};
};

#endif /* INC_MAX30102_HEARTRATE_H_ */
