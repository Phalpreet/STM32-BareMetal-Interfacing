#include "stm32g0b1xx.h"
#include <stdint.h>
#include <stdio.h>

//mY
#include "clock.h"
#include "gpio.h"

#define UART_TX  2u //PA2
#define UART_RX  3u //PA3

//#define PartA
//#define PartB
#define PartC

//usart2 init
static void usart2_init_9600(void)
{
  RCC-> APBENR1 |= RCC_APBENR1_USART2EN;

  GPIO_InitAlternateF(GPIOA, UART_TX, 1u);
  GPIO_InitAlternateF(GPIOA, UART_RX, 1u);

  UART_Init(USART2, 9600u, 0);

}
#ifdef PartA
#define SCOPE_PIN 9u
#define SCOPE_PORT GPIOD //PD9 for scope 


//systick ISR fires 1 ms and does:
void SysTick_Handler(void)
{
  GPIO_Toggle(SCOPE_PORT, SCOPE_PIN);
}



int main(void)
{
  //clock to 40MHz
    printf("Before Clock_InitPll %d\r\n", SystemCoreClock);
  Clock_InitPll(PLL_40MHZ);
  printf("after Clock_InitPll %d\r\n", SystemCoreClock);
  //uart hello
  usart2_init_9600();
  UART_TxStr(USART2, "Hello SysTick will now toggle PD9 every 1ms.\r\n");

  //pd9 pin as output
  GPIO_InitOutput(SCOPE_PORT, SCOPE_PIN);

  //systic every 1ms
  SysTick_Config(SystemCoreClock/1000u);
  printf("After Systick_Config %d\r\n", SystemCoreClock);
  __enable_irq();
  
  while(1)
  {
    //empty

  }
}
#endif
#ifdef PartB

static volatile uint16_t g_ms = 0;
static volatile uint32_t g_sec = 0; //total seconds since start

void  SysTick_Handler(void)
{
  g_ms++;
  if(g_ms >= 1000u)
  {
    g_ms = 0;
    g_sec++;
  }
}

int main(void)
{
  Clock_InitPll(PLL_40MHZ);

  SystemCoreClockUpdate();

  usart2_init_9600();
  UART_TxStr(USART2, "Part B: SysTick timekeeper\r\n");
  
  SysTick_Config(SystemCoreClock/1000u); //systick every 1ms
  __enable_irq();

  uint32_t last_print_sec = 0xFFFFFFFFu;
  while(1)
  {
    uint32_t sec;
    uint16_t ms;

    __disable_irq();
    sec = g_sec;
    ms = g_ms;
    __enable_irq();
    

    //update only when second changes
    if(sec != last_print_sec)
    {
      last_print_sec = sec;

      uint32_t days = sec/ 86400u;
      uint32_t hours = (sec / 3600u) % 24u;
      uint32_t mins = (sec/ 60u) % 60u;
      uint32_t secs = sec % 60u;

      char line[80];
      snprintf(line, sizeof(line), "\r%lu %02lu:%02lu.%03u\x1b[K",
      (unsigned long)days,
      (unsigned long)hours,
      (unsigned long)mins,
      (unsigned long)secs,
      (unsigned)ms);

      UART_TxStr(USART2, line);

    }

    __WFI(); //sleep untill next interrupt
  }

}
#endif

#ifdef PartC

static volatile uint32_t g_ms_total = 0; // global ms count (never resets)
static volatile uint16_t g_ms = 0;       // 0..999
static volatile uint32_t g_sec = 0;      // seconds since boot

void  SysTick_Handler(void)
{
  g_ms_total++;
  g_ms++;
  if(g_ms >= 1000u)
  {
    g_ms = 0;
    g_sec++;
  }
}

int main(void)
  {
  Clock_InitPll(PLL_40MHZ);
  SystemCoreClockUpdate();

  usart2_init_9600();

  TERM_ClearScreen(USART2);

  SysTick_Config(SystemCoreClock/1000u);
  __enable_irq();

  uint32_t last_print_sec = 0xFFFFFFFFu;
  while(1)
  {
    uint32_t sec;
    uint16_t ms;
    __disable_irq();
    sec = g_sec;
    ms = g_ms;
    __enable_irq();
    //render only once per second
    if(sec != last_print_sec)
    {
      last_print_sec =sec;

      //snapshot before preparing buffer
      uint32_t t0 = g_ms_total;

      //Build elasped time string
      uint32_t days  = sec / 86400u;
      uint32_t hours = (sec / 3600u) % 24u;
      uint32_t mins  = (sec / 60u) % 60u;
      uint32_t secs  = sec % 60u;

      char time_line[80];
      snprintf(time_line, sizeof(time_line),
               "%lu %02lu:%02lu:%02lu.%03u",
               (unsigned long)days,
               (unsigned long)hours,
               (unsigned long)mins,
               (unsigned long)secs,
               (unsigned)ms);
      
      //Print time line 
      TERM_GotoXY(USART2, 1, 1);
      UART_TxStr(USART2, time_line);

      //snapshot after 
      uint32_t t1 = g_ms_total;
      uint32_t dt = t1 -t0;

      //In second row
      char perf_line[80];
      snprintf(perf_line, sizeof(perf_line), "Render time: %lu ms", (unsigned long)dt);
      
      TERM_GotoXY(USART2, 1, 2);
      UART_TxStr(USART2, perf_line);

    }
    
    __WFI();
  }

}

#endif
