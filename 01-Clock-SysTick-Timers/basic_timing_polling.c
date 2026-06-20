#include <stdint.h>
#include "stm32g0b1xx.h"
#include <stdio.h>
#include "clock.h"
#include "timer.h"
#include "gpio.h"

//#define PartA
//#define PartB
#define PartC


#ifdef PartA
  #define LED_PORT GPIOA
  #define LED_PIN  5u

 static void tim6_init_25ms(void)
{
  //We will make TIM6 tick at 1kHz(1ms per tick), then ARR = 24(25ms update)
  // timer_update_period = (PSC+1) * (ARR+1) / TIMCLK

  uint32_t psc = (SystemCoreClock / 1000u) -1u; //1ms tick
  uint32_t arr = 25u- 1u; //25ms

  Timer_Init(TIM6, (uint16_t  )psc, (uint16_t) arr);
  Timer_SetEnable(TIM6, 1);
}

  
  int main(void)
  { 
    Clock_InitPll(PLL_64MHZ); 

    //led pin
    GPIO_InitOutput(LED_PORT, LED_PIN);

    tim6_init_25ms();  // TIM6 @ 25ms update (polling)

    while(1)
    {
      //Poll for update event
      if(TIM6->SR & TIM_SR_UIF)
      {
        TIM6->SR &= ~TIM_SR_UIF; //clear flag
        GPIO_Toggle(LED_PORT, LED_PIN); //toggle PA5
      }
    }
  }
#endif

#ifdef  PartB

#define LED_PIN   5u   // PA5 = LD4
#define SCOPE_PIN 6u   // PA6 = scope pin
#define UART_TX   2u   // PA2 = USART2_TX
#define UART_RX   3u   // PA3 = USART2_RX
#define UART_AF   1u   

static volatile uint32_t g_tim6_flag =0; //event 
static volatile uint32_t g_tim6_count = 0; //total interrupts

static void tim6_init_25ms_irq(void)
{
  //We will make TIM6 tick at 1kHz(1ms per tick), then ARR = 24(25ms update)
  // timer_update_period = (PSC+1) * (ARR+1) / TIMCLK

  uint32_t psc = (SystemCoreClock / 1000u) -1u; //1ms tick
  uint32_t arr = 25u- 1u; //25ms

  Timer_Init(TIM6, (uint16_t  )psc, (uint16_t) arr);

  //Enable TIM6 update interrupt
  TIM6->DIER |= TIM_DIER_UIE;

  //Clear stale UIF
  TIM6->SR &= ~TIM_SR_UIF;

  //Enable NVIC line
  NVIC_ClearPendingIRQ(TIM6_DAC_LPTIM1_IRQn);
  NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);

  Timer_SetEnable(TIM6, 1);
}

static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN ; //enable usart2 peripheral clock

  GPIO_InitAlternateF(GPIOA, UART_TX, UART_AF);
  GPIO_InitAlternateF(GPIOA, UART_RX, UART_AF);

  UART_Init(USART2, 38400u, 0);
}


//ISR for  TIM6 update event
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
  //check UIF (update interrupt flag
  if(TIM6-> SR & TIM_SR_UIF)
  {
    TIM6->SR &= ~TIM_SR_UIF; //cleat UIF

    //toggle PA6 for scope
    uint32_t mask = 1u <<SCOPE_PIN;
    if(GPIOA->ODR & mask){ GPIOA->BSRR = (mask << 16);} //reset
    else {GPIOA->BSRR = mask;} //set
  
    //increment flag
    g_tim6_flag++;
    g_tim6_count++;  
  }
}

