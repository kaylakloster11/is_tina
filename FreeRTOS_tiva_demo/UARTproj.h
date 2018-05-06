/*
 * UARTproj.h
 *
 *  Created on: Mar 12, 2018
 *      Author: kaylak
 */

#ifndef UARTPROJ_H_
#define UARTPROJ_H_


uint32_t ClockSRCFreq;

#define ADCFIFO_MASK    0x0FFF
uint32_t adcBuffer[3];

void UART_CONFIG(void);

#ifdef BLUETOOTH

void UART_send_byteA4(uint8_t data);
void UART_send_A4(uint8_t * data, uint32_t length);
#endif

#ifdef UART_0

void UART_send_byteA0(uint8_t data);
void UART_send_A0(uint8_t * data, uint32_t length);
#endif

#ifdef ADC_AND_FP
    #define TICKS   (200)
    #define TIME_MS (0.2)
#endif

void UART_send_byteA6(uint8_t data);
void UART_send_A6(const uint8_t * data, uint32_t length);
void UART6_IRQHandler(void);
void UART0_IRQHandler(void);
void UART4_IRQHandler(void);

void itoa(int n, char s[]);
void reverse(char s[]);
void FloatReverse(char *str, int len);
int intToStr(int x, char str[], int d, uint8_t isNeg);
void ftoa(float n, char *res, int afterpoint);

void MENU_DISPLAY(void *pvParameter);

void TIMERA0_CONFIG(void);
void TIMERA2_CONFIG(void);
void TimerA_0_IRQHandler();
void TimerA_2_IRQHandler();

#endif /* UARTPROJ_H_ */
