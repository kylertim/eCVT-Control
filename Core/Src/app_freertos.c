/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern TIM_HandleTypeDef htim3;
uint8_t currentGear = 1;
volatile uint8_t shiftRequested = 0;

/* USER CODE END Variables */
/* Definitions for LEDTask */
osThreadId_t LEDTaskHandle;
const osThreadAttr_t LEDTask_attributes = {
  .name = "LEDTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for YellowTask */
osThreadId_t YellowTaskHandle;
const osThreadAttr_t YellowTask_attributes = {
  .name = "YellowTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for RPMTask */
osThreadId_t RPMTaskHandle;
const osThreadAttr_t RPMTask_attributes = {
  .name = "RPMTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for GearTask */
osThreadId_t GearTaskHandle;
const osThreadAttr_t GearTask_attributes = {
  .name = "GearTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for MotorTask */
osThreadId_t MotorTaskHandle;
const osThreadAttr_t MotorTask_attributes = {
  .name = "MotorTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for RPMQueue */
osMessageQueueId_t RPMQueueHandle;
const osMessageQueueAttr_t RPMQueue_attributes = {
  .name = "RPMQueue"
};
/* Definitions for MotorQueue */
osMessageQueueId_t MotorQueueHandle;
const osMessageQueueAttr_t MotorQueue_attributes = {
  .name = "MotorQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of RPMQueue */
  RPMQueueHandle = osMessageQueueNew (4, sizeof(uint32_t), &RPMQueue_attributes);
  /* creation of MotorQueue */
  MotorQueueHandle = osMessageQueueNew (4, sizeof(uint32_t), &MotorQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of LEDTask */
  LEDTaskHandle = osThreadNew(StartDefaultTask, NULL, &LEDTask_attributes);

  /* creation of YellowTask */
  YellowTaskHandle = osThreadNew(StartYellowTask, NULL, &YellowTask_attributes);

  /* creation of RPMTask */
  RPMTaskHandle = osThreadNew(StartRPMTask, NULL, &RPMTask_attributes);

  /* creation of GearTask */
  GearTaskHandle = osThreadNew(StartGearTask, NULL, &GearTask_attributes);

  /* creation of MotorTask */
  MotorTaskHandle = osThreadNew(StartMotorTask, NULL, &MotorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the LEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN LEDTask */
  /* Infinite loop */
  for(;;)
  {
	  BSP_LED_Toggle(LED_GREEN);
	  osDelay(500);
  }
  /* USER CODE END LEDTask */
}

/* USER CODE BEGIN Header_StartYellowTask */
/**
* @brief Function implementing the YellowTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartYellowTask */
void StartYellowTask(void *argument)
{
  /* USER CODE BEGIN YellowTask */
  /* Infinite loop */
  for(;;)
  {
	BSP_LED_Toggle(LED_YELLOW);
    osDelay(200);
  }
  /* USER CODE END YellowTask */
}

/* USER CODE BEGIN Header_StartRPMTask */
/**
* @brief Function implementing the RPMTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRPMTask */
void StartRPMTask(void *argument)
{
  /* USER CODE BEGIN RPMTask */

  uint32_t rpm = 1000;

  for (;;)
  {
    osMessageQueuePut(RPMQueueHandle, &rpm, 0, 0);

    rpm += 100;

    if (rpm > 2000)
    {
      rpm = 1000;
    }

    osDelay(1000);
  }

  /* USER CODE END RPMTask */
}

/* USER CODE BEGIN Header_StartGearTask */
/**
* @brief Function implementing the GearTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGearTask */
void StartGearTask(void *argument)
{
  /* USER CODE BEGIN GearTask */

  uint32_t receivedRPM;
  uint32_t targetRPM;
  float gearRatio;
  char message[100];

  for (;;)
  {
    if (osMessageQueueGet(RPMQueueHandle,
                          &receivedRPM,
                          NULL,
                          osWaitForever) == osOK)
    {
    	if (shiftRequested)
    	{
    	    shiftRequested = 0;

    	    currentGear++;

    	    if (currentGear > 5)
    	    {
    	        currentGear = 1;
    	    }
    	}
      switch (currentGear)
      {
        case 1:
          gearRatio = 3.50f;
          break;

        case 2:
          gearRatio = 2.20f;
          break;

        case 3:
          gearRatio = 1.50f;
          break;

        case 4:
          gearRatio = 1.00f;
          break;

        case 5:
          gearRatio = 0.80f;
          break;

        default:
          gearRatio = 3.50f;
          currentGear = 1;
          break;
      }

      uint32_t motorPWM;

      switch (currentGear)
      {
        case 1:
          motorPWM = 750;
          break;

        case 2:
          motorPWM = 800;
          break;

        case 3:
          motorPWM = 850;
          break;

        case 4:
          motorPWM = 900;
          break;

        case 5:
          motorPWM = 999;
          break;

        default:
          motorPWM = 750;
          break;
      }

      osMessageQueuePut(MotorQueueHandle,
                        &motorPWM,
                        0,
                        0);

      targetRPM = (uint32_t)(receivedRPM * gearRatio);

      snprintf(message, sizeof(message),
               "Gear: %u | Output RPM: %lu | Target RPM: %lu | PWM: %lu\r\n",
               currentGear,
               (unsigned long)receivedRPM,
               (unsigned long)targetRPM,
               (unsigned long)motorPWM);

      HAL_UART_Transmit(&hcom_uart[COM1],
                        (uint8_t *)message,
                        strlen(message),
                        100);
    }
  }

  /* USER CODE END GearTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the MotorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
	/* USER CODE BEGIN MotorTask */

	  uint32_t motorPWM = 750;

	  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

	  HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port,
	                    MOTOR_IN1_Pin,
	                    GPIO_PIN_SET);

	  HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port,
	                    MOTOR_IN2_Pin,
	                    GPIO_PIN_RESET);

	  /* Start motor at our known-good 75% PWM */
	  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, motorPWM);

	  for(;;)
	  {
	    if (osMessageQueueGet(MotorQueueHandle,
	                          &motorPWM,
	                          NULL,
	                          osWaitForever) == osOK)
	    {
	      /* Safety limit */
	      if (motorPWM > 999)
	      {
	        motorPWM = 999;
	      }

	      __HAL_TIM_SET_COMPARE(&htim3,
	                            TIM_CHANNEL_1,
	                            motorPWM);
	    }
	  }

	  /* USER CODE END MotorTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

