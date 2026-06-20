//#define Part_A
//#define Part_B
#define Part_C
#include "stdio.h"
#include "stdint.h"
#include "stm32g0b1xx.h"
#include "clock.h"
#include "gpio.h"

#define Counter_Max 9999U //highest display value before wrapping
#define  Debounce_Ms 30U //button bounce filter time
#define Auto_Period_Ms 1000U // 1sec
#define Long_Press_Ms 3000U //3sec

#define LD4_PORT         GPIOA   // On-board LED port
#define LD4_PIN          5U      // On-board LED pin

#define USER_PORT        GPIOC   // User button port
#define USER_PIN         13U     // User button pin (PC13)

#define SW1_PORT         GPIOD   // SW1 port
#define SW1_PIN          8U      // SW1 pin (PD8)

#define SW4_PORT         GPIOD   // SW4 port
#define SW4_PIN          9U      // SW4 pin (PD9)


volatile uint16_t g_counter = 0; //current displayed value
volatile uint8_t g_displayUpdateNeeded = 1; //to refresh 7-seg display
volatile uint32_t g_msTicks = 0; //1ms sys tick counter

volatile uint32_t g_lastSW1Ms = 0; //for bounce
volatile uint32_t g_lastSW4Ms = 0; //for bounce
volatile uint32_t g_lastUserMs = 0; //for bounce

// This function maps an EXTI line to a GPIO port.
// On STM32G0 the EXTICR field is selected by line number.
// Port codes are: A=0, B=1, C=2, D=3, ...''
static void EXTI_MapLineToPort(uint8_t line, uint8_t portCode)
{
    uint32_t regIndex   = line / 4U;                 // Which EXTICR register holds this line
    uint32_t fieldShift = (line % 4U) * 8U;          // Each line uses one 8-bit field in G0 EXTICR

    EXTI->EXTICR[regIndex] &= ~(0xFFUL << fieldShift);         // Clear the old port selection for this line
    EXTI->EXTICR[regIndex] |=  ((uint32_t)portCode << fieldShift); // Write the new port selection
}

// This helper enables a falling-edge interrupt for one EXTI line.
static void EXTI_EnableFallingEdge(uint8_t line)
{
    EXTI->RTSR1 &= ~(1UL << line);   // Disable rising edge trigger on this line
    EXTI->FTSR1 |=  (1UL << line);   // Enable falling edge trigger on this line
    EXTI->IMR1  |=  (1UL << line);   // Unmask the interrupt so it can reach the NVIC
}

// This helper enables both falling and rising edges for one EXTI line.
static void EXTI_EnableBothEdges(uint8_t line)
{
    EXTI->RTSR1 |= (1UL << line);    // Enable rising edge trigger on this line
    EXTI->FTSR1 |= (1UL << line);    // Enable falling edge trigger on this line
    EXTI->IMR1  |= (1UL << line);    // Unmask the interrupt so it can reach the NVIC
}

// This helper increments the counter with the required wrap.
static void Counter_Increment(void)
{
    if (g_counter >= Counter_Max)    // If already at 9999
    {
        g_counter = 0;               // Wrap forward back to 0000
    }
    else
    {
        g_counter++;                 // Otherwise increment by one
    }
}


static void Counter_Decrement(void)
{
    if (g_counter == 0)              // If already at 0000
    {
        g_counter = Counter_Max;     // Wrap backward to 9999
    }
    else
    {
        g_counter--;                 // Otherwise decrement by one
    }
}

static void App_InitBase(void)
{
    Clock_InitPll(PLL_40MHZ);                     // Run the MCU from the 40 MHz PLL
    SysTick_Config(SystemCoreClock / 1000U);     // Make SysTick interrupt every 1 ms
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    GPIO_InitAlternateF(GPIOA , 9, 1);
    UART_Init(USART1, 9600, 0);
    GPIO_InitOutput(LD4_PORT, LD4_PIN);          // Configure LD4 pin as output
    GPIO_Clear(LD4_PORT, LD4_PIN);               // Start with LD4 off

    GPIO_InitInputPullUp(USER_PORT, USER_PIN);   // Configure PC13 as input with pull-up
    GPIO_InitInputPullUp(SW1_PORT, SW1_PIN);     // Configure PD8 as input with pull-up
    GPIO_InitInputPullUp(SW4_PORT, SW4_PIN);     // Configure PD9 as input with pull-up
}

