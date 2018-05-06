/*
 * adc.c
 *
 *  Created on: Mar 19, 2018
 *      Author: kayla
 *
 *      1) validate voltage with multimeter
 *      2) Simpilfy to x
 *      3) harden wiring and setup for more consistent velocity testing (perhaps with a wheel or drill)
 *      4) integer math as much as possible, floating at the end end
 *      5) improve word choice
 *
 *      1) multimeter reading for Vx was around 1.5 volts (this is TYP for a Vs of 3V on datasheet). But should I change Vref in ADC.h for calculation
 *      to 3V or 1.5V
 *      2) How is the internal reference voltage 3V when I'm supplying it with 3.3V (Also, how would you use the external voltage if I needed a different reference
 *      voltage)
 */

#include "main.h"
#include "circbuf.h"
#include "adc.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
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

#include "driverlib/adc.h"


// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"

static uint8_t labelX[] =" X-Accel: ", labelY[] = " Y-Accel: ", labelZ[] = " Z-Accel: ";
static uint8_t vLabelX[] =" X-Velocity: ", vLabelY[] = " Y-Velocity: ";
static uint8_t labelsize = 10;
static uint8_t vlen = 13;

extern EventGroupHandle_t xADCEvent, xADC_CALEvent, xADCEventTimed;
float velocity[2] = {0}, accel[2] = {0}, Yvelocity[2] = {0}, Yaccel[2] = {0};
float calib_val = 0;

uint8_t delete = 0;

extern TaskHandle_t xCalib_ADC, xFirst_Calib, xADC_Timed;
#ifdef ADC_AND_FP
   extern int Xadc[BUFFER];
   float dispSpeed = 0.0;
#endif

void ADC_CONFIG(void){
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_AHB_BASE, GPIO_PIN_1 /*(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_2) */);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
   // ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL, 15); //32 MHz clock
    ADCReferenceSet(ADC0_BASE, ADC_REF_INT); // Reference voltage = internal 3V
    //Enable first sample sequencer to capture channel 0 adc value after ADC trigger in task
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    // X-axis data in ADC Channel0 (PE3 on board)
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, (ADC_CTL_IE |ADC_CTL_CH0 | ADC_CTL_END) /*(ADC_CTL_SHOLD_8 | ADC_CTL_CH0) */); //FCONV (Ksps) is 1600 (limit of 2Msps)
    //ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1  /*(ADC_CTL_SHOLD_8 | ADC_CTL_CH1) */);
    //ADCSequenceStepConfigure(ADC0_BASE, 0, 2, (ADC_CTL_CH2 | ADC_CTL_END));
       /* ADCSequenceStepConfigure(uint32_t ui32Base,
                                             uint32_t ui32SequenceNum,
                                             uint32_t ui32Step, uint32_t ui32Config); */

    ADCSequenceEnable(ADC0_BASE, 0);
   }


