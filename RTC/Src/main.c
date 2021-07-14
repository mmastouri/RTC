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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint8_t mode_setting = 0, change_set_ready = 1, apply_set_ready = 0;
uint8_t button_state, last_button_state = 0, debounce_time;

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void Refresh_Time(uint8_t state);
static void Refresh_vbat (void);
static void process_event (uint32_t setting);
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
  uint32_t tick_lcd, tick_button_press, tick_button_unpress;    
  
  /* Init HAL Library */
  HAL_Init();
  
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* Configure the system clock to 100 MHz */
  SystemClock_Config();
  
  /* Init RTC */
  RTC_Init();
  
  /* Init VBAT monitor */
  VBAT_Init();
  
  /* Set_Init Time */
  //RTC_SetTime(HOUR_S, MIN_S);
  
  /* Init LCD*/
  LCD_Init();
  
  /* Init key user */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  
  /* get start tick */
  tick_lcd = HAL_GetTick();    
  
  while (1)
  { 
    
    if(HAL_GetTick() - tick_lcd > 500)
    {
      tick_lcd = HAL_GetTick();  
      
      if(mode_setting == 3)
      {
        Refresh_vbat();
      }
      else
      {
        Refresh_Time(mode_setting);
      }
    }

    
    /* debounce button */ 
    uint8_t  read = 1 - BSP_PB_GetState(BUTTON_KEY); // invert 1 : press / 0 : unpress
    
    if (read != last_button_state)
    {
      debounce_time = HAL_GetTick() ;
      last_button_state = read;
    }
    
    if(HAL_GetTick() - debounce_time > 100)
    {
      button_state = read ;
    }

    /* handle events */
    if(button_state)
    {
      tick_button_unpress = HAL_GetTick();
      
      if(change_set_ready == 0)
      {
        if(HAL_GetTick() - tick_button_press > 700)
        {
          apply_set_ready = 0;
          change_set_ready = 1;
          if(++mode_setting > 3)
          {
            mode_setting = 0;
          }
        }
        else
        {
          apply_set_ready = 1;
        }
      }
    }
    else
    {
      tick_button_press = HAL_GetTick();
      
      if(HAL_GetTick() - tick_button_unpress > 100)
      {           
        if(apply_set_ready)
        {
          process_event(mode_setting);   
          apply_set_ready = 0; 
        }
      }
      change_set_ready = 0; 
    }
  }
}

/**
  * @brief  process events
  * @param  None
  * @retval None
  */
static void process_event (uint32_t setting)
{
  uint8_t m , h;
  
  if((setting > 0) && (setting < 3))
  {
    RTC_GetTime(&h, &m);
    
    if(setting == 1)
    {
      h = ( h + 1) % 24;
    }
    else if( setting == 2)
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
static void Refresh_vbat (void)
{
   LCD_PrintNumber(((VBAT_GetVoltage() * 3) * 3300) / 4096);
}
/**
  * @brief  Display time
  * @param  None
  * @retval None
  */
static void Refresh_Time (uint8_t state)
{
  static uint8_t colon = 0;
  uint8_t m , h;
 
  RTC_GetTime(&h, &m);
  colon = 1- colon;
  
  LCD_PrintTime(h, m, 1);    
  
  if(state == 1 )//Hour
  {
    LCD_ControlDigit(0, colon);
    LCD_ControlDigit(1, colon);
    
  }
  else if(state == 2)
  {
    LCD_ControlDigit(2, colon);
    LCD_ControlDigit(3, colon);
  }
  else
  {
    LCD_ControlDigit(0, 1);
    LCD_ControlDigit(1, 1);
    LCD_ControlDigit(2, 1);
    LCD_ControlDigit(3, 1);
  }
  LCD_PrintTime(h, m, colon);
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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