void Display_WriteNumber4(uint16_t value)
{
    char display_buffer[6];
    sprintf(display_buffer, "#%04u", value);
    UART_TxStr(USART1, display_buffer);
}
// SysTick runs in all three parts because:
// - it gives us bounce filtering time
// - Part B needs 1-second timing
// - Part C needs 3-second hold timing


#ifdef Part_A
//sw4 increments pd9
//sw1 decrements pd8
// Counter wraps between 0000 and 9999
void SysTick_Handler(void)
{
    g_msTicks++;                                  // Advance the 1 ms tick counter
}
void EXTI4_15_IRQHandler(void)
{
  
  // Check whether EXTI8 (PD8 / SW1) has a falling-edge pending flag
  if(EXTI -> FPR1 & (1UL << 8)) // EXTI_FPR1_FPIF8
  {
    //clear the pending flag
    EXTI->FPR1 = (1UL << 8); //EXTI_FPR1_FPIF8

    if((g_msTicks - g_lastSW1Ms) >= Debounce_Ms)// Accept this press only if enough time passed
    {
      // Record the accepted press time
      g_lastSW1Ms = g_msTicks;
      Counter_Decrement();                              // Decrement the counter with wrap
      g_displayUpdateNeeded = 1;  // Tell main to refresh the 7-seg display

    }

  }

  // Check whether EXTI9 (PD9 / SW4) has a falling-edge pending flag
  if(EXTI->FPR1 & (1UL << 9))
  {
    EXTI->FPR1 = (1UL << 9);
    if ((g_msTicks - g_lastSW4Ms) >= Debounce_Ms)
    {
      g_lastSW4Ms = g_msTicks;                          // Record the accepted press time
      Counter_Increment();                              // Increment the counter with wrap
      g_displayUpdateNeeded = 1;
    }

  }
}

int main(void)
{
  uint16_t localCounter;
  App_InitBase();

  EXTI_MapLineToPort(8U, 3U);  // Map EXTI8 to port D (PD8)
  EXTI_MapLineToPort(9U, 3U);  // Map EXTI9 to port D (PD9)
  EXTI_EnableFallingEdge(8U);  // Trigger SW1 on falling edge
  EXTI_EnableFallingEdge(9U); // Trigger SW4 on falling edge
  
  NVIC_EnableIRQ(EXTI4_15_IRQn);   
  while(1)
  {
    if(g_displayUpdateNeeded)
    {
      __disable_irq();                                  // Briefly block interrupts while copying shared state
      localCounter = g_counter;                         // Copy the current counter value into a local variable
      g_displayUpdateNeeded = 0;                        // Clear the refresh flag because we are handling it now
      __enable_irq();  

      Display_WriteNumber4(localCounter);
    }
    __WFI();
  }
}
#endif


#ifdef Part_B
//USER button (PC13) toggles autoRun
//While autoRun = 1, counter increments every 1 second
//LD4 toggles every second while autoRun is active

volatile uint8_t g_autoRun =0;
volatile uint32_t g_lastAutoMs = 0; 

void SysTick_Handler(void)
{
  g_msTicks++;

  if(g_autoRun) //only do 1sec automatic action
  {
    if((g_msTicks - g_lastAutoMs) >= Auto_Period_Ms) //has 1ms elasped
    {
      g_lastAutoMs += Auto_Period_Ms;
      Counter_Increment();
      GPIO_Toggle(LD4_PORT, LD4_PIN);
      g_displayUpdateNeeded=1;
    }
  }
}

