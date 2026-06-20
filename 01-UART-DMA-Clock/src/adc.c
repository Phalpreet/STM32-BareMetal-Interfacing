#include "adc.h"


/*********************************************************************
*
*       ADC1_Init
*
*  Function description
*   Enables ADC clock and places ADC in a known default state.
*/
void ADC1_Init(void)
{
    // Enable ADC clock
    RCC->APBENR2 |= RCC_APBENR2_ADCEN;

    // Disable ADC before configuration
    ADC1_Disable();

    // Clear channel selection
    ADC1->CHSELR = 0;

    // Disable external trigger by default
    ADC1_DisableExternalTrigger();

    // Disable interrupts by default
    ADC1->IER &= ~(ADC_IER_EOCIE | ADC_IER_EOSIE);
}

/*********************************************************************
*
*       ADC1_Calibrate
*
*  Function description
*   Starts ADC self-calibration and waits for completion.
*/
void ADC1_Calibrate(void)
{
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
    {
    }
}

/*********************************************************************
*
*       ADC1_Enable
*
*  Function description
*   Enables ADC1 and waits until ready.
*/
void ADC1_Enable(void)
{
    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY))
    {
    }
}

/*********************************************************************
*
*       ADC1_Disable
*
*  Function description
*   Disables ADC1 and waits until disabled.
*/
void ADC1_Disable(void)
{
    ADC1->CR |= ADC_CR_ADDIS;
    while (ADC1->CR & ADC_CR_ADEN)
    {
    }
}

/*********************************************************************
*
*       ADC1_SetPrescalerDiv10
*
*  Function description
*   Sets ADC common clock prescaler to divide-by-10.
*/
void ADC1_SetPrescalerDiv10(void)
{
    ADC1_COMMON->CCR &= ~ADC_CCR_PRESC_Msk;
    ADC1_COMMON->CCR |= ADC_CCR_PRESC_2 | ADC_CCR_PRESC_0;
}

/*********************************************************************
*
*       ADC1_EnableVref
*
*  Function description
*   Enables internal voltage reference.
*/
void ADC1_EnableVref(void)
{
    ADC1_COMMON->CCR |= ADC_CCR_VREFEN;
}

/*********************************************************************
*
*       ADC1_SelectChannels
*
*  Function description
*   Selects ADC regular sequence channels through CHSELR bitmask.
*
*  Parameters
*   channels - Bitwise OR of ADC_CHSELR_CHSELx values.
*/
void ADC1_SelectChannels(uint32_t channels)
{
    ADC1->CHSELR = channels;
}

/*********************************************************************
*
*       ADC1_SetExternalTrigger
*
*  Function description
*   Selects external trigger source and edge.
*
*  Parameters
*   trigger - ADC trigger source code
*   edge    - rising/falling/both/disable
*/
void ADC1_SetExternalTrigger(ADC_Trigger_t trigger, ADC_TriggerEdge_t edge)
{
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTSEL;
    ADC1->CFGR1 |= ((uint32_t)trigger << ADC_CFGR1_EXTSEL_Pos);

    ADC1->CFGR1 &= ~ADC_CFGR1_EXTEN;
    ADC1->CFGR1 |= ((uint32_t)edge << ADC_CFGR1_EXTEN_Pos);
}

/*********************************************************************
*
*       ADC1_DisableExternalTrigger
*
*  Function description
*   Disables ADC external trigger.
*/
void ADC1_DisableExternalTrigger(void)
{
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTEN;
}

/*********************************************************************
*
*       ADC1_EnableEOCInterrupt
*
*  Function description
*   Enables End Of Conversion interrupt.
*/
void ADC1_EnableEOCInterrupt(void)
{
    ADC1->IER |= ADC_IER_EOCIE;
}

/*********************************************************************
*
*       ADC1_DisableEOCInterrupt
*
*  Function description
*   Disables End Of Conversion interrupt.
*/
void ADC1_DisableEOCInterrupt(void)
{
    ADC1->IER &= ~ADC_IER_EOCIE;
}

/*********************************************************************
*
*       ADC1_EnableEOSInterrupt
*
*  Function description
*   Enables End Of Sequence interrupt.
*/
void ADC1_EnableEOSInterrupt(void)
{
    ADC1->IER |= ADC_IER_EOSIE;
}

/*********************************************************************
*
*       ADC1_DisableEOSInterrupt
*
*  Function description
*   Disables End Of Sequence interrupt.
*/
void ADC1_DisableEOSInterrupt(void)
{
    ADC1->IER &= ~ADC_IER_EOSIE;
}

/*********************************************************************
*
*       ADC1_StartConversion
*
*  Function description
*   Starts ADC conversion or arms ADC for external trigger mode.
*/
void ADC1_StartConversion(void)
{
    ADC1->CR |= ADC_CR_ADSTART;
}

/*********************************************************************
*
*       ADC1_IsReady
*
*  Function description
*   Returns non-zero if ADC is ready.
*/
uint8_t ADC1_IsReady(void)
{
    return (ADC1->ISR & ADC_ISR_ADRDY) ? 1U : 0U;
}

/*********************************************************************
*
*       ADC1_IsEOC
*
*  Function description
*   Returns non-zero if End Of Conversion flag is set.
*/
uint8_t ADC1_IsEOC(void)
{
    return (ADC1->ISR & ADC_ISR_EOC) ? 1U : 0U;
}

/*********************************************************************
*
*       ADC1_IsEOS
*
*  Function description
*   Returns non-zero if End Of Sequence flag is set.
*/
uint8_t ADC1_IsEOS(void)
{
    return (ADC1->ISR & ADC_ISR_EOS) ? 1U : 0U;
}

/*********************************************************************
*
*       ADC1_ClearEOS
*
*  Function description
*   Clears ADC End Of Sequence flag.
*/
void ADC1_ClearEOS(void)
{
    ADC1->ISR = ADC_ISR_EOS;
}

/*********************************************************************
*
*       ADC1_ReadData
*
*  Function description
*   Reads ADC data register.
*
*  Return value
*   12-bit ADC result.
*/
uint16_t ADC1_ReadData(void)
{
    return (uint16_t)ADC1->DR;
}