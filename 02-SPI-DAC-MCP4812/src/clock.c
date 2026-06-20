#include "clock.h"
#include "stm32g0b1xx.h"
//Some terms to know before
//PLL - it is the MCU's built-in clock "gearbox".
// It takes a known clock source (internal HSI16 or external crystal) and then multiplies or divides 
// to create faster clock for the chip
// Everything depends on the system clock(SYSCLK).
// PLL can make it run faster i.e. 16MHz to 64MHz.
// PLL is Phase Locked Loop as it constantly compares its output to the input clock and 
// keeps the output locked to it (same rhythm), just at a different frequency.


//What this does is 
//1 choose and input clock (HSI16).
//2 set PLL multipliers/dividers.
//3 enable PLL and wait untill it's "ready/locked".
//4 switch SYSCLK to PLL output.
void Clock_InitPll(PllRange pllRange)
{

    //Enable power interface Clock as we need to write PWR registers.

    RCC->APBENR1 |= RCC_APBENR1_PWREN; //This turns on Clock to PWR peripheral

    //Set voltage scaling to Range 1 (required for higher SYSCLK)
    PWR->CR1 &= ~PWR_CR1_VOS_Msk;                      // Clear VOS bits (voltage scaling selection)
    PWR->CR1 |=  (1U << PWR_CR1_VOS_Pos);              // Set VOS = 01 => Range 1 (higher performance range)
    while (PWR->SR2 & PWR_SR2_VOSF) {                  // Wait until VOSF flag clears (voltage scaling ready)
        /* wait */
    }

    //Now we PLL
    //Enable HSI (16 MHz) as we want to use it as PLL source
    RCC->CR |= RCC_CR_HSION;                 // Enable internal 16 MHz oscillator
    while(!(RCC->CR & RCC_CR_HSIRDY)) { }    // Wait until HSI16 ready flag is set


    // Configure FLASH wait states BEFORE increasing SYSCLK
    FLASH->ACR &= ~FLASH_ACR_LATENCY;       // Clear latency bits
    FLASH->ACR |= FLASH_ACR_LATENCY_2;      // Set 2 wait states
    FLASH->ACR |= FLASH_ACR_PRFTEN;         // Enable prefetch (helps performance at higher speeds)

    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2)
    {
        // Wait until the latency bits actually reflect the value we requested
    }
    // If currently running from PLL, switch SYSCLK to HSI16 first
    if (((RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos) == 2u) 
    {
        RCC->CFGR &= ~RCC_CFGR_SW_Msk;
        RCC->CFGR |=  (1u << RCC_CFGR_SW_Pos);     // SW=001 => HSI16
        while (((RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos) != 1u) { }
    } 
    //Disable PLL before changing config
    RCC->CR &= ~RCC_CR_PLLON;           // Turn PLL off
    while(RCC->CR & RCC_CR_PLLRDY) 
    {
        // Wait until PLLRDY goes low (PLL fully stopped)
    }  

    //Write PLL configuration
    RCC->PLLCFGR = (uint32_t)pllRange;       // Write the full PLLCFGR config (from the enum)

    // 4) Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY))
    {
        // Wait until PLLRDY is set
    }

    // 5) Switch SYSCLK to PLL
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;        // Clear SW[2:0] (system clock switch bits)
    RCC->CFGR |= (2U << RCC_CFGR_SW_Pos); // SW = 010 => select PLL as SYSCLK

    // Wait until PLL is used as system clock
    while (((RCC->CFGR & RCC_CFGR_SWS_Msk) >> RCC_CFGR_SWS_Pos) != 2U)
    {
        /*Wait until hardware confirms PLL is the system clock source.
       SWS[2:0] encodes the current system clock.
       For PLL, SWS should become 010 as well.*/
    }

    // 6) Update SystemCoreClock
    SystemCoreClockUpdate();
}


//Its Puts the Clock on PA8 so that you can see it is running at the chosen rate
//It uses MCU's MCO (microcontroller Clock output) which picks the internal clock source and divide it, and route it to PA8
//1 Enable GPIOA clock
//2 PA8 -> alternate function mode
//3 PA8 high speed
//4 AF0 on PA8
//5 program RCC_CFGR.MCOSEL + MCOPRE

void Clock_EnableOutput(MCO_Select mcoSelect, MCO_Div mcoDiv) 
{
    // 1. Enable Port A
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    
    // 2. Set PA8 to Alternate Function (Mode 10)
    GPIOA->MODER &= ~GPIO_MODER_MODE8; 
    GPIOA->MODER |= GPIO_MODER_MODE8_1; 

    // 3. CRITICAL: Set PA8 to Very High Speed
    // This allows the pin to toggle fast enough for >10MHz signals
    GPIOA->OSPEEDR &= ~(3u << (8u * 2u));
    GPIOA->OSPEEDR |=  (3u << (8u * 2u)); 

    // 4. Set AF0 for PA8
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL8; 

    // 5. Configure MCO (Select + prescale in RCC_CFGR)
    RCC->CFGR &= ~(RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE);
    RCC->CFGR |= (mcoSelect | mcoDiv);
}

void Clock_LSE_Init(void)
{
    // Enable PWR peripheral clock
       //Required because backup domain (BDCR) is protected by PWR 
    RCC->APBENR1 |= RCC_APBENR1_PWREN;

    // Enable write access to backup domain
       //Without this, writes to RCC->BDCR are ignored 
    PWR->CR1 |= PWR_CR1_DBP;
    while ((PWR->CR1 & PWR_CR1_DBP) == 0) { }

    // Enable the LSE oscillator 
    RCC->BDCR |= RCC_BDCR_LSEON;

    // Wait until LSE is ready
       
    while (!(RCC->BDCR & RCC_BDCR_LSERDY))
    {
        /* wait */
    }
    ////Optionally select the LSE as the RTC clock source
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= (1u << RCC_BDCR_RTCSEL_Pos);
    //Enable the RTC domain write protection in the PWR_CR1 register (RM0444::4.4.1)
    //PWR->CR1 &= ~(PWR_CR1_DBP_Msk);
    //enable rtc
    RCC->BDCR |= RCC_BDCR_RTCEN;
}