void EXTI4_15_IRQHandler(void)
{
  // Handle SW1 (PD8) decrement
    if (EXTI->FPR1 & (1UL << 8))
    {
        EXTI->FPR1 = (1UL << 8);                              // Clear EXTI8 falling pending flag

        if ((g_msTicks - g_lastSW1Ms) >= Debounce_Ms)         // Simple bounce filter
        {
            g_lastSW1Ms = g_msTicks;                          // Save time of accepted press
            Counter_Decrement();                              // Decrement counter
            g_displayUpdateNeeded = 1;                        // Request display refresh
        }
    }

    // Handle SW4 (PD9) increment
    if (EXTI->FPR1 & (1UL << 9))
    {
        EXTI->FPR1 = (1UL << 9);                              // Clear EXTI9 falling pending flag

        if ((g_msTicks - g_lastSW4Ms) >= Debounce_Ms)         // Simple bounce filter
        {
            g_lastSW4Ms = g_msTicks;                          // Save time of accepted press
            Counter_Increment();                              // Increment counter
            g_displayUpdateNeeded = 1;                        // Request display refresh
        }
    }

    if(EXTI->FPR1 & (1UL << 13))
    {
      EXTI->FPR1 = (1UL << 13);
      if ((g_msTicks - g_lastUserMs) >= Debounce_Ms)
      {
        g_lastUserMs = g_msTicks;
        g_autoRun ^= 1U; //toggle autorun

        if(!g_autoRun)
        {
          GPIO_Clear(LD4_PORT, LD4_PIN);
        }
        else
        {
          g_lastAutoMs = g_msTicks;
        }
      }
    }
}

int main(void)
{
  uint16_t localCounter;   

  App_InitBase();

  EXTI_MapLineToPort(8U, 3U);                               // Map EXTI8 to PD8
  EXTI_MapLineToPort(9U, 3U);                               // Map EXTI9 to PD9
  EXTI_MapLineToPort(13U, 2U);                              // Map EXTI13 to PC13
  
  EXTI_EnableFallingEdge(8U);                               // SW1 triggers on falling edge
  EXTI_EnableFallingEdge(9U);                               // SW4 triggers on falling edge
  EXTI_EnableFallingEdge(13U);                              // USER button toggles autoRun on falling edge

  NVIC_EnableIRQ(EXTI4_15_IRQn); 
  
  while(1)
  {
    if(g_displayUpdateNeeded)
    {
      __disable_irq();                                  // Briefly protect shared data
      localCounter = g_counter;                         // Copy counter to a local variable
      g_displayUpdateNeeded = 0;                        // Clear the request flag
      __enable_irq(); 

      Display_WriteNumber4(localCounter);
    }
    __WFI();
  }     

}
#endif

#ifdef Part_C
//USER button short press toggles autoRun
//USER button held for >= 3 seconds resets counter to 0000
//Long press also stops autoRun
volatile uint8_t g_autoRun =0;
volatile uint32_t g_lastAutoMs = 0;
volatile uint8_t g_buttonDown = 0;
volatile uint32_t g_pressStartMs =0;
volatile uint8_t g_longPressHandled = 0;

void SysTick_Handler(void)
{
  g_msTicks++;
  if (g_autoRun)                                            // Do timed auto increment only when auto mode is enabled
  {
    if ((g_msTicks - g_lastAutoMs) >= Auto_Period_Ms)     // Has one second elapsed?
    {
      g_lastAutoMs += Auto_Period_Ms;                   // Move the next one-second boundary forward
      Counter_Increment();                              // Increment the counter once
      GPIO_Toggle(LD4_PORT, LD4_PIN);                   // Toggle LD4 each second while auto mode is active
      g_displayUpdateNeeded = 1;                        // Ask main to refresh the display
    }
  }

  if(g_buttonDown && !g_longPressHandled)
  {
    if ((g_msTicks - g_pressStartMs) >= Long_Press_Ms)
    {
      g_counter = 0;
      g_autoRun = 0;
      g_longPressHandled = 1;
      GPIO_Clear(LD4_PORT, LD4_PIN);
      g_displayUpdateNeeded =1;
    }
  }
}

