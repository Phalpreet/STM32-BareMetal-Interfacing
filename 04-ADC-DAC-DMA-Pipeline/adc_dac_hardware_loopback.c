/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include <stdint.h>
#include "stm32g0b1xx.h"
#include "clock.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "dac.h"


volatile uint16_t g_adcValue = 0;
volatile uint16_t g_pwmValue = 0;
volatile uint8_t g_update = 0;
volatile uint16_t g_invValue = 0;
/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) 
{
  //Clock
  RCC->APBENR1 |= RCC_APBENR1_PWREN;
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;

  Clock_InitPll(PLL_40MHZ);
  printf("SysCLK = %u Hz\n", SystemCoreClock);


  //Led
  GPIO_InitOutput(GPIOA, 5);
  GPIO_Clear(GPIOA, 5);

  //DAC CH1 Setup 
  //DAC output pin = PA4
  DAC1_CH1_Init();
  DAC1_CH1_EnableBuffer();
  DAC1_CH1_Enable();

  //DAC CH2 Setup
  //pa5
  DAC1_CH2_Init();
  DAC1_CH2_EnableBuffer();
  DAC1_CH2_Enable();

  //ADC input pin setup
  //AN0 = ADC_IN0 = PA0
  GPIOA->MODER &= ~GPIO_MODER_MODE0_Msk;
  GPIOA->MODER |=  GPIO_MODER_MODE0_Msk;

  //TIM 14 CH1 on PA7
  // PWN output setup

  GPIO_InitAlternateF(GPIOA, 7, 4);

  //TIM14 Setup
  //psc = 0
  //arr = 999
  //pwm freq = 40MHz / 1000 40kHz
  Timer_Init(TIM14, 0, 999);
  Timer_SetupChannel(TIM14, TimCCR1, Pwm1);
  Timer_WriteCCR(TIM14, TimCCR1, 0);

 // Enable CH1 output
  TIM14->CCER |= TIM_CCER_CC1E;

  // Load registers and start timer
  TIM14->EGR |= TIM_EGR_UG;
  Timer_SetEnable(TIM14, 1);

  //ADC Setup
  ADC1_Init();
  ADC1_SetPrescalerDiv10();
  ADC1_EnableVref();

  ADC1_SelectChannels(ADC_CHSELR_CHSEL0);

  //External Trigger = TIM6_TGRO, rising edge
  ADC1_SetExternalTrigger(ADC_TRIGGER_TRG5, ADC_TRIGGER_RISING);

  ADC1_Calibrate();
  ADC1_Enable();

  ADC1_EnableEOCInterrupt();
  NVIC_EnableIRQ(ADC1_COMP_IRQn);

  //TIM6 Setup
  //40 MHz 
  // PSC = 39 -> timer tick - 1MHz
  // ARR = 99 -> update every 100us
  // Sample Rate = 10Khz
  Timer_Init(TIM6, 39,99);
  
  // TIM6 TRGO on update event
  TIM6->CR2 &= ~TIM_CR2_MMS_Msk;
  TIM6->CR2 |= TIM_CR2_MMS_1;   // MMS = 010 -> update event as TRGO

  //Arm ADC for External trigger
  ADC1_StartConversion();
  
  //TIM6 Start
  Timer_SetEnable(TIM6, 1);
  while(1)
  {
    
  }
}


//ADC Interrupt
//Read ADC result and write same value to DAC
void ADC1_COMP_IRQHandler(void)
{
  if (ADC1->ISR & ADC_ISR_EOC)
    {
        g_adcValue = (uint16_t)ADC1->DR;   // reading DR clears EOC
        DAC1_CH1_Write(g_adcValue);  
        g_pwmValue = (uint16_t)(((uint32_t)g_adcValue * 999) / 4095U);
        // Inverted output
        g_invValue = 4095U - g_adcValue;
        DAC1_CH2_Write(g_invValue);
        //update pwm duty cycle
        Timer_WriteCCR(TIM14, TimCCR1, g_pwmValue);
        GPIO_Toggle(GPIOA, 5);            
    }

    NVIC_ClearPendingIRQ(ADC1_COMP_IRQn);
}

/*************************** End of file ****************************/
