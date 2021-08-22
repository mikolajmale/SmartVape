/** \file max30102.cpp ******************************************************
*
* Project: MAXREFDES117#
* Filename: max30102.cpp
* Description: This module is an embedded controller driver for the MAX30102
*
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*  Modified original MAXIM source code on: 13.01.2019
*		Author: Mateusz Salamon
*		www.msalamon.pl
*		mateusz@msalamon.pl
*	Code is modified to work with STM32 HAL libraries.
*
*	Website: https://msalamon.pl/palec-mi-pulsuje-pulsometr-max30102-pod-kontrola-stm32/
*	GitHub:  https://github.com/lamik/MAX30102_STM32_HAL
*
*/
#include "main.h"
#include "i2c.h"
#include "FreeRTOS.h"
#include "task.h"

#include "MAX30102/MAX30102.h"
#include "etl/circular_buffer.h"
//#include "MAX30102/algorithm.h"

#define I2C_TIMEOUT	100

I2C_HandleTypeDef *i2c_max30102;

template <typename T>
struct OxPair{
	T ir;
	T red;
};

template<typename T, size_t MAX_SIZE>
using OxData =  etl::circular_buffer<OxPair<T>, MAX_SIZE>;

OxData<uint32_t, MAX30102_BUFFER_LENGTH> data{};

volatile uint32_t CollectedSamples{0};
volatile uint8_t IsFingerOnScreen{0};
int32_t Sp02Value;
int8_t Sp02IsValid;
int32_t HeartRate;
int8_t IsHrValid;

typedef enum
{
	MAX30102_STATE_BEGIN,
	MAX30102_STATE_CALIBRATE,
	MAX30102_STATE_CALCULATE_HR,
	MAX30102_STATE_COLLECT_NEXT_PORTION
}MAX30102_STATE;

MAX30102_STATE StateMachine;

MAX30102_STATUS Max30102_WriteReg(uint8_t uch_addr, uint8_t uch_data)
{	auto const status = HAL_I2C_Mem_Write(i2c_max30102, MAX30102_ADDRESS, uch_addr, 1, &uch_data, 1, I2C_TIMEOUT);
	if(status == HAL_OK)
		return MAX30102_OK;
	return MAX30102_ERROR;
}

MAX30102_STATUS Max30102_ReadReg(uint8_t uch_addr, uint8_t *puch_data)
{
	if(HAL_I2C_Mem_Read(i2c_max30102, MAX30102_ADDRESS, uch_addr, 1, puch_data, 1, I2C_TIMEOUT) == HAL_OK)
		return MAX30102_OK;
	return MAX30102_ERROR;
}

