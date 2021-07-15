/**
  ******************************************************************************
  * @file    lcd.c
  * @author  MCD Application Team
  * @brief   lcd 7 segments driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lcd.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const signal_t seg7_sig[7]=
{
 {GPIOC, GPIO_PIN_7},
 {GPIOA, GPIO_PIN_9},  
 {GPIOA, GPIO_PIN_8}, 
 {GPIOB, GPIO_PIN_10}, 
 {GPIOB, GPIO_PIN_4},
 {GPIOB, GPIO_PIN_5}, 
 {GPIOB, GPIO_PIN_3}, 
};

const signal_t colon_sig = {GPIOA, GPIO_PIN_10};
    
const signal_t ctl_sig[4] = 
{
  {GPIOA, GPIO_PIN_0},  
  {GPIOA, GPIO_PIN_1},
  {GPIOA, GPIO_PIN_4},
  {GPIOC, GPIO_PIN_0},
};


const uint8_t seg7_table[10][7] = 
{
  {0, 0, 0, 0, 0, 0, 1},  //0
  {1, 0, 0, 1, 1, 1, 1},  //1
  {0, 0, 1, 0, 0, 1, 0},  //2
  {0, 0, 0, 0, 1, 1, 0},  //3
  {1, 0, 0, 1, 1, 0, 0},  //4
  {0, 1, 0, 0, 1, 0, 0},  //5
  {0, 1, 0, 0, 0, 0, 0},  //6
  {0, 0, 0, 1, 1, 1, 1},  //7
  {0, 0, 0, 0, 0, 0, 0},  //8
  {0, 0, 0, 0, 1, 0, 0},  //9
};

const uint8_t seg7_dim[10] = 
{
  6,  //0
  2,  //1
  5,  //2
  5,  //3
  4,  //4
  5,  //5
  6,  //6
  2,  //7
  7,  //8
  6,  //9
};

static GPIO_InitTypeDef  GPIO_InitStruct;
TIM_HandleTypeDef    TimHandle;
static digit_t digits;
void TIM3_IRQHandler(void);
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void LCD_PrintDigit (uint8_t digit, uint8_t position, uint8_t colon)
{
  digits.digit[position] = digit;
  if(position == 2)
  {
    digits.colon = colon;  
  }
}


/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void LCD_ControlDigit (uint8_t position, uint8_t onoff)
{
  digits.state[position] = onoff;
}
/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void LCD_Update (void)
{
  uint8_t position;
  static uint8_t counter = 1, prev = 0;
  
  position =  counter  % 4; 
#ifdef DIMMING   
  uint8_t dim_max;
  static uint32_t dim = 0;
  
  dim_max = seg7_dim[digits.digit[position]];  

  if((position == 1) && (digits.colon))
  {
    dim_max += 2;
  }
    
  if(++dim > dim_max)
  {
    dim = 0;
    counter++;
  }
#else
    counter++;
#endif
  
  
  if(prev != position) 
  {
    HAL_GPIO_WritePin(ctl_sig[prev].port, 
                      ctl_sig[prev].pin, 
                      (GPIO_PinState)0);    
    
    
    for(uint8_t seg = 0; seg < 7; seg++)
    {
      HAL_GPIO_WritePin(seg7_sig[seg].port, 
                        seg7_sig[seg].pin, 
                        (GPIO_PinState)seg7_table[digits.digit[position]][seg]);
    }
    
    HAL_GPIO_WritePin(colon_sig.port, 
                      colon_sig.pin, 
                      (GPIO_PinState)(1-digits.colon));  
    
    
    HAL_GPIO_WritePin(ctl_sig[position].port, 
                      ctl_sig[position].pin, 
                      (GPIO_PinState)digits.state[position]); 
    prev = position; 
  }
}


/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void LCD_PrintNumber (uint32_t Number)
{
   LCD_PrintDigit((Number/1000)%10, 0, 0); 
   LCD_PrintDigit((Number/100)%10, 1, 0);
   LCD_PrintDigit((Number/10)%10, 2, 0); 
   LCD_PrintDigit((Number%10), 3, 0);   
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
 void LCD_PrintTime (uint8_t hour, uint8_t min, uint8_t colon)
{  
    LCD_PrintDigit(hour / 10, 0, 0); 
    LCD_PrintDigit(hour % 10, 1, 0);
    LCD_PrintDigit(min / 10, 2, colon); 
    LCD_PrintDigit(min % 10, 3, 0);     
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void LCD_Init (void)
{
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  // segments Pins
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  
  for (uint8_t seg = 0; seg < 7; seg ++)
  {
    GPIO_InitStruct.Pin = seg7_sig[seg].pin;
    HAL_GPIO_Init(seg7_sig[seg].port, &GPIO_InitStruct); 
  }
  
  //colon pin
  GPIO_InitStruct.Pin = colon_sig.pin;
  HAL_GPIO_Init(colon_sig.port, &GPIO_InitStruct); 
  
  // Control Pins
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  
  for (uint8_t ctl = 0; ctl < 4; ctl ++)
  {
    GPIO_InitStruct.Pin = ctl_sig[ctl].pin;
    HAL_GPIO_Init(ctl_sig[ctl].port, &GPIO_InitStruct); 
  }

  
  /* Configure timer @ 2KhZ */
  __HAL_RCC_TIM3_CLK_ENABLE();
  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);  
  
  // 2 KHz
  TimHandle.Instance = TIM3;
  TimHandle.Init.Period = SystemCoreClock/8000 - 1;
  TimHandle.Init.Prescaler = 4 - 1;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&TimHandle); 
  HAL_TIM_Base_Start_IT(&TimHandle);
  
  for (uint8_t pos = 0; pos < 4; pos ++)
  {
      digits.state[pos] = 1;
  }
  

}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    LCD_Update();
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&TimHandle);
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
