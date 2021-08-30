/*
 * ox_data_structure.hpp
 *
 *  Created on: Aug 24, 2021
 *      Author: maskopol
 */

#ifndef INC_MAX30102_OX_DATA_STRUCTURE_HPP_
#define INC_MAX30102_OX_DATA_STRUCTURE_HPP_

#include "etl/circular_buffer.h"
#include "etl/array.h"

struct OxSample{
	uint32_t ir;
	uint32_t red;

	OxSample operator + (const OxSample& other){
		OxSample temp;
		temp.ir = ir + other.ir;
		temp.red = red + other.red;
		return temp;
	}

	OxSample operator - (const OxSample& other){
		OxSample temp;
		temp.ir = ir - other.ir;
		temp.red = red - other.red;
		return temp;
	}

	OxSample operator * (const OxSample& other){
		OxSample temp;
		temp.ir = ir * other.ir;
		temp.red = red * other.red;
		return temp;
	}

	OxSample operator / (const int constant){
		OxSample temp;
		temp.ir = ir / constant;
		temp.red = red / constant;
		return temp;
	}
};

struct TimestampedOxSample{
	uint32_t ts;
	uint32_t ir;
	uint32_t red;

	TimestampedOxSample operator + (const TimestampedOxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir + other.ir;
		temp.red = red + other.red;
		return temp;
	}

	TimestampedOxSample operator - (const TimestampedOxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir - other.ir;
		temp.red = red - other.red;
		return temp;
	}

	TimestampedOxSample operator * (const TimestampedOxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir * other.ir;
		temp.red = red * other.red;
		return temp;
	}

	// without time
	TimestampedOxSample operator + (const OxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir + other.ir;
		temp.red = red + other.red;
		return temp;
	}

	TimestampedOxSample operator - (const OxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir - other.ir;
		temp.red = red - other.red;
		return temp;
	}

	TimestampedOxSample operator * (const OxSample& other){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir * other.ir;
		temp.red = red * other.red;
		return temp;
	}

	TimestampedOxSample operator / (const int constant){
		TimestampedOxSample temp;
		temp.ts = ts;
		temp.ir = ir / constant;
		temp.red = red / constant;
		return temp;
	}
};

struct OxStream{
		using array = etl::array<uint32_t, MAX30102_BUFFER_LENGTH>;
		array ts;
		array ir;
		array red;

		OxStream() : _iterator{0} {}

		void clear(void){
			ts.fill(0);
			ir.fill(0);
			red.fill(0);
			_iterator = 0;
		}

		bool append(const TimestampedOxSample& sample){
			if (_iterator == MAX30102_BUFFER_LENGTH) return false;

			ts[_iterator] = sample.ts;
			ir[_iterator] = sample.ir;
			red[_iterator] = sample.red;

			_iterator++;
			return true;
		}

		array & get_time(void) { return ts; }

		array const & get_time(void) const { return ts; }

		array & get_ir(void) { return ir; }

		array const & get_ir(void) const { return ir; }

		array & get_red(void) { return red; }

		array const & get_red(void) const { return red; }


	private:
		uint16_t _iterator;

};

using OxReadData =  etl::circular_buffer<TimestampedOxSample, MAX30102_BUFFER_LENGTH>;
using OxWriteData =  etl::array<TimestampedOxSample, MAX30102_BUFFER_LENGTH>;


#endif /* INC_MAX30102_OX_DATA_STRUCTURE_HPP_ */
