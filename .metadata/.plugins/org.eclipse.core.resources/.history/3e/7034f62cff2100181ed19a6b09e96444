/* FreeRTOS_tiva_demo
 *
 * main.c
 *
 * Kayla Kloster
 */

#include <ledtasks.h>
#include <stdint.h>
#include "main.h"
#include "drivers/pinout.h"

// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

// FreeRTOS includes
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"



// Demo Task declarations
void LED_Task1(void *pvParameters);
void LED_Task2(void *pvParameters);

// Main function
int main(void)
{
    // Initialize the GPIO pins for the Launchpad
   PinoutSet(0, 0);

    // Create demo tasks
    xTaskCreate(LED_Task1, (const portCHAR *)"LED1",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    xTaskCreate(LED_Task2, (const portCHAR *)"LED2",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    return 0;
}


void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
