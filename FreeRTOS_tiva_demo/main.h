/*
 * main.h
 *
 *
 *  Created on: Feb 21, 2018
 *      Author: kayla
 */


#ifndef MAIN_H_
#define MAIN_H_

#define ADC_AND_FP
#define BLUETOOTH
// System clock rate, 120 MHz
#define SYSTEM_CLOCK    120000000U
#define FiveSecFreq      (SYSTEM_CLOCK * 5)
#define ONE_MS           (SYSTEM_CLOCK/1000)
//#define LESS_MS           (SYSTEM_CLOCK/5000)
#define dispMENUBit     (2UL << 2UL)

#define Vref            3.0
#define RESOLUTION      4096.0
#define SENSE           0.300
#define Vmax            1.5

#endif /* MAIN_H_ */
