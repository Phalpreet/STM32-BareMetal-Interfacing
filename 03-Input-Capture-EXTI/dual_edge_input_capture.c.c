#include "stm32g0b1xx.h"
#include <stdint.h>
#include <stdio.h>

#include "clock.h"
#include "gpio.h"

#define IC_PORT GPIOB
#define IC_PIN 14u //PB14
//Tim15_ch1 on pb14 = af5


//#define PartA
//#define PartB
#define  PartC

#ifdef PartA
static volatile uint16_t g_last_rise =0;
static volatile uint16_t g_last_fall= 0;
volatile uint32_t tim15_hits = 0;
static volatile uint16_t g_period_us = 0;
static volatile uint16_t g_high_us = 0; //latest measured  high time
static volatile uint8_t g_ready = 0; //new measurement ready

static volatile uint32_t g_ms= 0;

//sys handler
void SysTick_Handler(void)
{
  g_ms++;
}

//TIM15 input capture ISR
void TIM15_IRQHandler(void)
{
  if(TIM15->SR & TIM_SR_CC1IF)
  {
    tim15_hits++;
    uint16_t cap = (uint16_t)TIM15->CCR1;

    //clear capture flag
    TIM15->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
     
    //determine which edge just happened by reading pin level
    //if its high now - rising happened, low - falling edge happened
    uint32_t level = (IC_PORT->IDR >> IC_PIN) & 1u;

    if(level) //rising
    {
      uint16_t period = (uint16_t)(cap - g_last_rise);
      g_last_rise = cap;

      if(period != 0)
      {
        g_period_us = period;
        // "ready" only if we already have a recent high time too
        if (g_high_us != 0 && g_high_us < g_period_us)
        {g_ready = 1;}
      }
    }
    else//falling
    {
      //High time = falling - last rising
      uint16_t high = (uint16_t)(cap - g_last_rise);
      g_last_fall = cap;
      
      if(high != 0)
      {
        g_high_us = high;
        if(g_period_us != 0 && g_high_us < g_period_us)
        {
          g_ready=1;
        } 
      }
    } 
  }
}

static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;

  GPIO_InitAlternateF(GPIOA, 2u, 1);
  GPIO_InitAlternateF(GPIOA, 3u, 1);

  UART_Init(USART2, 38400u, 0);
}

static void tim15_input_capture_init_1us(void)
{
  //clocks
  RCC->IOPENR  |= RCC_IOPENR_GPIOBEN;
  RCC->APBENR2 |= RCC_APBENR2_TIM15EN;

  //PB14 as AF
  GPIO_InitAlternateF(GPIOB, 14, 5);
  
  //off timer
  TIM15->CR1 =0;
  
  //1MHz timer => 1 us per count
  uint32_t psc = (SystemCoreClock/ 1000000u) -1u;
  TIM15->PSC = (uint32_t)psc;

  //max arr?
  TIM15->ARR = 0xFFFF;

  //ch1 as input
  //mapped to TI1 => CC1S = 01
  TIM15->CCMR1 &= ~TIM_CCMR1_CC1S;
  TIM15->CCMR1 |= (1u << TIM_CCMR1_CC1S_Pos);

  //capture both rising and falling edges
  //CC1P and CC1NP = 1, both edges
  TIM15->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP);
  
  //enable capture
  TIM15-> CCER |= TIM_CCER_CC1E;
  
  //enable cc1 interrupt
  TIM15->DIER |= TIM_DIER_CC1IE;
  
  //NVIC
  NVIC_EnableIRQ(TIM15_IRQn);

  //Start Timer
  TIM15->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
  Clock_InitPll(PLL_40MHZ);
  SystemCoreClockUpdate();

  usart2_init_38400();
  TERM_ClearScreen(USART2);
  UART_TxStr(USART2, "TIM15 Inpute capture PB14/D6\r\n");

  //1ms systick
  SysTick_Config(SystemCoreClock/1000u);

  tim15_input_capture_init_1us();

  __enable_irq();

  uint32_t last_print =0;

  while(1)
  {
    if((g_ms-last_print)>= 250u)//every 250ms
    {
      last_print +=250u;

      uint16_t period, high;
      uint8_t ready;

      __disable_irq();
      period = g_period_us;
      high = g_high_us;
      ready = g_ready;
      g_ready =0;
      __enable_irq();

      if(ready && period!=0 && high < period)
      {
        //frequency 
        uint32_t freq = 1000000u / period;
        
        //duty
        uint32_t duty_x100 = ((uint32_t)high * 10000u ) / period;

        char line[120];
        snprintf(line, sizeof(line),"T=%u us, HIGH=%u us, f=%lu Hz, duty=%lu.%02lu%%\r\n",
        (unsigned) period,
        (unsigned) high,
        (unsigned long) freq,
        (unsigned long) (duty_x100 / 100u),
        (unsigned long) (duty_x100 % 100u));

        UART_TxStr(USART2, line);
      }
    }

    __WFI();
  }

}
#endif

#ifdef PartB
static volatile uint32_t g_ms =0;

