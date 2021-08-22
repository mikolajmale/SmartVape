/*
 * HearRate.h
 *
 *  Created on: Jul 12, 2021
 *      Author: maskopol
 */

#ifndef INC_MAX30102_HEARTRATE_H_
#define INC_MAX30102_HEARTRATE_H_

#include <stdint.h>

template <typename T>
class HeartRate {
public:
	HeartRate() : _heart_rate{0} {};
	virtual ~HeartRate(){};

	void process(const T&){
		_heart_rate = 0;
	};

	uint32_t get_hr(void) {return _heart_rate;};

private:
	uint32_t _heart_rate;
};

#endif /* INC_MAX30102_HEARTRATE_H_ */
