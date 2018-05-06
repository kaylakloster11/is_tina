/*
 * ADC.h
 *
 *  Created on: Apr 10, 2018
 *      Author: kayla
 */

#ifndef ADC_H_
#define ADC_H_

#define readADCBit      (1UL << 1UL)
#define calibADCBit     (2UL << 1UL)
#define readADCBitTimed (3UL << 1UL)

extern volatile int Xval_adc, Yval_adc, Zval_adc;
extern volatile int countA0;

void READ_ADC(void *pvParameter);
void READ_ADCTimed(void *pvParameter);
void CALIBRATE_ADC(void *pvParameter);
void CALIB_ONE(void *pvParameter);
void ADC_CONFIG(void);

#define Vref            3.0
#define RESOLUTION      4096.0
#define SENSE           0.300
#define Vmax            1.5

typedef struct {
    volatile float Vx;
    volatile float Ax;
} ADCBuf_t;

#endif /* ADC_H_ */
