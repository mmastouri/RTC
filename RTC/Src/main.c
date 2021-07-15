/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd.h"   
#include "rtc.h"  
#include "button.h"  

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
  static uint8_t mode_setting = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

static void Refresh_Time  (uint32_t period);
static void Refresh_vbat  (uint32_t period);

static void process_set (void);
static void process_mode (void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Init HAL Library */
  HAL_Init();
   
  /* Configure the system clock to 100 MHz */
  SystemClock_Config();
  
  /* Init RTC */
  RTC_Init();
  
  /* Init VBAT monitor */
  VBAT_Init();
  
  /* Init LCD*/
  LCD_Init();
  
  /* Init button */
  BUTTON_Init(process_mode, process_set);

  while (1)
  { 
    Refresh_vbat(1000);
    Refresh_Time(500);
    BUTTON_HandleEvents();
  }
}


/**
  * @brief  process mode
  * @param  None
  * @retval None
  */
static void process_mode (void)
{
  if(++mode_setting > 3)
  {
    mode_setting = 0;
  }  
}

/**
  * @brief  process events
  * @param  None
  * @retval None
  */
static void process_set (void)
{
  uint8_t m , h;
  
  if((mode_setting > 0) && (mode_setting < 3))
  {
    RTC_GetTime(&h, &m);
    
    if(mode_setting == 1)
    {
      h = ( h + 1) % 24;
    }
    else if( mode_setting == 2)
    {
      m = ( m + 1) % 60;
    }
    
    RTC_SetTime(h, m);  
  }
  
}
/**
  * @brief  Display time
  * @param  None
  * @retval None
  */
static void Refresh_vbat (uint32_t period)
{
  static uint32_t tick = 0;
  
  if((HAL_GetTick() - tick > period) || (!tick))
  {
    tick = HAL_GetTick();  
    
    if(mode_setting == 3)  
    {
      LCD_ControlDigit(0, 1);
      LCD_ControlDigit(1, 1);
      LCD_ControlDigit(2, 1);
      LCD_ControlDigit(3, 1);
      LCD_PrintNumber(((VBAT_GetVoltage() * 3) * 3300) / 4096);
    }
  }
}
/**
  * @brief  Display time
  * @param  None
  * @retval None
  */
static void Refresh_Time (uint32_t period)
{
  static uint32_t tick = 0;
  static uint8_t colon = 0;
  uint8_t m , h;
    
  if((HAL_GetTick() - tick > period) || (!tick))
  {
    tick = HAL_GetTick();  
    
    RTC_GetTime(&h, &m);
    colon = 1- colon;
    
    LCD_ControlDigit(0, 1);
    LCD_ControlDigit(1, 1);
    LCD_ControlDigit(2, 1);
    LCD_ControlDigit(3, 1);
    
    if(mode_setting == 1 )//Hour
    {
      LCD_ControlDigit(0, colon);
      LCD_ControlDigit(1, colon);
      LCD_PrintTime(h, m, 0);
      
    }
    else if(mode_setting == 2)
    {
      LCD_ControlDigit(2, colon);
      LCD_ControlDigit(3, colon);
      LCD_PrintTime(h, m, 0);
    }
    else if(mode_setting == 0)
    {
      LCD_PrintTime(h, m, colon);    
    }
  }
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_HIGH);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