static volatile uint16_t g_last_fall = 0;

static volatile uint16_t g_period_us = 0;
static volatile uint16_t g_high_us = 0;
static volatile uint8_t g_ready = 0;
static volatile uint32_t g_cc2hits = 0;


void SysTick_Handler(void)
{
  g_ms++;
}

void TIM15_IRQHandler(void)
{
  //as we only enables cc2 interrupts, we only check cc2if
  if(TIM15->SR & TIM_SR_CC2IF)
  {
    g_cc2hits++;

    uint16_t fall = (uint16_t)TIM15->CCR2; //falling timestamp
    uint16_t rise = (uint16_t)TIM15->CCR1; //recent rising timestamp

    //cleared flags
    TIM15->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF ); 

    //high time : falling - rising
    uint16_t high = (uint16_t)(fall -rise);

    //period : falling - previous falling
    uint16_t period = (uint16_t) (fall - g_last_fall);
    g_last_fall = fall;

    //validation
    if(period != 0 && high < period)
    {
      g_high_us = high;
      g_period_us = period;
      g_ready = 1;
    }
  }
}

static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;

  GPIO_InitAlternateF(GPIOA, 2u, 1u);
  GPIO_InitAlternateF(GPIOA, 3u, 1u);

  UART_Init(USART2, 38400u, 0);
}

static void tim15_pwn_input_dual_capture_init_1us(void)
{
  //clocks
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
  RCC->APBENR2 |= RCC_APBENR2_TIM15EN;

  //pb14 as af
  GPIO_InitAlternateF(IC_PORT, IC_PIN, 5u);

  //timer off during config
  TIM15->CR1 = 0;

  //1MHz tick => 1us resolution
  TIM15->PSC = (uint16_t)((SystemCoreClock / 1000000u)-1u);
  TIM15-> ARR = 0xFFFF; //max value for unsigned 16 bit integer
  TIM15->EGR = TIM_EGR_UG; //loads PSC/ARR

  //capture config
  //CH1 input on TI1 (CC1S = 01)
  TIM15-> CCMR1 &= ~(3u << TIM_CCMR1_CC1S_Pos);
  TIM15-> CCMR1 |= (1u << TIM_CCMR1_CC1S_Pos);

  //Ch2 input also on TI1(CC2S = 10)
  TIM15-> CCMR1 &= ~(3u << TIM_CCMR1_CC2S_Pos);
  TIM15-> CCMR1 |= (2u << TIM_CCMR1_CC2S_Pos);
  
  //CH1 rising (CC1p=0)
  //CH2 falling (CC2p=1)
  TIM15->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC2NP);
  TIM15->CCER |= (TIM_CCER_CC2P);

  //enable captures
  TIM15->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

  //clear stale flags
  TIM15->SR = 0;

  //Interrupt enable only on CH2 
  TIM15->DIER |= TIM_DIER_CC2IE;

  NVIC_EnableIRQ(TIM15_IRQn);


  //Start
  TIM15->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
  Clock_InitPll(PLL_40MHZ);
  SystemCoreClockUpdate();

  usart2_init_38400();
  TERM_ClearScreen(USART2);
  UART_TxStr(USART2, "TIM15 Part B : Dual Capture (CH1 rise, CH2 fall)\r\n");

  SysTick_Config(SystemCoreClock/ 1000u);

  tim15_pwn_input_dual_capture_init_1us();
  __enable_irq();

  uint32_t last_print = 0;
  while(1)
  {
    if((g_ms - last_print) >= 250)
    {
      last_print += 250;

      uint16_t period, high;
      uint8_t ready;

      period = g_period_us;
      high = g_high_us;
      ready = g_ready;
      g_ready = 0;

      if(ready && period != 0 && high < period)
      {
        uint32_t freq = 1000000 / period;
        uint32_t duty_x100 = ((uint32_t)high * 10000) / period;

        char line[120];

        snprintf(line, sizeof(line), "T=%u us, High=%u us, f=%lu Hz, duty=%lu.%02lu%%\r\n",
                (unsigned)period,
                 (unsigned)high,
                 (unsigned long)freq,
                 (unsigned long)(duty_x100 / 100u),
               (unsigned long)(duty_x100 % 100u));
      UART_TxStr(USART2, line);

      }
    }

    __WFI();
  }

}
#endif
#ifdef PartC
static volatile uint32_t g_ms =0;

static volatile uint32_t g_ovf = 0; // this will count timer overflows
//or how many times the 16 bit timer wrapped
static volatile uint32_t g_last_fall_ts = 0; //last falling-edge timestamp 32bit

static volatile uint32_t g_period_us = 0;
static volatile uint32_t g_high_us = 0;
static volatile uint8_t g_ready = 0;
static volatile uint32_t g_cc2hits = 0;


void SysTick_Handler(void)
{
  g_ms++;
}

