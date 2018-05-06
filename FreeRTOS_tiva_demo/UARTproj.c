/*
 * UARTproj.c
 *
 *  Created on: Mar 12, 2018
 *      Author: kaylak
 */


// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"


#include "FPS.h"
#include "ADC.h"
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
#include "driverlib/uart.h"
#include "UARTproj.h"


extern EventGroupHandle_t xUserInEvent;
extern EventGroupHandle_t xADCEvent;
extern TaskHandle_t xMenuHandle, xFP_Menu, xCalib_ADC;

extern volatile uint8_t FP_state;
uint8_t i;
extern volatile  uint8_t data_databuf[30];
extern volatile  uint8_t rsp_databuf[12];
extern volatile uint8_t FPS_byte_count;

void reverse(char s[])
 {
     int i, j;
     char c;
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void UART_CONFIG(void){

#ifdef BLUETOOTH
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

    GPIOPinTypeUART(GPIO_PORTK_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOPinConfigure(GPIO_PK0_U4RX);
    GPIOPinConfigure(GPIO_PK1_U4TX);
    ROM_UARTConfigSetExpClk(UART4_BASE, ClockSRCFreq, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOPinConfigure(GPIO_PK0_U4RX);
    GPIOPinConfigure(GPIO_PK1_U4TX);
    ROM_UARTConfigSetExpClk(UART4_BASE, ClockSRCFreq, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));

#endif

#ifdef UART_0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_UARTConfigSetExpClk(UART0_BASE, ClockSRCFreq, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));
#endif

#ifdef ADC_AND_FP
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);
        GPIOPinConfigure(GPIO_PP0_U6RX);
        GPIOPinConfigure(GPIO_PP1_U6TX);
        GPIOPinTypeUART(GPIO_PORTP_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
        ROM_UARTConfigSetExpClk(UART6_BASE, ClockSRCFreq, 9600,
                                   (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE));
    ROM_IntEnable(INT_UART6);
    ROM_UARTIntEnable(UART6_BASE, (UART_INT_RX | UART_INT_RT));
#endif

#ifdef  BLUETOOTH
    ROM_IntEnable(INT_UART4);
    ROM_UARTIntEnable(UART4_BASE, (UART_INT_RX | UART_INT_RT));
#endif

#ifdef UART_0
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, (UART_INT_RX | UART_INT_RT));
#endif

}

void MENU_DISPLAY(void *pvParameter){
    /*
    UART_send_A0("MENU Options:", 13);
    UART_send_byteA0(0x0A);
    UART_send_A0("1.Check Print", 13);
    UART_send_byteA0(0x0A);
    UART_send_A0("2.Enroll Print", 14);
    UART_send_byteA0(0x0A);
    UART_send_A0("3.Delete Print", 14);
    UART_send_byteA0(0x0A);
    UART_send_A0("4.Delete ALL", 12); */
   // xTaskNotify(xFP_Menu, 0x01, eSetBits );
    xTaskNotifyGive(xFP_Menu);
    vTaskDelete(xMenuHandle);
}

#ifdef BLUETOOTH
void UART_send_byteA4(uint8_t data){
     while(UARTBusy(UART4_BASE)); //May have to change to using the TXIFG flag
         UARTCharPut(UART4_BASE, data);
  }

  void UART_send_A4(uint8_t * data, uint32_t length){
      int i = 0;
      for(i=0 ; i<length ; i++){
          UART_send_byteA4(data[i]);
      }
  }
#endif


#ifdef UART_0
void UART_send_byteA0(uint8_t data){
     while(UARTBusy(UART0_BASE)); //May have to change to using the TXIFG flag
         UARTCharPut(UART0_BASE, data);
  }

  void UART_send_A0(uint8_t * data, uint32_t length){
      int i = 0;
      for(i=0 ; i<length ; i++){
          UART_send_byteA0(data[i]);
      }
  }

#endif

#ifdef ADC_AND_FP

  void UART_send_byteA6(uint8_t data){
       while(UARTBusy(UART6_BASE)); //May have to change to using the TXIFG flag
           UARTCharPut(UART6_BASE, data);
    }

    void UART_send_A6(const uint8_t * data, uint32_t length){
        int i = 0;
        for(i=0 ; i<length ; i++){
            UART_send_byteA6(data[i]);
        }
    }
#endif



  void UART6_IRQHandler(void){
         if (FP_state == FP_WAIT_RSP)
            {
                rsp_databuf[FPS_byte_count++] = UARTCharGet(UART6_BASE);
                if(FPS_byte_count == FP_RSP_PACKET_LENGTH)
                {
                    FP_state = FP_PROCESS_RSP;
                    UARTIntClear(UART6_BASE, UART_INT_RX);
                }
           }
         else if (FP_state == FP_WAIT_DATA)
             {
                 data_databuf[FPS_byte_count++] = UARTCharGet(UART6_BASE);
                 if(FPS_byte_count == FP_DATA_PACKET_LENGTH )
                     {
                         FP_state = FP_PROCESS_RSP;
                         UARTIntClear(UART6_BASE, UART_INT_RX);
                     }
             }
     }

#ifdef UART_0
void UART0_IRQHandler(void){
     // uint32_t IntRX = ROM_UARTIntStatus(UART0_BASE, (UART_INT_RX));
      if(ROM_UARTIntStatus(UART0_BASE, (UART_INT_RX))){
          //ROM_UARTIntClear(UART0_BASE, (UART_INT_RX | UART_INT_RT));
          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
          uint8_t input = UARTCharGet(UART0_BASE);

      switch(input){
#ifdef ADC_AND_FP
      case '0':
          vTaskNotifyGiveFromISR(xFP_Menu, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
      case '1':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
          xEventGroupSetBitsFromISR(xUserInEvent, FP_CHECKID_TASK_BIT, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
      case '2':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_ENROLL_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '3':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_DELETEID_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '4':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_DELETEALL_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '5':
              xEventGroupSetBitsFromISR(xADCEvent, readADCBit, &xHigherPriorityTaskWoken);
              portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
              break;
      case '6':
              xEventGroupSetBitsFromISR(xADC_CALEvent, calibADCBit, &xHigherPriorityTaskWoken);
              portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
              break;
#endif
#ifdef ADC_ONLY
      case '6':
          vTaskNotifyGiveFromISR(xCalib_ADC, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
#endif
      default:
          UART_send_A0("Invalid Option", 14);
          UART_send_byteA0(0x0A);
          break;
      }
      ROM_UARTIntClear(UART0_BASE, (UART_INT_RX));
    }

      ROM_UARTIntClear(UART0_BASE, (UART_INT_RT));
  }
#endif


void UART4_IRQHandler(void){
     // uint32_t IntRX = ROM_UARTIntStatus(UART0_BASE, (UART_INT_RX));
      if(ROM_UARTIntStatus(UART4_BASE, (UART_INT_RX))){
          //ROM_UARTIntClear(UART0_BASE, (UART_INT_RX | UART_INT_RT));
          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
          uint8_t input = UARTCharGet(UART4_BASE);
          countA0 = 0;

      switch(input){
#ifdef ADC_AND_FP
      case '0':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
          vTaskNotifyGiveFromISR(xFP_Menu, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
      case '1':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
          xEventGroupSetBitsFromISR(xUserInEvent, FP_CHECKID_TASK_BIT, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
      case '2':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_ENROLL_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '3':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_DELETEID_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '4':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
               xEventGroupSetBitsFromISR(xUserInEvent, FP_DELETEALL_TASK_BIT, &xHigherPriorityTaskWoken);
               portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
               break;
      case '5':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
              xEventGroupSetBitsFromISR(xADCEvent, readADCBit, &xHigherPriorityTaskWoken);
              portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
              break;
      case '6':
          ROM_TimerDisable(TIMER2_BASE, TIMER_A);
              vTaskNotifyGiveFromISR(xCalib_ADC, &xHigherPriorityTaskWoken);
              portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
              break;
#endif
#ifdef ADC_ONLY
      case '6':
          vTaskNotifyGiveFromISR(xCalib_ADC, &xHigherPriorityTaskWoken);
          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
          break;
#endif
      default:
          UART_send_A4("Invalid Option", 14);
          UART_send_byteA4(0x0A);
          break;
      }
      ROM_UARTIntClear(UART4_BASE, (UART_INT_RX));
    }

      ROM_UARTIntClear(UART4_BASE, (UART_INT_RT));
  }



