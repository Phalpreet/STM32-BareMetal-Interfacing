#include <stdio.h>
#include <stdint.h>
#include "stm32g0b1xx.h" 
#include "clock.h"
#include "timer.h"
#include "gpio.h"

static volatile uint16_t g_last_ccr = 0;
static volatile uint16_t g_delta_ticks = 0;
static volatile uint8_t g_new = 0;

static void TIM14_RTC_Capture_Init(void)
{
  RCC-> APBENR2 |= RCC_APBENR2_TIM14EN;

  //stop during config
  TIM14->CR1 = 0;

  //64 Mhz 
  TIM14->PSC = 0;
  TIM14->ARR = 0xFFFF;
  TIM14->EGR = TIM_EGR_UG;

  //CH1 as inpute, mapped to TI1 
  TIM14->CCMR1 &= ~(3u << TIM_CCMR1_CC1S_Pos);
  TIM14->CCMR1 |= (1u << TIM_CCMR1_CC1S_Pos); //ccis = 01

  //capture every 8 events: IC1PSC = 11
  TIM14->CCMR1 &= ~(3u << TIM_CCMR1_IC1PSC_Pos);
  TIM14->CCMR1 |=  (3u << TIM_CCMR1_IC1PSC_Pos);

  // Rising edge only (CC1P=0)
  TIM14->CCER &= ~(TIM_CCER_CC1P);
  
  //Select TI1 source = RTCCLK using TISEL
  //On stm32go , tim14->tisel tis1sel is 4 bits (bits 3:0).
  //0 = GPIO, 1=rtcclk .If capture never fires, we’ll verify/change this value.
  TIM14->TISEL &= ~0xFu; //1111
  TIM14->TISEL |=  0x1u;   // try 1 = RTCCLK

  // Enable capture
  TIM14->CCER |= TIM_CCER_CC1E;

  // Enable CC1 interrupt
  TIM14->DIER |= TIM_DIER_CC1IE;
  NVIC_EnableIRQ(TIM14_IRQn);

  // Start
  TIM14->CR1 |= TIM_CR1_CEN;
}

void TIM14_IRQHandler(void)
{
  if (TIM14->SR & TIM_SR_CC1IF)
  {
    uint16_t ccr = (uint16_t)TIM14->CCR1;
    TIM14->SR &= ~TIM_SR_CC1IF; // clear flag

    g_delta_ticks = (uint16_t)(ccr - g_last_ccr);
    g_last_ccr = ccr;
    g_new =1;

  }
}

static volatile uint32_t g_ms =0;
void SysTick_Handler(void){g_ms ++;}
static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;

  GPIO_InitAlternateF(GPIOA, 2u, 1);
  GPIO_InitAlternateF(GPIOA, 3u, 1);

  UART_Init(USART2, 38400u, 0);
}
int main(void)
{
  Clock_InitPll(PLL_64MHZ);
  SystemCoreClockUpdate();
  usart2_init_38400();

  TERM_ClearScreen(USART2);
  Clock_LSE_Init();
  
  SysTick_Config(SystemCoreClock/ 1000u); //1ms tick
  TIM14_RTC_Capture_Init();
  __enable_irq();
  uint32_t last_print =0;

  while(1)
  {
    
    if ((g_ms - last_print) >= 250u)
    {
      last_print += 250u;
      
      uint16_t dt;
      uint8_t new;
      dt = g_delta_ticks;
      new =g_new;
      g_new = 0;
      if(new)
      {
        int32_t err = (int32_t)dt - (int32_t)15625u;

        //ppm = err/ideal * 1e6
        int32_t ppm = (int32_t)((int64_t)err * 1000000LL)/ (int32_t)15625u;

        char line[120];
        snprintf(line, sizeof(line),
                 "dt=%u (ideal=%u) err=%ld ticks, ppm=%ld\r\n",
                 (unsigned)dt, (unsigned)15625u,
                 (long)err, (long)ppm);

        UART_TxStr(USART2, line);
      }
    }
    __WFI();
  }
}