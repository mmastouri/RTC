/**
  ******************************************************************************
  * @file    button.c
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
#include "button.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t short_press_enabled = 1, apply_short_press = 0;
static uint8_t button_state = 0, last_button_state = 0, debounce_time = 0;  
static uint32_t tick_button_press = 0, tick_button_unpress = 0;   
static button_cb button_long_press_cb;
static button_cb button_short_press_cb;
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  process events
  * @param  None
  * @retval None
  */
void BUTTON_Init (button_cb long_press, button_cb set)
{
  button_long_press_cb = long_press;
  button_short_press_cb = set;
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
}
/**
  * @brief  process events
  * @param  None
  * @retval None
  */
void BUTTON_HandleEvents (void)
{

  uint8_t  read;
  
   read = BSP_PB_GetState(BUTTON_KEY); 
   
  /* debounce */   
  if (read != last_button_state)
  {
    debounce_time = HAL_GetTick() ;
    last_button_state = read;
  }
  
  if(HAL_GetTick() - debounce_time > DEBOUNCE_TIME)
  {
    button_state = read ;
  }
  
  /* handle events */
  if(!button_state)
  {
    tick_button_unpress = HAL_GetTick();
    
    if(short_press_enabled == 0)
    {
      if(HAL_GetTick() - tick_button_press > LONG_PRESS_TIME)
      {
        button_long_press_cb();
        apply_short_press = 0;
        short_press_enabled = 1;        
      }
      else
      {
        apply_short_press = 1;
      }
    }
  }
  else
  {
    tick_button_press = HAL_GetTick();
    
    if(HAL_GetTick() - tick_button_unpress > SHORT_PRESS_TIME)
    {           
      if(apply_short_press)
      {
        button_short_press_cb();   
        apply_short_press = 0; 
      }
    }
    short_press_enabled = 0; 
  }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
