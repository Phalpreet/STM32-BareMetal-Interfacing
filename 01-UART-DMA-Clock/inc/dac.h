#include <stdint.h>
#include "stm32g0b1xx.h"

#ifndef DAC_H
#define DAC_H



typedef enum
{
    DAC_TRIGGER_NONE       = 0,
    DAC_TRIGGER_TIM1_TRGO  = 1,
    DAC_TRIGGER_TIM2_TRGO  = 2,
    DAC_TRIGGER_TIM3_TRGO  = 3,
    DAC_TRIGGER_TIM6_TRGO  = 5,
    DAC_TRIGGER_TIM7_TRGO  = 6,
    DAC_TRIGGER_TIM15_TRGO = 8,
    DAC_TRIGGER_LPTIM1_OUT = 11,
    DAC_TRIGGER_LPTIM2_OUT = 12,
    DAC_TRIGGER_EXTI9      = 13
} DAC_Trigger_t;

void DAC1_CH1_Init(void);
void DAC1_CH1_Enable(void);
void DAC1_CH1_Disable(void);

void DAC1_CH1_EnableBuffer(void);
void DAC1_CH1_DisableBuffer(void);

void DAC1_CH1_SetTrigger(DAC_Trigger_t trigger);
void DAC1_CH1_EnableTrigger(void);
void DAC1_CH1_DisableTrigger(void);

void DAC1_CH1_EnableDMA(void);
void DAC1_CH1_DisableDMA(void);

void DAC1_CH1_Write(uint16_t value);

void DAC1_CH2_Init(void);
void DAC1_CH2_Enable(void);
void DAC1_CH2_Disable(void);

void DAC1_CH2_EnableBuffer(void);
void DAC1_CH2_DisableBuffer(void);

void DAC1_CH2_SetTrigger(DAC_Trigger_t trigger);
void DAC1_CH2_EnableTrigger(void);
void DAC1_CH2_DisableTrigger(void);

void DAC1_CH2_EnableDMA(void);
void DAC1_CH2_DisableDMA(void);

void DAC1_CH2_Write(uint16_t value);
#endif