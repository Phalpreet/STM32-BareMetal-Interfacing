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

const uint16_t test_table[2] = {0, 4095};
const uint16_t sine_lookup[256] = {
  2098,2148,2198,2248,2298,2348,2398,2447,2496,2545,2594,2642,2690,2737,2784,2831,
  2877,2923,2968,3013,3057,3100,3143,3185,3226,3267,3307,3346,3385,3423,3459,3495,
  3530,3565,3598,3630,3662,3692,3722,3750,3777,3804,3829,3853,3876,3898,3919,3939,
  3958,3975,3992,4007,4021,4034,4045,4056,4065,4073,4080,4085,4089,4093,4094,4095,
  4094,4093,4089,4085,4080,4073,4065,4056,4045,4034,4021,4007,3992,3975,3958,3939,
  3919,3898,3876,3853,3829,3804,3777,3750,3722,3692,3662,3630,3598,3565,3530,3495,
  3459,3423,3385,3346,3307,3267,3226,3185,3143,3100,3057,3013,2968,2923,2877,2831,
  2784,2737,2690,2642,2594,2545,2496,2447,2398,2348,2298,2248,2198,2148,2098,2048,
  1997,1947,1897,1847,1797,1747,1697,1648,1599,1550,1501,1453,1405,1358,1311,1264,
  1218,1172,1127,1082,1038,995,952,910,869,828,788,749,710,672,636,600,
  565,530,497,465,433,403,373,345,318,291,266,242,219,197,176,156,
  137,120,103,88,74,61,50,39,30,22,15,10,6,2,1,0,
  1,2,6,10,15,22,30,39,50,61,74,88,103,120,137,156,
  176,197,219,242,266,291,318,345,373,403,433,465,497,530,565,600,
  636,672,710,749,788,828,869,910,952,995,1038,1082,1127,1172,1218,1264,
  1311,1358,1405,1453,1501,1550,1599,1648,1697,1747,1797,1847,1897,1947,1997,2048
};

/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) 
{
    //Clocks
    RCC->APBENR1 |= RCC_APBENR1_PWREN;
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_DAC1EN;
    RCC->APBENR1 |= RCC_APBENR1_TIM6EN;
    RCC->AHBENR  |= RCC_AHBENR_DMA1EN;  // On STM32G0, this enables BOTH DMA and DMAMUX clocks

    Clock_InitPll(PLL_40MHZ);

    //PA4 = DAC output (analog mode)
    GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
    GPIOA->MODER |=  GPIO_MODER_MODE4_Msk;

    /*****DAC SETUP*****/
    DAC1->CR &= ~DAC_CR_EN1;           // Disable DAC CH1 before config
    DAC1->MCR &= ~DAC_MCR_MODE1_Msk;   // Output buffer enabled (MODE=000)

    // Select trigger source for DAC CH1
    DAC1->CR &= ~DAC_CR_TSEL1_Msk;     // Clear trigger select bits first
    DAC1->CR |=  DAC_CR_TSEL1_0 | DAC_CR_TSEL1_2;       // FIX: TSEL=0b0001 => TIM6_TRGO
                                        // (0b0000 = SWTRIG, which ignores TIM6 entirely)

    DAC1->CR |= DAC_CR_TEN1;           // Enable hardware trigger

    // Enable DAC DMA requests
    DAC1->CR |= DAC_CR_DMAEN1;         // FIX: SET the bit, not clear it
                                        // DAC must have DMA enabled to pull samples automatically

    // Note: No manual preload needed — DMA will write the first sample before
    // the first trigger fires once the engine is started below

    /*****DMAMUX SETUP*****/
    // DAC1_CH1 request id = 8
    // This routes DAC1_CH1's DMA request signal to DMA1_Channel1
    DMAMUX1_Channel0->CCR = 8;

    /*****DMA1 CHANNEL 1 SETUP*****/
    DMA1_Channel1->CCR &= ~DMA_CCR_EN; // Ensure DMA is off while configuring

    // Memory Source = sine_lookup array
    DMA1_Channel1->CMAR  = (uint32_t)sine_lookup;

    // Peripheral Destination = DAC holding register
    DMA1_Channel1->CPAR  = (uint32_t)&DAC1->DHR12R1;

    // Number of Samples
    DMA1_Channel1->CNDTR = 256;

    // Clear all mode bits before setting new ones
    DMA1_Channel1->CCR &= ~(DMA_CCR_MEM2MEM |
                             DMA_CCR_PL      |
                             DMA_CCR_MSIZE   |
                             DMA_CCR_PSIZE   |
                             DMA_CCR_MINC    |
                             DMA_CCR_PINC    |
                             DMA_CCR_CIRC    |
                             DMA_CCR_DIR);

    // New Modes:
    // Memory -> Peripheral    (DIR=1)
    // Memory increment ON     (MINC=1) — advances through sine_lookup each transfer
    // Peripheral increment OFF (PINC=0) — always writes to the same DAC register
    // 16-bit memory size      (MSIZE_0) — matches uint16_t sine_lookup entries
    // 16-bit peripheral size  (PSIZE_0) — DAC DHR12R1 is a 16-bit register
    // Circular mode ON        (CIRC=1)  — auto-resets to start of array after 256 transfers
    DMA1_Channel1->CCR |= DMA_CCR_DIR    |
                          DMA_CCR_MINC   |
                          DMA_CCR_MSIZE_0|
                          DMA_CCR_PSIZE_0|
                          DMA_CCR_CIRC;

    // Clear any stale interrupt flags before starting
    // Prevents DMA from acting on leftover flags from a previous run
    DMA1->IFCR = DMA_IFCR_CGIF1;

    // Start DMA engine
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    // Enable DAC — must come AFTER DMA is running so first trigger is not missed
    DAC1->CR |= DAC_CR_EN1;

    /*****TIMER SETUP*****/
    // PSC=39 => timer clock = 40MHz/(39+1) = 1MHz (1 tick = 1us)
    // ARR=9  => fires every 10 ticks = 10us = 100kHz trigger rate
    // Output frequency = 100kHz / 256 samples = ~390 Hz sine wave
    Timer_Init(TIM6, 39, 9);

    TIM6->CR2 &= ~TIM_CR2_MMS_Msk;
    TIM6->CR2 |=  TIM_CR2_MMS_1;      // MMS = 010 -> TRGO = update event
                                       // This is the signal the DAC listens to

    TIM6->DIER &= ~TIM_DIER_UDE;      // FIX: CLEAR timer DMA request
                                       // DAC has its own DMA request line (DMAEN1)
                                       // Setting UDE creates a second competing DMA
                                       // request on the same channel — unpredictable

    // Start the Timer — no interrupt needed, hardware triangle runs on its own:
    // TIM6 update -> TRGO -> DAC trigger -> DAC requests DMA -> DMA writes next sample
    Timer_SetEnable(TIM6, 1);

    while(1)
    {
        // CPU is free — entire sine wave runs in hardware
    }
}


/*************************** End of file ****************************/