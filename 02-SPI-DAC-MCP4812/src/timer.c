#include "timer.h"


//Internal : Enable RCC clock for whichever TIMx was passed in.
void Timer_EnableClock(TIM_TypeDef *timer)
{
    if (timer == TIM1)
    {
        #ifdef RCC_APBENR2_TIM1EN
        RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
        #endif
    }
    else if (timer == TIM2)
    {
        #ifdef RCC_APBENR1_TIM2EN
        RCC->APBENR1 |= RCC_APBENR1_TIM2EN;
        #endif
    }
    else if (timer == TIM3)
    {
        #ifdef RCC_APBENR1_TIM3EN
        RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
        #endif
    }
    else if (timer == TIM4)
    {
        #ifdef RCC_APBENR1_TIM4EN
        RCC->APBENR1 |= RCC_APBENR1_TIM4EN;
        #endif
    }
    else if (timer == TIM6)
    {
        #ifdef RCC_APBENR1_TIM6EN
        RCC->APBENR1 |= RCC_APBENR1_TIM6EN;
        #endif
    }
    else if (timer == TIM7)
    {
        #ifdef RCC_APBENR1_TIM7EN
        RCC->APBENR1 |= RCC_APBENR1_TIM7EN;
        #endif
    }
    else if (timer == TIM14)
    {
        #ifdef RCC_APBENR2_TIM14EN
        RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
        #endif
    }
    else if (timer == TIM15)
    {
        #ifdef RCC_APBENR2_TIM15EN
        RCC->APBENR2 |= RCC_APBENR2_TIM15EN;
        #endif
    }
    else if (timer == TIM16)
    {
        #ifdef RCC_APBENR2_TIM16EN
        RCC->APBENR2 |= RCC_APBENR2_TIM16EN;
        #endif
    }
    else if (timer == TIM17)
    {
        #ifdef RCC_APBENR2_TIM17EN
        RCC->APBENR2 |= RCC_APBENR2_TIM17EN;
        #endif
    }
}

//Enables timer clock and sets PSC + ARR.
//Always put psc -1. cause the registors divides by psc+1
void Timer_Init(TIM_TypeDef *timer, uint16_t psc, uint16_t period)
{
    //Enable RCC clock for this peripheral
    Timer_EnableClock(timer);

    //Stop timer before while configuring 
    timer->CR1 &= ~TIM_CR1_CEN;
    
    //Configure prescaler and autoreload
    timer->PSC = psc;
    timer->ARR = period;

    //Enable ARR preload 
    timer->CR1 |= TIM_CR1_ARPE;

    //Update event to load PSC/ARR into active shadow registors immediately
    timer->EGR = TIM_EGR_UG;

    //Clear update flag 
    timer->SR &= ~ TIM_SR_UIF;

}

void Timer_SetupChannel(TIM_TypeDef *timer, CCR_Typedef ccr, ChannelMode_Typedef chMode)
{
    uint32_t channelIndex = 0;
    volatile uint32_t *pCCMR = 0;
    uint8_t shift = 0;

    // Map CCR offset -> channel number + CCMR register + byte shift
    switch (ccr)
    {
        case TimCCR1:
            channelIndex = 1; pCCMR = &timer->CCMR1; shift = 0;  break;
        case TimCCR2:
            channelIndex = 2; pCCMR = &timer->CCMR1; shift = 8;  break;
        case TimCCR3:
            channelIndex = 3; pCCMR = &timer->CCMR2; shift = 0;  break;
        case TimCCR4:
            channelIndex = 4; pCCMR = &timer->CCMR2; shift = 8;  break;

        case TimCCR5:
                    // CCR5/CCR6 are only valid on TIM1 in this course context
            if (timer != TIM1) return;
            #ifdef TIM1_CCMR3
            channelIndex = 5; pCCMR = &timer->CCMR3; shift = 0;  break;
            #else
            return;
            #endif

        case TimCCR6:
            if (timer != TIM1) return;
            #ifdef TIM1_CCMR3
            channelIndex = 6; pCCMR = &timer->CCMR3; shift = 8;  break;
            #else   
            return;
            #endif

        default:
            return;
    }   

    // Clear the 8-bit field for this channel inside CCMR
    *pCCMR &= ~(0xFFUL << shift);
    
    // Build mode bits inside that 8-bit slice:
    // Bits (within slice):
    //   CCxS  : bits 1:0  (00=output, 01=input mapped)
    //   OCxPE : bit  3
    //   OCxM  : bits 6:4  (toggle=011, pwm1=110, pwm2=111)
    uint32_t modeConfig = 0;

    switch (chMode)
    {
        case OutputCompareToggle:
            // Output mode, Toggle on match
            modeConfig = (3U << 4); // OCxM=011
            break;

        case Pwm1:
            // Output mode, PWM1 + preload enable
            modeConfig = (6U << 4) | (1U << 3);
            break;

        case Pwm2:
            // Output mode, PWM2 + preload enable
            modeConfig = (7U << 4) | (1U << 3);
            break;

        case InputCapture:
            // Input capture mapped to TIx
            modeConfig = (1U << 0); // CCxS=01
            break;

        default:
            return;
    }

    *pCCMR |= (modeConfig << shift);

    // Disable this channel nibble first (CCxE/Polarity/etc), then enable CCxE
    timer->CCER &= ~(0xFUL << ((channelIndex - 1U) * 4U));
    timer->CCER |=  (1UL  << ((channelIndex - 1U) * 4U)); // CCxE

     // If we are using TIM1 outputs (toggle/pwm), main output enable must be set
    if (timer == TIM1 && chMode != InputCapture)
    {
    #ifdef TIM_BDTR_MOE
            timer->BDTR |= TIM_BDTR_MOE;
    #endif
    }

    // If preload is enabled (PWM), force an update so CCR/ARR/PSC preloads latch cleanly
    timer->EGR = TIM_EGR_UG;
    timer->SR &= ~TIM_SR_UIF;

}

void Timer_WriteCCR(TIM_TypeDef *timer, CCR_Typedef ccr, uint32_t ccrTicks)
{
    volatile uint32_t *targetCCR = (volatile uint32_t *)((uint32_t)&timer->CCR1 + (uint32_t)ccr);
    *targetCCR = ccrTicks;

}

void Timer_EnableInterrupt(TIM_TypeDef *timer, IRQn_Type timerIRQn, Timer_IE interruptMask)
{   
    timer->DIER |= interruptMask;
    NVIC_EnableIRQ(timerIRQn);
}

void Timer_SetEnable(TIM_TypeDef *timer, uint16_t en)
{
    if(en) 
    {
        timer->CR1 |=  TIM_CR1_CEN;
    }
    else
    {
        timer->CR1 &= ~TIM_CR1_CEN; 
    }
}

void Timer_SetDelay_us(TIM_TypeDef *timer)
{
     // Divide system clock down to 1 MHz:
    // timer_tick = SystemCoreClock / (PSC+1)
    // => PSC = (SystemCoreClock/1e6) - 1
    uint32_t pscValue = (SystemCoreClock / 1000000UL);
    if (pscValue == 0) pscValue = 1; // safety
    pscValue -= 1;

    Timer_Init(timer, (uint16_t)pscValue, 0xFFFF);
    Timer_SetEnable(timer, 1);
}

void Timer_Delay_us(TIM_TypeDef *timer, uint16_t us)
{
    timer->CNT = 0;
    while (timer->CNT < us) { /* wait */ }
}