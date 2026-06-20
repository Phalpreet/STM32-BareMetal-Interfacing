/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include "gpio.h"
#include "clock.h"
#include "timer.h"

volatile uint16_t dacValue =0;
volatile uint8_t waveMode = 0;   // 0 = ramp up, 1 = ramp down
volatile uint16_t msCounter =0;
volatile uint8_t update = 0;
volatile uint8_t triDirection = 1;    // 1 = up, 0 = down
/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) 
{

  //Initialize
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN |RCC_IOPENR_GPIOBEN |RCC_IOPENR_GPIOCEN ;
  RCC->APBENR1 |= RCC_APBENR1_TIM6EN;
  RCC->APBENR1 |= RCC_APBENR1_DAC1EN;
  
  Clock_InitPll(PLL_40MHZ);
  printf("SYSCLK = %u Hz\n", SystemCoreClock);

  GPIO_InitOutput(GPIOA, 5);
  GPIO_InitInputPullUp(GPIOC, 13); //pc13 pull up
  SysTick_Config(SystemCoreClock /1000);
  /*   DAC SETTINGS  */
  //PA4 Analog Mode
  GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
  GPIOA->MODER |= GPIO_MODER_MODE4_Msk;
  
  //Disble DAC
  DAC1->CR &= ~DAC_CR_EN1;
  //Enable Output buffer
  DAC1->MCR &= ~DAC_MCR_MODE1_Msk; 
  DAC1->DHR12R1 = 0;
  //Enable DAC
  DAC1->CR |= DAC_CR_EN1;
 
  //TIM6 
  Timer_Init(TIM6, 39,999);
  Timer_EnableInterrupt(TIM6, TIM6_DAC_LPTIM1_IRQn, TimUIE); 
  Timer_SetEnable(TIM6, 1);
  
  uint8_t lastButton = 1;
  while (1)
  {
    uint8_t currentButton = GPIO_Read(GPIOC, 13);
    if(lastButton == 1 && currentButton ==0)
    {
      if(update)
      {
        update = 0;
        waveMode++;
        if (waveMode == 0)
        {
            dacValue = 0;
        }
        else if (waveMode == 1)
        {
            dacValue = 4095;
        }
        else if (waveMode == 2)
        {
            dacValue = 0;
            triDirection = 1;   // start triangle going up
        }
        else 
        {
          waveMode = 0;
        }
      }      
    }
    lastButton = currentButton;
    
  }
}

void SysTick_Handler(void)
{
  if(msCounter++ > 30)
  {
    update =1;
    msCounter =0;
  }
}

void TIM6_DAC_LPTIM1_IRQHandler(void)
{
  if(TIM6->SR & TIM_SR_UIF)
  {
    TIM6->SR &= ~TIM_SR_UIF;
    DAC1->DHR12R1 = dacValue;

    if(waveMode == 0)
    {
      dacValue+= 16;
      if(dacValue > 4095)
      {
        dacValue = 0;
      }
    }
    else if(waveMode == 1)
    {
      if(dacValue < 16)
      {
        dacValue = 4095;
      }
      else
      {
        dacValue -= 16;
      }
    }
    else 
    {
      if(triDirection)
      {
        if(dacValue >= 4095-16)
        {
          dacValue = 4095;
          triDirection = 0; //down
        }
        else
        {
          dacValue+=16;
        }
      }
      else
      {
        if(dacValue < 16)
        {
          dacValue =0;
          triDirection = 1;
        }
        else
        {
          dacValue -= 16;
        }
      }
    }
    
  } 
  NVIC_ClearPendingIRQ(TIM6_DAC_LPTIM1_IRQn);
}

/*************************** End of file ****************************/
