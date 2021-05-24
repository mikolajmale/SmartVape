/*
 * MyClass.cpp
 *
 *  Created on: May 18, 2021
 *      Author: maskopol
 */

#include <MyClass.h>

MyClass::MyClass() {
	// TODO Auto-generated constructor stub
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

}

MyClass::~MyClass() {
	// TODO Auto-generated destructor stub
}

