#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "stm32g0b1xx.h"

/*********************************************************************
*
*       Types
*
**********************************************************************/

typedef enum
{
    ADC_TRIGGER_NONE       = 0,
    ADC_TRIGGER_TRG0       = 0,
    ADC_TRIGGER_TRG1       = 1,
    ADC_TRIGGER_TRG2       = 2,
    ADC_TRIGGER_TRG3       = 3,
    ADC_TRIGGER_TRG4       = 4,
    ADC_TRIGGER_TRG5       = 5,   // TIM6_TRGO in your course examples
    ADC_TRIGGER_TRG6       = 6,
    ADC_TRIGGER_TRG7       = 7
} ADC_Trigger_t;

typedef enum
{
    ADC_TRIGGER_DISABLE    = 0,
    ADC_TRIGGER_RISING     = 1,
    ADC_TRIGGER_FALLING    = 2,
    ADC_TRIGGER_BOTH       = 3
} ADC_TriggerEdge_t;

/*********************************************************************
*
*       Function Prototypes
*
**********************************************************************/

void ADC1_Init(void);
void ADC1_Calibrate(void);
void ADC1_Enable(void);
void ADC1_Disable(void);

void ADC1_SetPrescalerDiv10(void);
void ADC1_EnableVref(void);

void ADC1_SelectChannels(uint32_t channels);

void ADC1_SetExternalTrigger(ADC_Trigger_t trigger, ADC_TriggerEdge_t edge);
void ADC1_DisableExternalTrigger(void);

void ADC1_EnableEOCInterrupt(void);
void ADC1_DisableEOCInterrupt(void);
void ADC1_EnableEOSInterrupt(void);
void ADC1_DisableEOSInterrupt(void);

void ADC1_StartConversion(void);

uint8_t ADC1_IsReady(void);
uint8_t ADC1_IsEOC(void);
uint8_t ADC1_IsEOS(void);

void ADC1_ClearEOS(void);

uint16_t ADC1_ReadData(void);

#endif