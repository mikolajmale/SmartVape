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

using OxReadData =  etl::circular_buffer<TimestampedOxSample, MAX30102_BUFFER_LENGTH>;
using OxWriteData =  etl::array<TimestampedOxSample, MAX30102_BUFFER_LENGTH>;


#endif /* INC_MAX30102_OX_DATA_STRUCTURE_HPP_ */
