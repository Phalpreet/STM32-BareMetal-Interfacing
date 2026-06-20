#ifndef EXTI_H
#define EXTI_H

#include "stm32g0b1xx.h"
#include <stdint.h>

// Which edge(s) should trigger the EXTI line
typedef enum
{
    EXTI_TRIGGER_NONE    = 0,
    EXTI_TRIGGER_RISING  = 1,
    EXTI_TRIGGER_FALLING = 2,
    EXTI_TRIGGER_BOTH    = 3
} EXTI_Trigger_t;

// Optional pull configuration for the GPIO input
typedef enum
{
    EXTI_PULL_NONE = 0,
    EXTI_PULL_UP,
    EXTI_PULL_DOWN
} EXTI_Pull_t;

// Configure one GPIO pin as an EXTI input line.
// Example: EXTI_InitGPIOInterrupt(GPIOC, 13, EXTI_TRIGGER_BOTH, EXTI_PULL_UP);
uint8_t EXTI_InitGPIOInterrupt(GPIO_TypeDef *port,
                               uint8_t pin,
                               EXTI_Trigger_t trigger,
                               EXTI_Pull_t pull);

// Change trigger edge(s) later if needed
void EXTI_SetTrigger(uint8_t pin, EXTI_Trigger_t trigger);

// Mask / unmask the EXTI line at the peripheral level
void EXTI_EnableInterrupt(uint8_t pin);
void EXTI_DisableInterrupt(uint8_t pin);

// Get the NVIC IRQ number for a GPIO EXTI pin
IRQn_Type EXTI_GetIRQn(uint8_t pin);

// Enable / disable the corresponding NVIC IRQ
void EXTI_EnableIRQ(uint8_t pin, uint32_t priority);
void EXTI_DisableIRQ(uint8_t pin);

// Pending flag helpers
uint8_t EXTI_IsFallingPending(uint8_t pin);
uint8_t EXTI_IsRisingPending(uint8_t pin);
uint8_t EXTI_IsPending(uint8_t pin);

void EXTI_ClearFallingPending(uint8_t pin);
void EXTI_ClearRisingPending(uint8_t pin);
void EXTI_ClearPending(uint8_t pin);

#endif