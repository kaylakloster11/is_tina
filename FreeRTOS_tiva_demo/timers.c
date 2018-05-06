/*
 * timers.c
 *
 *  Created on: Mar 13, 2018
 *      Author: kaylak
 */

#include "main.h"
#include "circbuf.h"
#include "FPS.h"
#include "adc.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "UARTproj.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"

extern EventGroupHandle_t xADCEvent;
extern TaskHandle_t xADC_Timed;

#ifdef ADC_AND_FP
   extern int Xadc[BUFFER];
#endif

int adc_i = 0;

volatile int Xval_adc = 0;


volatile int countA0 = 0;

void TIMERA0_CONFIG(void){
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ClockSRCFreq);  /* Set load value to the clock source frequency so we will interrupt every 1 second*/
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT /*TIMERA match interrupt*/);

}

 /* void TIMERA1_CONFIG(void){
     ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
     ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
     ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, SYSTEM_CLOCK);
     ROM_IntEnable(INT_TIMER1A);
     ROM_TimerIntEnable(TIMER1_BASE, (TIMER_TIMA_TIMEOUT | TIMER_CAPA_MATCH));
   } */

void TIMERA2_CONFIG(void){
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, ONE_MS);
    ROM_IntEnable(INT_TIMER2A);
    ROM_TimerIntEnable(TIMER2_BASE, (TIMER_TIMA_TIMEOUT | TIMER_CAPA_MATCH) /*TIMERA match interrupt*/);
 }

void TimerA_0_IRQHandler() {
    ROM_TimerIntClear(TIMER0_BASE, (TIMER_TIMA_TIMEOUT | TIMER_CAPA_MATCH));
    ROM_IntMasterDisable();
#ifdef UART_0
    UART_send_A0("Please press finger\n", 20);
#endif

#ifdef BLUETOOTH
    UART_send_A4("Please press finger\n", 20);
#endif
    ROM_IntMasterEnable();
}

/*void TimerA_1_IRQHandler() {
    ROM_TimerIntClear(TIMER1_BASE, (TIMER_TIMA_TIMEOUT | TIMER_CAPA_MATCH));
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(xADCEventTimed, readADCBitTimed, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
} */

void TimerA_2_IRQHandler(){
    ROM_TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ADCProcessorTrigger(ADC0_BASE, 0);
        while(!ADCIntStatus(ADC0_BASE, 0, false));
        ADCSequenceDataGet(ADC0_BASE, 0, &adcBuffer);
        countA0++;

    #ifdef ADC_AND_FP
        Xadc[adc_i] = (adcBuffer[0] & ADCFIFO_MASK);
        adc_i++;

        if(adc_i >= BUFFER){
            adc_i = 0;
        }

    if(countA0 >= TICKS){
       vTaskNotifyGiveFromISR(xADC_Timed, &xHigherPriorityTaskWoken);
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    #endif

    #ifdef ADC_ONLY
        Xval_adc += (adcBuffer[0] & ADCFIFO_MASK);
        if(countA0 == 100){
                    xEventGroupSetBitsFromISR(xADCEvent, readADCBit, &xHigherPriorityTaskWoken);
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                }
    #endif

 }