#ifdef ADC_ONLY
 void READ_ADC(void *pvParameter){
            EventBits_t xADCEventValue;
            const EventBits_t xBitsToWaitFor = readADCBit;
            for(;;){
                xADCEventValue = xEventGroupWaitBits(xADCEvent, xBitsToWaitFor, pdTRUE, pdFALSE, portMAX_DELAY);
                if(xADCEventValue){
          //Typ Sensitivity with Vref = 3V is 300 mV/g:
                    ROM_IntMasterDisable();
                    char printX[10] = {0}, printY[10] = {0}, printZ[10] = {0}, printVel[10] = {0}, printVelY[10] = {0};
                    countA0 = 0;
                    ADCBuf_t ADCBUFF;
                    float Xavg_val  =  (float)(Xval_adc/100);
                    ADCBUFF.Vx = (((Xavg_val)*Vref)/RESOLUTION);
                    ADCBUFF.Ax = (ADCBUFF.Vx - Vmax)/SENSE;
/*
                    ADCBUFF.Vy = ((Yval_adc/100.0)*Vref)/RESOLUTION;
                    ADCBUFF.Ay = (ADCBUFF.Vy-Vmax)/SENSE;

                    ADCBUFF.Vz = ((Zval_adc/100.0)*Vref)/RESOLUTION;
                    ADCBUFF.Az = (ADCBUFF.Vz-Vmax)/SENSE;

                     Xval_adc = 0.0;
                     Yval_adc = 0.0;
                     Zval_adc = 0.0; */

                     accel[1] = ADCBUFF.Ax - calib_val;
                  /*   if((accel[1] < .05) && (accel[1] > -0.05)){
                         accel[1] = 0;
                     }*/
                    // velocity[1] = velocity[0] + accel[0] + ((accel[1] - accel[0])/2);

                    Xval_adc = 0;

                     velocity[1] = velocity[0] + (accel[1] * 0.1);
                     float dispSpeed = velocity[1];
                     if((dispSpeed < 0.05) && (dispSpeed > -0.05)){
                         dispSpeed = 0.00;
                         velocity[1] = 0;
                     }
                     accel[0] = accel[1];
                     velocity[0] = velocity[1];

                     ftoa(dispSpeed, printVel, 3);
                     uint32_t len_speed = strlen((char *)printVel);

             /*        Yaccel[1] = ADCBUFF.Ax - calib_val;
                      Yvelocity[1] = Yvelocity[0] + (accel[1] * 0.1);
                      float YdispSpeed = Yvelocity[1];
                      if((YdispSpeed < 0.005) && (dispSpeed > -0.005)){
                          YdispSpeed = 0.00;
                          Yvelocity[1] = 0;
                      }

                      Yvelocity[0] = Yvelocity[1];

                      ftoa(YdispSpeed, printVelY, 3);
                      uint32_t len_speedY = strlen((char *)printVelY);
                    */
                     ftoa(accel[1], printX, 3);
                     uint32_t length_X = strlen((char *)printX);
#ifdef UART_0
                     UART_send_A0(labelX, labelsize);
                     UART_send_A0(&printX, length_X);
                     UART_send_A0(vLabelX, vlen);
                     UART_send_A0(&printVel, len_speed);
                     UART_send_byteA0(0x0A);

#endif

#ifdef BLUETOOTH
                     UART_send_A4(labelX, labelsize);
                     UART_send_A4(&printX, length_X);
                     UART_send_A4(vLabelX, vlen);
                     UART_send_A4(&printVel, len_speed);
                     UART_send_byteA4(0x0A);
#endif

                     ROM_IntMasterEnable();
                }
           }
     }

 void CALIBRATE_ADC(void *pvParameter){
        for(;;){
           uint8_t waitVal = ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
           if(waitVal != 0){
               ROM_IntMasterDisable();
#ifdef BLUETOOTH
               UART_send_A4("Calibrating...", 14);
               UART_send_byteA4(0x0A);
#endif

#ifdef UART_0
               UART_send_A0("Calibrating...", 14);
               UART_send_byteA0(0x0A);
#endif
                                 //Typ Sensitivity with Vref = 3V is 300 mV/g:
                char printX[10] = {0}, printY[10] = {0}, printZ[10] = {0}, printVel[10] = {0};
                ADCBuf_t ADCBUFF;
                int i = 0;
                float Xavg_val  =  (float)(Xval_adc/countA0);
                countA0 = 0;
                ADCBUFF.Vx = (((Xavg_val)*Vref)/RESOLUTION);
                ADCBUFF.Ax = (ADCBUFF.Vx - Vmax)/SENSE;

                Xval_adc = 0;

                velocity[0] = 0;
                velocity[1] = 0;
                accel[0] = 0;
                accel[1] = 0;

                calib_val = ADCBUFF.Ax;
#ifdef BLUETOOTH
               UART_send_A4("Done Calibrating", 16);
               UART_send_byteA4(0x0A);
#endif

#ifdef UART_0
               UART_send_A0("Done Calibrating", 16);
               UART_send_byteA0(0x0A);
#endif
                ROM_IntMasterEnable();
            }
        }
     }

