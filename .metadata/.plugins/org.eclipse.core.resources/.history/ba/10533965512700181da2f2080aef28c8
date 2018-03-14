/*
 * LED2Task.c
 *
 *  Created on: Feb 26, 2018
 *      Author: kayla
 */

// FreeRTOS includes
#include <ledtasks.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "drivers/pinout.h"
#include "pin_map.h"

// Flash the LEDs on the launchpad
void LED_Task2(void *pvParameters)
{
    for (;;)
    {
        // Turn on LED 2
        LEDWrite(GPIO_PF0_EN0LED0, 0x00);
        // Turn on LED 2
        LEDWrite(GPIO_PF4_EN0LED1, 0x01);
        vTaskDelay(100);
    }
}