MAX30102_STATUS Max30102_WriteRegisterBit(uint8_t Register, uint8_t Bit, uint8_t Value)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(Register, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(1<<Bit);
	tmp |= (Value&0x01)<<Bit;
	if(MAX30102_OK != Max30102_WriteReg(Register, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}

//
//	Interrupts
//
MAX30102_STATUS Max30102_SetIntAlmostFullEnabled(uint8_t Enable)
{
	return Max30102_WriteRegisterBit(REG_INTR_ENABLE_1, INT_A_FULL_BIT, Enable);
}

MAX30102_STATUS Max30102_SetIntFifoDataReadyEnabled(uint8_t Enable)
{

	return Max30102_WriteRegisterBit(REG_INTR_ENABLE_1, INT_PPG_RDY_BIT, Enable);
}

MAX30102_STATUS Max30102_SetIntAmbientLightCancelationOvfEnabled(uint8_t Enable)
{

	return Max30102_WriteRegisterBit(REG_INTR_ENABLE_2, INT_ALC_OVF_BIT, Enable);
}
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
MAX30102_STATUS Max30102_SetIntInternalTemperatureReadyEnabled(uint8_t Enable)
{

	return Max30102_WriteRegisterBit(REG_INTR_ENABLE_2, INT_DIE_TEMP_RDY_BIT, Enable);
}
#endif

//
//	FIFO Configuration
//
MAX30102_STATUS Max30102_FifoWritePointer(uint8_t Address)
{
	if(MAX30102_OK != Max30102_WriteReg(REG_FIFO_WR_PTR,(Address & 0x1F)))  //FIFO_WR_PTR[4:0]
			return MAX30102_ERROR;
	return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoOverflowCounter(uint8_t Address)
{
	if(MAX30102_OK != Max30102_WriteReg(REG_OVF_COUNTER,(Address & 0x1F)))  //OVF_COUNTER[4:0]
			return MAX30102_ERROR;
	return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoReadPointer(uint8_t Address)
{
	if(MAX30102_OK != Max30102_WriteReg(REG_FIFO_RD_PTR,(Address & 0x1F)))  //FIFO_RD_PTR[4:0]
			return MAX30102_ERROR;
	return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoSampleAveraging(uint8_t Value)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_FIFO_CONFIG, &tmp))
		return MAX30102_ERROR;
	// again magic
	tmp &= ~(0x07);
	tmp |= (Value&0x07)<<5;
	if(MAX30102_OK != Max30102_WriteReg(REG_FIFO_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}

MAX30102_STATUS Max30102_FifoRolloverEnable(uint8_t Enable)
{
	return Max30102_WriteRegisterBit(REG_FIFO_CONFIG, FIFO_CONF_FIFO_ROLLOVER_EN_BIT, (Enable & 0x01));
}

MAX30102_STATUS Max30102_FifoAlmostFullValue(uint8_t Value)
{
	// wtf
	if(Value < 17) Value = 17;
	if(Value > 32) Value = 32;
	Value = 32 - Value;
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_FIFO_CONFIG, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(0x0F);
	tmp |= (Value & 0x0F);
	if(MAX30102_OK != Max30102_WriteReg(REG_FIFO_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}
//
//	Mode Configuration
//
MAX30102_STATUS Max30102_ShutdownMode(uint8_t Enable)
{
	return Max30102_WriteRegisterBit(REG_MODE_CONFIG, MODE_SHDN_BIT, (Enable & 0x01));
}

MAX30102_STATUS Max30102_Reset(void)
{
	uint8_t tmp = 0xFF;
    if(MAX30102_OK != Max30102_WriteReg(REG_MODE_CONFIG,0x40))
        return MAX30102_ERROR;
    do
    {
    	if(MAX30102_OK != Max30102_ReadReg(REG_MODE_CONFIG, &tmp))
    		return MAX30102_ERROR;
    } while(tmp & (1<<6));

    return MAX30102_OK;
}

MAX30102_STATUS Max30102_SetMode(uint8_t Mode)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_MODE_CONFIG, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(0x07);
	tmp |= (Mode & 0x07);
	if(MAX30102_OK != Max30102_WriteReg(REG_MODE_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}
//
//	SpO2 Configuration
//
MAX30102_STATUS Max30102_SpO2AdcRange(uint8_t Value)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(0x03);
	tmp |= ((Value & 0x03) << 5);
	if(MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}

MAX30102_STATUS Max30102_SpO2SampleRate(uint8_t Value)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(0x07);
	tmp |= ((Value & 0x07) << 2);
	if(MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}

MAX30102_STATUS Max30102_SpO2LedPulseWidth(uint8_t Value)
{
	uint8_t tmp;
	if(MAX30102_OK != Max30102_ReadReg(REG_SPO2_CONFIG, &tmp))
		return MAX30102_ERROR;
	tmp &= ~(0x03);
	tmp |= (Value & 0x03);
	if(MAX30102_OK != Max30102_WriteReg(REG_SPO2_CONFIG, tmp))
		return MAX30102_ERROR;

	return MAX30102_OK;
}

//
//	LEDs Pulse Amplitute Configuration
//	LED Current = Value * 0.2 mA
//
MAX30102_STATUS Max30102_Led1PulseAmplitude(uint8_t Value)
{
	if(MAX30102_OK != Max30102_WriteReg(REG_LED1_PA, Value))
		return MAX30102_ERROR;
	return MAX30102_OK;
}

MAX30102_STATUS Max30102_Led2PulseAmplitude(uint8_t Value)
{
	if(MAX30102_OK != Max30102_WriteReg(REG_LED2_PA, Value))
		return MAX30102_ERROR;
	return MAX30102_OK;
}

//
//	Usage functions
//
MAX30102_STATUS Max30102_IsFingerOnSensor(void)
{
	return static_cast<MAX30102_STATUS>(IsFingerOnScreen);
}

int32_t Max30102_GetHeartRate(void)
{
	return HeartRate;
}

int32_t Max30102_GetSpO2Value(void)
{
	return Sp02Value;
}

void led_low_startover(){
	Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW);
	Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW);
	StateMachine = MAX30102_STATE_BEGIN;
}

void Max30102_Task(void)
{
	switch(StateMachine)
	{
		case MAX30102_STATE_BEGIN:
			HeartRate = 0;
			Sp02Value = 0;
			if(IsFingerOnScreen)
			{
				CollectedSamples = 0;
				Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_HIGH);
				Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_HIGH);
				StateMachine = MAX30102_STATE_CALIBRATE;
			}
		break;

		case MAX30102_STATE_CALIBRATE:
				if(IsFingerOnScreen)
				{
					if(CollectedSamples > (MAX30102_BUFFER_LENGTH-MAX30102_SAMPLES_PER_SECOND))
					{
						StateMachine = MAX30102_STATE_CALCULATE_HR;
					}
				}
				else led_low_startover();

		break;

		case MAX30102_STATE_CALCULATE_HR:
			if(IsFingerOnScreen)
			{
//				taskDISABLE_INTERRUPTS();
//				maxim_heart_rate_and_oxygen_saturation(IrBuffer, RedBuffer, MAX30102_BUFFER_LENGTH - MAX30102_SAMPLES_PER_SECOND, BufferTail, &Sp02Value, &Sp02IsValid, &HeartRate, &IsHrValid);
//				taskENABLE_INTERRUPTS();
				CollectedSamples = 0;
				StateMachine = MAX30102_STATE_COLLECT_NEXT_PORTION;
			}
			else led_low_startover();

		break;

		case MAX30102_STATE_COLLECT_NEXT_PORTION:
			if(IsFingerOnScreen)
			{
				if(CollectedSamples > MAX30102_SAMPLES_PER_SECOND)
				{
					StateMachine = MAX30102_STATE_CALCULATE_HR;
				}
			}
			else led_low_startover();

		break;
	}
}

MAX30102_STATUS Max30102_ReadFifo()
{
	uint32_t un_temp, temp_ir, temp_red;
	uint8_t ach_i2c_data[6];

	if(HAL_I2C_Mem_Read(i2c_max30102, MAX30102_ADDRESS, REG_FIFO_DATA, 1, ach_i2c_data, 6, I2C_TIMEOUT) != HAL_OK)
	{
		return MAX30102_ERROR;
	}
	un_temp=(unsigned char) ach_i2c_data[0];
	un_temp<<=16;
	temp_red=un_temp;
	un_temp=(unsigned char) ach_i2c_data[1];
	un_temp<<=8;
	temp_red+=un_temp;
	un_temp=(unsigned char) ach_i2c_data[2];
	temp_red+=un_temp;

	un_temp=(unsigned char) ach_i2c_data[3];
	un_temp<<=16;
	temp_ir=un_temp;
	un_temp=(unsigned char) ach_i2c_data[4];
	un_temp<<=8;
	temp_ir+=un_temp;
	un_temp=(unsigned char) ach_i2c_data[5];
	temp_ir+=un_temp;

	temp_red&=0x03FFFF;  //Mask MSB [23:18]
	temp_ir&=0x03FFFF;  //Mask MSB [23:18]

	data.push({temp_ir, temp_red});

	return MAX30102_OK;
}

MAX30102_STATUS Max30102_ReadInterruptStatus(uint8_t *Status)
{
	uint8_t tmp;
	*Status = 0;

	if(MAX30102_OK != Max30102_ReadReg(REG_INTR_STATUS_1, &tmp))
		return MAX30102_ERROR;
	*Status |= tmp & 0xE1; // 3 highest bits
#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
	if(MAX30102_OK != Max30102_ReadReg(REG_INTR_STATUS_2, &tmp))
		return MAX30102_ERROR;
	*Status |= tmp & 0x02;
#endif
	return MAX30102_OK;
}

void collect_fifo() {
	while(MAX30102_OK != Max30102_ReadFifo()); // read 2 words
	const auto ir = data.back().ir;
	if(IsFingerOnScreen)
	{
		if(ir < MAX30102_IR_VALUE_FINGER_OUT_SENSOR) IsFingerOnScreen = 0;
	}
	else
	{
		if(ir > MAX30102_IR_VALUE_FINGER_ON_SENSOR) IsFingerOnScreen = 1;
	}

	CollectedSamples++;
}

void Max30102_InterruptCallback(void)
{
	uint8_t Status;
	// TODO: omin bledna paczke
	while(MAX30102_OK != Max30102_ReadInterruptStatus(&Status));

	// Almost Full FIFO Interrupt handle
	if(Status & (1<<INT_A_FULL_BIT))
	{
		for(uint8_t i = 0; i < MAX30102_FIFO_ALMOST_FULL_SAMPLES; i++) collect_fifo();
	}

	// New FIFO Data Ready Interrupt handle
	if(Status & (1<<INT_PPG_RDY_BIT)) collect_fifo();

//	//  Ambient Light Cancellation Overflow Interrupt handle
//	if(Status & (1<<INT_ALC_OVF_BIT)){};
//
//	// Power Ready Interrupt handle
//	if(Status & (1<<INT_PWR_RDY_BIT)){};

#ifdef MAX30102_USE_INTERNAL_TEMPERATURE
	// Internal Temperature Ready Interrupt handle
	if(Status & (1<<INT_DIE_TEMP_RDY_BIT)){};
#endif
}

//
//	Initialization
//
MAX30102_STATUS Max30102_Init(I2C_HandleTypeDef *i2c)
{
	uint8_t uch_dummy;
	i2c_max30102 = i2c;
	if(MAX30102_OK != Max30102_Reset()) //resets the MAX30102
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_ReadReg(0,&uch_dummy))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoWritePointer(0x00))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoOverflowCounter(0x00))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoReadPointer(0x00))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoSampleAveraging(FIFO_SMP_AVE_1))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoRolloverEnable(0))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_FifoAlmostFullValue(MAX30102_FIFO_ALMOST_FULL_SAMPLES))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SetMode(MODE_SPO2_MODE))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SpO2AdcRange(SPO2_ADC_RGE_4096))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SpO2SampleRate(SPO2_SAMPLE_RATE))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SpO2LedPulseWidth(SPO2_PULSE_WIDTH_411))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_Led1PulseAmplitude(MAX30102_RED_LED_CURRENT_LOW))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_Led2PulseAmplitude(MAX30102_IR_LED_CURRENT_LOW))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SetIntAlmostFullEnabled(1))
		return MAX30102_ERROR;
	if(MAX30102_OK != Max30102_SetIntFifoDataReadyEnabled(1))
		return MAX30102_ERROR;
//	if(MAX30102_OK != Max30102_WriteReg(REG_PILOT_PA,0x7f))   // Choose value for ~ 25mA for Pilot LED
//		return MAX30102_ERROR;
	StateMachine = MAX30102_STATE_BEGIN;
	return MAX30102_OK;
}