void TIM15_IRQHandler(void)
{
  uint32_t sr = TIM15->SR;
  //if overflow happened 
  //this becomes 1 when CNT overflows from 65335 -> 0
  if(sr & TIM_SR_UIF)
  {
    TIM15->SR &= ~TIM_SR_UIF; //clear update flag
    g_ovf++; //wrapped once
  }
  //as we only enables cc2 interrupts, we only check cc2if
  //(falling edge capture handling
  if(sr & TIM_SR_CC2IF)
  {
    g_cc2hits++;

    uint16_t fall = (uint16_t)TIM15->CCR2; //falling timestamp
    uint16_t rise = (uint16_t)TIM15->CCR1; //recent rising timestamp

    //cleared flags(acknowledge interrupt)
    TIM15->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF ); 
    
    //snap the overflow counter
    uint32_t ovf = g_ovf;

    //Edge case 
    //if the timer overflowed very close to this capture, UIF might still be set,
    //and captured ccr might be in next wrap
    //if uif is still and ccr is small, treat it as if overflow happened before the capture
    if((sr & TIM_SR_UIF) && (fall < 0x8000u)) //fall < 32768(less than half)
    {
      ovf++;
    }
    //32 bit timestamps 
    uint32_t ts_fall = (ovf << 16) | fall;
    
    //if the rise was before the wrap but fall after wrap, rise will be > fall

    uint32_t ts_rise;
    if(rise > fall)
    {
      ts_rise = ((ovf -1u) << 16) | rise;
    }
    else
    {
      ts_rise = (ovf << 16) | rise;
    }

    //high time : falling - rising
    uint32_t high = ts_fall -ts_rise;

    //period : falling - previous falling
    uint32_t period = ts_fall - g_last_fall_ts;
    g_last_fall_ts = ts_fall;

    //validation
    if(period != 0 && high < period)
    {
      g_high_us = high;
      g_period_us = period;
      g_ready = 1;
    }
  }
}

static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;

  GPIO_InitAlternateF(GPIOA, 2u, 1u);
  GPIO_InitAlternateF(GPIOA, 3u, 1u);

  UART_Init(USART2, 38400u, 0);
}

static void tim15_pwn_input_dual_capture_init_1us(void)
{
  //clocks
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
  RCC->APBENR2 |= RCC_APBENR2_TIM15EN;

  //pb14 as af
  GPIO_InitAlternateF(IC_PORT, IC_PIN, 5u);

  //timer off during config
  TIM15->CR1 = 0;

  //1MHz tick => 1us resolution
  TIM15->PSC = (uint16_t)((SystemCoreClock / 1000000u)-1u);
  TIM15-> ARR = 0xFFFF; //max value for unsigned 16 bit integer
  TIM15->EGR = TIM_EGR_UG; //loads PSC/ARR

  //capture config
  //CH1 input on TI1 (CC1S = 01)
  TIM15-> CCMR1 &= ~(3u << TIM_CCMR1_CC1S_Pos);
  TIM15-> CCMR1 |= (1u << TIM_CCMR1_CC1S_Pos);

  //Ch2 input also on TI1(CC2S = 10)
  TIM15-> CCMR1 &= ~(3u << TIM_CCMR1_CC2S_Pos);
  TIM15-> CCMR1 |= (2u << TIM_CCMR1_CC2S_Pos);
  
  //CH1 rising (CC1p=0)
  //CH2 falling (CC2p=1)
  TIM15->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC2NP);
  TIM15->CCER |= (TIM_CCER_CC2P);

  //enable captures
  TIM15->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);

  //clear stale flags
  TIM15->SR = 0;

  //Interrupt enable only on CH2 
  TIM15->DIER |= TIM_DIER_CC2IE;
  TIM15->DIER |= TIM_DIER_UIE; //enables overflow/update interrupt

  NVIC_EnableIRQ(TIM15_IRQn);


  //Start
  TIM15->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
  Clock_InitPll(PLL_40MHZ);
  SystemCoreClockUpdate();

  usart2_init_38400();
  TERM_ClearScreen(USART2);
  UART_TxStr(USART2, "TIM15 Part B : Dual Capture (CH1 rise, CH2 fall)\r\n");

  SysTick_Config(SystemCoreClock/ 1000u);

  tim15_pwn_input_dual_capture_init_1us();
  __enable_irq();

  uint32_t last_print = 0;
  while(1)
  {
    if((g_ms - last_print) >= 250)
    {
      last_print += 250;

      uint32_t period, high;
      uint8_t ready;

      period = g_period_us;
      high = g_high_us;
      ready = g_ready;
      g_ready = 0;

      if(ready && period != 0 && high < period)
      {
        uint32_t freq_mHz = (1000000*1000) / period;
        uint32_t duty_x100 = ((uint32_t)high * 10000) / period;

        char line[120];

        snprintf(line, sizeof(line), "T=%u us, High=%u us, f=%lu.%03lu Hz, duty=%lu.%02lu%%\r\n",
                (unsigned)period,
                 (unsigned)high,
                 (unsigned long)(freq_mHz/1000),
                  (unsigned long)(freq_mHz%1000),
                 (unsigned long)(duty_x100 / 100u),
               (unsigned long)(duty_x100 % 100u));
      UART_TxStr(USART2, line);

      }
    }

    __WFI();
  }

}
#endif