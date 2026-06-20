#include "exti.h"

// Convert GPIO port pointer into STM32G0 EXTI port code
// A=0, B=1, C=2, D=3, E=4, F=5
static int32_t EXTI_GetPortCode(GPIO_TypeDef *port)
{
    if (port == GPIOA) return 0;
    if (port == GPIOB) return 1;
    if (port == GPIOC) return 2;
    if (port == GPIOD) return 3;
    if (port == GPIOE) return 4;
    if (port == GPIOF) return 5;

    return -1;
}

// Enable RCC clock for the requested GPIO port
static void EXTI_EnableGPIOClock(GPIO_TypeDef *port)
{
    if (port == GPIOA) RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    else if (port == GPIOB) RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    else if (port == GPIOC) RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    else if (port == GPIOD) RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    else if (port == GPIOE) RCC->IOPENR |= RCC_IOPENR_GPIOEEN;
    else if (port == GPIOF) RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
}

// Small helper to get bit mask from pin number
static uint32_t EXTI_LineMask(uint8_t pin)
{
    return (1UL << pin);
}

uint8_t EXTI_InitGPIOInterrupt(GPIO_TypeDef *port,
                               uint8_t pin,
                               EXTI_Trigger_t trigger,
                               EXTI_Pull_t pull)
{
    int32_t portCode;
    uint32_t regIndex;
    uint32_t fieldShift;

    // Only GPIO EXTI lines 0..15 are handled by this library
    if (pin > 15U)
    {
        return 0;
    }

    portCode = EXTI_GetPortCode(port);
    if (portCode < 0)
    {
        return 0;
    }

    // Enable clocks needed for GPIO and EXTI routing
    EXTI_EnableGPIOClock(port);
    RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN;

    // Configure pin as input mode
    port->MODER &= ~(3UL << (pin * 2U));

    // Configure pull-up / pull-down / no-pull
    port->PUPDR &= ~(3UL << (pin * 2U));

    if (pull == EXTI_PULL_UP)
    {
        port->PUPDR |= (1UL << (pin * 2U));
    }
    else if (pull == EXTI_PULL_DOWN)
    {
        port->PUPDR |= (2UL << (pin * 2U));
    }

    // Map GPIO port to EXTI line
    // STM32G0 uses 8-bit EXTICR fields per EXTI line
    regIndex   = pin / 4U;
    fieldShift = (pin % 4U) * 8U;

    EXTI->EXTICR[regIndex] &= ~(0xFFUL << fieldShift);
    EXTI->EXTICR[regIndex] |=  ((uint32_t)portCode << fieldShift);

    // Configure trigger
    EXTI_SetTrigger(pin, trigger);

    // Unmask interrupt line
    EXTI_EnableInterrupt(pin);

    return 1;
}

void EXTI_SetTrigger(uint8_t pin, EXTI_Trigger_t trigger)
{
    uint32_t mask;

    if (pin > 15U)
    {
        return;
    }

    mask = EXTI_LineMask(pin);

    // Clear old trigger selection first
    EXTI->RTSR1 &= ~mask;
    EXTI->FTSR1 &= ~mask;

    // Apply new trigger mode
    if (trigger == EXTI_TRIGGER_RISING)
    {
        EXTI->RTSR1 |= mask;
    }
    else if (trigger == EXTI_TRIGGER_FALLING)
    {
        EXTI->FTSR1 |= mask;
    }
    else if (trigger == EXTI_TRIGGER_BOTH)
    {
        EXTI->RTSR1 |= mask;
        EXTI->FTSR1 |= mask;
    }
}

void EXTI_EnableInterrupt(uint8_t pin)
{
    if (pin > 15U)
    {
        return;
    }

    EXTI->IMR1 |= EXTI_LineMask(pin);
}

void EXTI_DisableInterrupt(uint8_t pin)
{
    if (pin > 15U)
    {
        return;
    }

    EXTI->IMR1 &= ~EXTI_LineMask(pin);
}

IRQn_Type EXTI_GetIRQn(uint8_t pin)
{
    if (pin <= 1U)  return EXTI0_1_IRQn;
    if (pin <= 3U)  return EXTI2_3_IRQn;
    return EXTI4_15_IRQn;
}

void EXTI_EnableIRQ(uint8_t pin, uint32_t priority)
{
    IRQn_Type irqn;

    if (pin > 15U)
    {
        return;
    }

    irqn = EXTI_GetIRQn(pin);
    NVIC_SetPriority(irqn, priority);
    NVIC_EnableIRQ(irqn);
}

void EXTI_DisableIRQ(uint8_t pin)
{
    IRQn_Type irqn;

    if (pin > 15U)
    {
        return;
    }

    irqn = EXTI_GetIRQn(pin);
    NVIC_DisableIRQ(irqn);
}

uint8_t EXTI_IsFallingPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return 0;
    }

    return (EXTI->FPR1 & EXTI_LineMask(pin)) ? 1U : 0U;
}

uint8_t EXTI_IsRisingPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return 0;
    }

    return (EXTI->RPR1 & EXTI_LineMask(pin)) ? 1U : 0U;
}

uint8_t EXTI_IsPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return 0;
    }

    return ((EXTI->FPR1 | EXTI->RPR1) & EXTI_LineMask(pin)) ? 1U : 0U;
}

void EXTI_ClearFallingPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return;
    }

    EXTI->FPR1 = EXTI_LineMask(pin);
}

void EXTI_ClearRisingPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return;
    }

    EXTI->RPR1 = EXTI_LineMask(pin);
}

void EXTI_ClearPending(uint8_t pin)
{
    if (pin > 15U)
    {
        return;
    }

    EXTI->FPR1 = EXTI_LineMask(pin);
    EXTI->RPR1 = EXTI_LineMask(pin);
}