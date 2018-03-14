/*
 * LED1Task.c
 *
 *  Created on: Feb 26, 2018
 *      Author: kayla
 */

#ifndef LED1TASK_C_
#define LED1TASK_C_

// FreeRTOS includes
#include <ledtasks.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "pin_map.h"

// Flash the LEDs on the launchpad
void LED_Task1(void *pvParameters)
{
    for (;;)
    {
        LEDWrite(GPIO_PF4_EN0LED1, 0x00);
        LEDWrite(GPIO_PF0_EN0LED0, 0x01);
        vTaskDelay(100);
    }
}


#endif /* LED1TASK_C_ */