int main(void)
{
  //64MHz
  Clock_InitPll(PLL_64MHZ);

  //Outputs
  GPIO_InitOutput(GPIOA, LED_PIN);
  GPIO_InitOutput(GPIOA, SCOPE_PIN);

  //UART2 @ 38400
  usart2_init_38400();

  //TIM6 25ms interrupt
  tim6_init_25ms_irq();

  __enable_irq();

  uint32_t last_colon_count = 0;

  while(1)
  {
  //Consume pending
    if(g_tim6_flag)
    {
      __disable_irq();
      g_tim6_flag--; //consume 1
      __enable_irq();

      //toggle PA5 
      uint32_t mask = 1u <<LED_PIN;
      if(GPIOA->ODR & mask){ GPIOA->BSRR = (mask << 16);} //reset
      else {GPIOA->BSRR = mask;} //set
    }

    //every 100 interrupts, send ':'
    if((g_tim6_count - last_colon_count)>= 100u)
    {
      last_colon_count += 100u;
      UART_TxByte(USART2,(uint8_t) ':');
    }
  }
}
#endif

#ifdef  PartC

#define LED_PIN   5u   // PA5 = LD4
#define SCOPE_PIN 6u   // PA6 = scope pin
#define UART_TX   2u   // PA2 = USART2_TX
#define UART_RX   3u   // PA3 = USART2_RX
#define UART_AF   1u   

static volatile uint32_t g_tim6_flag =0; //event 
static volatile uint32_t g_tim6_count = 0; //total interrupts

static void tim6_init_25ms_irq(void)
{
  //We will make TIM6 tick at 1kHz(1ms per tick), then ARR = 24(25ms update)
  // timer_update_period = (PSC+1) * (ARR+1) / TIMCLK

  uint32_t psc = (SystemCoreClock / 1000u) -1u; //1ms tick
  uint32_t arr = 25u- 1u; //25ms

  Timer_Init(TIM6, (uint16_t  )psc, (uint16_t) arr);

  //Enable TIM6 update interrupt
  TIM6->DIER |= TIM_DIER_UIE;

  //Clear stale UIF
  TIM6->SR &= ~TIM_SR_UIF;

  //Enable NVIC line
  NVIC_ClearPendingIRQ(TIM6_DAC_LPTIM1_IRQn);
  NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);

  Timer_SetEnable(TIM6, 1);
}

static void usart2_init_38400(void)
{
  RCC->APBENR1 |= RCC_APBENR1_USART2EN ; //enable usart2 peripheral clock

  GPIO_InitAlternateF(GPIOA, UART_TX, UART_AF);
  GPIO_InitAlternateF(GPIOA, UART_RX, UART_AF);

  UART_Init(USART2, 38400u, 0);
}


//ISR for  TIM6 update event
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
  //check UIF (update interrupt flag
  if(TIM6-> SR & TIM_SR_UIF)
  {
    TIM6->SR &= ~TIM_SR_UIF; //cleat UIF

    //toggle PA6 for scope
    uint32_t mask = 1u <<SCOPE_PIN;
    if(GPIOA->ODR & mask){ GPIOA->BSRR = (mask << 16);} //reset
    else {GPIOA->BSRR = mask;} //set
  
    //increment flag
    g_tim6_flag++;
    g_tim6_count++;  
  }
}

int main(void)
{
  //64MHz
  Clock_InitPll(PLL_64MHZ);

  //Outputs
  GPIO_InitOutput(GPIOA, LED_PIN);
  GPIO_InitOutput(GPIOA, SCOPE_PIN);

  //UART2 @ 38400
  usart2_init_38400();

  //TIM6 25ms interrupt
  tim6_init_25ms_irq();

  __enable_irq();

  uint32_t last_colon_count = 0;
  
  while(1)
  {
    //__WFI();
  //Consume pending
    if(g_tim6_flag)
    {
     // __disable_irq(); 
      g_tim6_flag--; //consume 1
     // __enable_irq();

      //toggle PA5 
      uint32_t mask = 1u <<LED_PIN;
      if(GPIOA->ODR & mask){ GPIOA->BSRR = (mask << 16);} //reset
      else {GPIOA->BSRR |= mask;} //set
    }

    //every 100 interrupts, send ':'
    if((g_tim6_count - last_colon_count)>= 100u)
    {
      last_colon_count += 100u;
      UART_TxByte(USART2,(uint8_t) ':');
    }
  }
}
#endif