void EXTI4_15_IRQHandler(void)
{
// Handle SW1 (PD8) decrement
  if (EXTI->FPR1 & (1UL << 8))
  {
      EXTI->FPR1 = (1UL << 8);                              // Clear EXTI8 falling pending flag

      if ((g_msTicks - g_lastSW1Ms) >= Debounce_Ms)         // Simple bounce filter
      {
          g_lastSW1Ms = g_msTicks;                          // Save time of accepted press
          Counter_Decrement();                              // Decrement counter
          g_displayUpdateNeeded = 1;                        // Request display refresh
      }
  }

  // Handle SW4 (PD9) increment
  if (EXTI->FPR1 & (1UL << 9))
  {
      EXTI->FPR1 = (1UL << 9);                              // Clear EXTI9 falling pending flag

      if ((g_msTicks - g_lastSW4Ms) >= Debounce_Ms)         // Simple bounce filter
      {
          g_lastSW4Ms = g_msTicks;                          // Save time of accepted press
          Counter_Increment();                              // Increment counter
          g_displayUpdateNeeded = 1;                        // Request display refresh
      }
  }

  //handle user button press
  if(EXTI->FPR1 & (1UL << 13))
  {
    EXTI->FPR1 = (1UL << 13);
    if ((g_msTicks - g_lastUserMs) >= Debounce_Ms)
    {
      g_lastUserMs = g_msTicks;// Save accepted edge time
      g_buttonDown = 1;// Remember that the button is currently being held
      g_pressStartMs = g_msTicks;// Record the exact time the press started
      g_longPressHandled = 0;
    }
  }
// Handle USER button RELEASE on rising edge
  if(EXTI->RPR1 & (1UL << 13))
  {
    EXTI->RPR1 = (1UL << 13);
    if ((g_msTicks - g_lastUserMs) >= Debounce_Ms)
    {
      g_lastUserMs = g_msTicks;

      if(!g_longPressHandled)// Only do short-press action if long press did NOT already happen
      {
        g_autoRun ^= 1U;
        if (!g_autoRun)                               // If autoRun was just turned off
        {
            GPIO_Clear(LD4_PORT, LD4_PIN);            // Force LD4 off
        }
        else
        {
            g_lastAutoMs = g_msTicks;                 // Restart the 1-second timing window from now
        }
      }
      g_buttonDown = 0;// Button is no longer being held after release

    }        

  }

}

int main(void)
{
  uint16_t localCounter;                                    // Local stable copy of the counter for display update

  App_InitBase();                                           // Initialize common hardware

  EXTI_MapLineToPort(8U, 3U);                               // Map EXTI8 to PD8
  EXTI_MapLineToPort(9U, 3U);                               // Map EXTI9 to PD9
  EXTI_MapLineToPort(13U, 2U);                              // Map EXTI13 to PC13

  EXTI_EnableFallingEdge(8U);                               // SW1 triggers on falling edge
  EXTI_EnableFallingEdge(9U);                               // SW4 triggers on falling edge
  EXTI_EnableBothEdges(13U);                                // USER button needs both press and release edges

  NVIC_EnableIRQ(EXTI4_15_IRQn);                            // Enable grouped EXTI interrupt in NVIC
  
  while(1)
  {
  if (g_displayUpdateNeeded)                            // Refresh the display only when the value changed
  {
      __disable_irq();                                  // Protect the shared snapshot
      localCounter = g_counter;                         // Copy the current counter to a local variable
      g_displayUpdateNeeded = 0;                        // Clear the request flag
      __enable_irq();                                   // Re-enable interrupts immediately

      Display_WriteNumber4(localCounter);               // Update the 7-seg display from main
  }

  __WFI();  
  }
}
#endif