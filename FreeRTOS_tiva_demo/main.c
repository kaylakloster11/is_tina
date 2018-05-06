/* FreeRTOS_tiva_demo
 *
 * main.c
 *
 * Kayla Kloster
 */

//#include <ledtasks.h>
#include "main.h"
#include "ADC.h"
#include "circbuf.h"
#include "FPS.h"
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
#include "utils/uartstdio.h"
#include "UARTproj.h"

// TivaWare includes

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"


// Demo Task declarations
/*void LED_Task1(void *pvParameters);
void LED_Task2(void *pvParameters); */
#define USERIN_Handler  (1)
#define FPS_Handler     (2)


EventGroupHandle_t xUserInEvent;
EventGroupHandle_t xADCEvent;
#ifdef TIMED
    EventGroupHandle_t xADCEventTimed;
#endif

//FPS state
volatile FP_cmd_packet cp;
volatile uint8_t FP_state;
volatile  uint8_t data_databuf[30];
volatile  uint8_t rsp_databuf[12];
volatile  FP_resp rp;
volatile uint8_t FPS_byte_count = 0;

TaskHandle_t xMenuHandle = NULL, xFP_Menu = NULL, xCalib_ADC = NULL, xFirst_Calib = NULL, xADC_Timed;

#ifdef ADC_AND_FP
   int  Xadc[BUFFER] = {0};
#endif

// Main function
int main(void)
 {
    ClockSRCFreq = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000); /* Set clock source to be directly from crystal at 120 MHz */
    UART_CONFIG();
    TIMERA0_CONFIG();
    TIMERA2_CONFIG();
    //TIMERA1_CONFIG();
    ADC_CONFIG();

    ROM_IntMasterEnable();
    ROM_TimerEnable(TIMER2_BASE, TIMER_A);
  //  ROM_TimerEnable(TIMER1_BASE, TIMER_A);

      xUserInEvent = xEventGroupCreate();
      xADCEvent = xEventGroupCreate();
#ifdef TIMED
      xADCEventTimed = xEventGroupCreate();
#endif

      xTaskCreate(READ_ADC, (const portCHAR *)"Read Accel Data",
                       configMINIMAL_STACK_SIZE, NULL, 2, NULL);
#ifdef ADC_ONLY
      xTaskCreate(CALIBRATE_ADC, (const portCHAR *)"Calibrate ADC",
                      configMINIMAL_STACK_SIZE, NULL, 4, &xCalib_ADC);

      xTaskCreate(CALIB_ONE, (const portCHAR *)"First Calibration",
                            configMINIMAL_STACK_SIZE, NULL, 3, &xFirst_Calib);
#endif

#ifdef ADC_AND_FP
    xTaskCreate(FP_Task, (const portCHAR *)"Fingerprint Sensor",
                configMINIMAL_STACK_SIZE, NULL, 2, NULL);

      xTaskCreate(MENU_DISPLAY, (const portCHAR *)"Notify FP_MENU",
                  configMINIMAL_STACK_SIZE, NULL, 1, &xMenuHandle);

       xTaskCreate(FP_MENU, (const portCHAR *)"User Menu",
                     configMINIMAL_STACK_SIZE, NULL, 1, &xFP_Menu);

    xTaskCreate(READ_ADCTimed, (const portCHAR *)"Timed Accel Data",
                     configMINIMAL_STACK_SIZE, NULL, 3, &xADC_Timed);

    xTaskCreate(CALIBRATE_ADC, (const portCHAR *)"Calibrate ADC",
                    configMINIMAL_STACK_SIZE, NULL, 4, &xCalib_ADC);
#endif

    vTaskStartScheduler();

    for(;;);
    return 0;
}


void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