void CALIB_ONE(void *pvParameter){
    for(;;){
      if(delete){
          vTaskDelete(xFirst_Calib);
      }
      else if(countA0 == 50){
          delete = 1;
          xTaskNotifyGive(xCalib_ADC);
      }
    }
 }


#endif

#ifdef ADC_AND_FP
void READ_ADC(void *pvParameter){
       EventBits_t xADCEventValue;
       const EventBits_t xBitsToWaitFor = readADCBit;
       for(;;){
           xADCEventValue = xEventGroupWaitBits(xADCEvent, xBitsToWaitFor, pdTRUE, pdFALSE, portMAX_DELAY);
           if(xADCEventValue){
     //Typ Sensitivity with Vref = 3V is 300 mV/g:
               ROM_IntMasterDisable();
               char printX[10] = {0}, printY[10] = {0}, printZ[10] = {0}, printVel[10] = {0};

                 ftoa(dispSpeed, printVel, 3);
                 uint32_t len_speed = strlen((char *)printVel);
                 ftoa(accel[1], printX, 3);
                 uint32_t length_X = strlen((char *)printX);
#ifdef UART_0
                 UART_send_A0(labelX, labelsize);
                 UART_send_A0(&printX, length_X);
                 UART_send_A0(vLabelX, vlen);
                 UART_send_A0(&printVel, len_speed);
                 UART_send_byteA0(0x0A);

#endif

#ifdef BLUETOOTH
                 UART_send_A4(labelX, labelsize);
                 UART_send_A4(&printX, length_X);
                 UART_send_A4(vLabelX, vlen);
                 UART_send_A4(&printVel, len_speed);
                 UART_send_byteA4(0x0A);
#endif
                 ROM_TimerEnable(TIMER2_BASE, TIMER_A);
                 ROM_IntMasterEnable();
           }
       }
    }

void READ_ADCTimed(void *pvParameter){
    for(;;){
       uint8_t waitVal = ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
       if(waitVal != 0){
           ROM_IntMasterDisable();
               ADCBuf_t ADCBUFF;
                  int i = 0;
                  float Xval_adc = 0.0, time = 0.0;
                  int add_Xval = 0;
                  countA0 = 0;
                   /* while(!CB_is_buffer_empty(&Xadc)){
                       avg_X += CB_remove_item_from_buffer(&Xadc);
                       lenX++;
                   } */
                    for(i=0; i<BUFFER; i++){
                        add_Xval += Xadc[i];
                    }

                    Xval_adc  =  (float)(add_Xval);
                    ADCBUFF.Vx = (((Xval_adc/LENGTH)*Vref)/RESOLUTION);
                    ADCBUFF.Ax = (ADCBUFF.Vx - Vmax)/SENSE;

                    accel[1] = ADCBUFF.Ax - calib_val;
                 /*   if((accel[1] < .05) && (accel[1] > -0.05)){
                        accel[1] = 0;
                    }*/
                   // velocity[1] = velocity[0] + accel[0] + ((accel[1] - accel[0])/2);

                   Xval_adc = 0;

                    velocity[1] = velocity[0] + (accel[1] * TIME_MS);
                   dispSpeed = velocity[1];

                    if((dispSpeed < 0.05) && (dispSpeed > -0.05)){
                        dispSpeed = 0.00;
                        velocity[1] = 0;
                    }
                    accel[0] = accel[1];
                    velocity[0] = velocity[1];
#ifdef TEST
                char printX[10] = {0}, printY[10] = {0}, printZ[10] = {0}, printVel[10] = {0};

                  ftoa(dispSpeed, printVel, 3);
                  uint32_t len_speed = strlen((char *)printVel);
                  ftoa(accel[1], printX, 3);
                  uint32_t length_X = strlen((char *)printX);
 #ifdef UART_0
                  UART_send_A0(labelX, labelsize);
                  UART_send_A0(&printX, length_X);
                  UART_send_A0(vLabelX, vlen);
                  UART_send_A0(&printVel, len_speed);
                  UART_send_byteA0(0x0A);

 #endif

 #ifdef BLUETOOTH
                  UART_send_A4(labelX, labelsize);
                  UART_send_A4(&printX, length_X);
                  UART_send_A4(vLabelX, vlen);
                  UART_send_A4(&printVel, len_speed);
                  UART_send_byteA4(0x0A);
 #endif
#endif

                 ROM_IntMasterEnable();
           }
       }
    }

void CALIBRATE_ADC(void *pvParameter){
    for(;;){
       uint8_t waitVal = ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
       if(waitVal != 0){
           ROM_IntMasterDisable();
#ifdef BLUETOOTH
               UART_send_A4("Calibrating...", 14);
               UART_send_byteA4(0x0A);
#endif

#ifdef UART_0
               UART_send_A0("Calibrating...", 14);
               UART_send_byteA0(0x0A);
#endif
                                 //Typ Sensitivity with Vref = 3V is 300 mV/g:
                char printX[10] = {0}, printVel[10] = {0};
                ADCBuf_t ADCBUFF;
               int i = 0;
               float avg_X = 0.0;
               int add_X = 0;

               for(i=0; i<BUFFER; i++){
                   add_X += Xadc[i];
               }

               avg_X = (float)add_X;
               ADCBUFF.Vx = (((avg_X/LENGTH)*Vref)/RESOLUTION);
               ADCBUFF.Ax = (ADCBUFF.Vx - Vmax)/SENSE;

               calib_val = ADCBUFF.Ax;
               velocity[0] = 0;
               velocity[1] = 0;
               accel[0] = 0;
               accel[1] = 0;


#ifdef BLUETOOTH
              UART_send_A4("Done Calibrating", 16);
              UART_send_byteA4(0x0A);
#endif

#ifdef UART_0
              UART_send_A0("Done Calibrating", 16);
              UART_send_byteA0(0x0A);
#endif

              ROM_TimerEnable(TIMER2_BASE, TIMER_A);
               ROM_IntMasterEnable();
           }
       }
    }

#endif


  void FloatReverse(char *str, int len)
  {
      int i=0, j=len-1, temp;
      while (i<j)
      {
          temp = str[i];
          str[i] = str[j];
          str[j] = temp;
          i++; j--;
      }
  }
  // Converts a given integer x to string str[].  d is the number
   // of digits required in output. If d is more than the number
   // of digits in x, then 0s are added at the beginning.
  int intToStr(int x, char str[], int d, uint8_t isNeg)
  {
      int i = 0;
      while (x)
      {
          str[i++] = (x%10) + '0';
          x = x/10;
      }
      // If number of digits required is more, then
      // add 0s at the beginning
      while (i < d)
          str[i++] = '0';
      if(isNeg){
         str[i++] = '-' ;
       }
      FloatReverse(str, i);
      str[i] = '\0';
      return i;
  }
  // Converts a floating point number to string.
  void ftoa(float n, char *res, int afterpoint)
  {
      uint8_t isNeg = 0;
      if(n < 0){
          n = -n;
          isNeg = 1;
      }
      // Extract integer part
      int ipart = (int)n;
      // Extract floating part
      float fpart = n - (float)ipart;
      // convert integer part to string
      int i = intToStr(ipart, res, 0, isNeg);
      // check for display option after point
      if (afterpoint != 0)
      {
           res[i] = '.';  // add dot

          // Get the value of fraction part upto given no.
          // of points after dot. The third parameter is needed
          // to handle cases like 233.007
          fpart = fpart * pow(10, afterpoint);

          intToStr((int)fpart, res + i + 1, afterpoint, 0);
      }
  }



