#include "dac.h"

/*********************************************************************
*
*       DAC1_CH1_Init
*
*  Function description
*   Enables GPIOA and DAC1 clocks, configures PA4 as analog mode,
*   disables DAC channel 1 before configuration, enables output buffer
*   by default, disables trigger and DMA by default, and clears output.
*/
void DAC1_CH1_Init(void)
{
    // Enable clocks
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_DAC1EN;

    // PA4 = analog mode
    GPIOA->MODER &= ~GPIO_MODER_MODE4_Msk;
    GPIOA->MODER |=  GPIO_MODER_MODE4_Msk;

    // Disable DAC CH1 before configuration
    DAC1->CR &= ~DAC_CR_EN1;

    // Default: output buffer enabled (MODE1 = 000)
    DAC1->MCR &= ~DAC_MCR_MODE1_Msk;

    // Default: trigger disabled
    DAC1->CR &= ~DAC_CR_TEN1;
    DAC1->CR &= ~DAC_CR_TSEL1_Msk;

    // Default: DMA disabled
    DAC1->CR &= ~DAC_CR_DMAEN1;

    // Default output = 0
    DAC1->DHR12R1 = 0;
}

/*********************************************************************
*
*       DAC1_CH1_Enable
*
*  Function description
*   Enables DAC channel 1.
*/
void DAC1_CH1_Enable(void)
{
    DAC1->CR |= DAC_CR_EN1;
}

/*********************************************************************
*
*       DAC1_CH1_Disable
*
*  Function description
*   Disables DAC channel 1.
*/
void DAC1_CH1_Disable(void)
{
    DAC1->CR &= ~DAC_CR_EN1;
}

/*********************************************************************
*
*       DAC1_CH1_EnableBuffer
*
*  Function description
*   Enables the DAC output buffer by selecting MODE1 = 000.
*/
void DAC1_CH1_EnableBuffer(void)
{
    DAC1->MCR &= ~DAC_MCR_MODE1_Msk;
}

/*********************************************************************
*
*       DAC1_CH1_DisableBuffer
*
*  Function description
*   Disables the DAC output buffer.
*
*  Note
*   This selects MODE1 = 010 as a simple no-buffer external pin mode.
*   Adjust later if you want another DAC mode option.
*/
void DAC1_CH1_DisableBuffer(void)
{
    DAC1->MCR &= ~DAC_MCR_MODE1_Msk;
    DAC1->MCR |= DAC_MCR_MODE1_1;
}

/*********************************************************************
*
*       DAC1_CH1_SetTrigger
*
*  Function description
*   Selects the DAC hardware trigger source for channel 1.
*
*  Parameters
*   trigger - One of the DAC_Trigger_t enum values.
*/
void DAC1_CH1_SetTrigger(DAC_Trigger_t trigger)
{
    DAC1->CR &= ~DAC_CR_TSEL1_Msk;
    DAC1->CR |= ((uint32_t)trigger << DAC_CR_TSEL1_Pos);
}

/*********************************************************************
*
*       DAC1_CH1_EnableTrigger
*
*  Function description
*   Enables hardware trigger on DAC channel 1.
*/
void DAC1_CH1_EnableTrigger(void)
{
    DAC1->CR |= DAC_CR_TEN1;
}

/*********************************************************************
*
*       DAC1_CH1_DisableTrigger
*
*  Function description
*   Disables hardware trigger on DAC channel 1.
*/
void DAC1_CH1_DisableTrigger(void)
{
    DAC1->CR &= ~DAC_CR_TEN1;
}

/*********************************************************************
*
*       DAC1_CH1_EnableDMA
*
*  Function description
*   Enables DMA requests from DAC channel 1.
*/
void DAC1_CH1_EnableDMA(void)
{
    DAC1->CR |= DAC_CR_DMAEN1;
}

/*********************************************************************
*
*       DAC1_CH1_DisableDMA
*
*  Function description
*   Disables DMA requests from DAC channel 1.
*/
void DAC1_CH1_DisableDMA(void)
{
    DAC1->CR &= ~DAC_CR_DMAEN1;
}

/*********************************************************************
*
*       DAC1_CH1_Write
*
*  Function description
*   Writes a 12-bit value to DAC channel 1 holding register.
*
*  Parameters
*   value - DAC code from 0 to 4095
*/
void DAC1_CH1_Write(uint16_t value)
{
    if (value > 4095)
    {
        value = 4095;
    }

    DAC1->DHR12R1 = value;
}

/*********************************************************************
*
*       DAC1_CH2_Init
*
*  Function description
*   Enables GPIOA and DAC1 clocks, configures PA5 as analog mode,
*   disables DAC channel 2 before configuration, enables output buffer
*   by default, disables trigger and DMA by default, and clears output.
*/
void DAC1_CH2_Init(void)
{
    // Enable clocks
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_DAC1EN;

    // PA5 = analog mode
    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
    GPIOA->MODER |=  GPIO_MODER_MODE5_Msk;

    // Disable DAC CH2 before configuration
    DAC1->CR &= ~DAC_CR_EN2;

    // Default: output buffer enabled (MODE2 = 000)
    DAC1->MCR &= ~DAC_MCR_MODE2_Msk;

    // Default: trigger disabled
    DAC1->CR &= ~DAC_CR_TEN2;
    DAC1->CR &= ~DAC_CR_TSEL2_Msk;

    // Default: DMA disabled
    DAC1->CR &= ~DAC_CR_DMAEN2;

    // Default output = 0
    DAC1->DHR12R2 = 0;
}

/*********************************************************************
*
*       DAC1_CH2_Enable
*
*  Function description
*   Enables DAC channel 2.
*/
void DAC1_CH2_Enable(void)
{
    DAC1->CR |= DAC_CR_EN2;
}

/*********************************************************************
*
*       DAC1_CH2_Disable
*
*  Function description
*   Disables DAC channel 2.
*/
void DAC1_CH2_Disable(void)
{
    DAC1->CR &= ~DAC_CR_EN2;
}

/*********************************************************************
*
*       DAC1_CH2_EnableBuffer
*
*  Function description
*   Enables the DAC output buffer by selecting MODE2 = 000.
*/
void DAC1_CH2_EnableBuffer(void)
{
    DAC1->MCR &= ~DAC_MCR_MODE2_Msk;
}

/*********************************************************************
*
*       DAC1_CH2_DisableBuffer
*
*  Function description
*   Disables the DAC output buffer.
*
*  Note
*   This selects MODE2 = 010 as a simple no-buffer external pin mode.
*/
void DAC1_CH2_DisableBuffer(void)
{
    DAC1->MCR &= ~DAC_MCR_MODE2_Msk;
    DAC1->MCR |= DAC_MCR_MODE2_1;
}

/*********************************************************************
*
*       DAC1_CH2_SetTrigger
*
*  Function description
*   Selects the DAC hardware trigger source for channel 2.
*
*  Parameters
*   trigger - One of the DAC_Trigger_t enum values.
*/
void DAC1_CH2_SetTrigger(DAC_Trigger_t trigger)
{
    DAC1->CR &= ~DAC_CR_TSEL2_Msk;
    DAC1->CR |= ((uint32_t)trigger << DAC_CR_TSEL2_Pos);
}

/*********************************************************************
*
*       DAC1_CH2_EnableTrigger
*
*  Function description
*   Enables hardware trigger on DAC channel 2.
*/
void DAC1_CH2_EnableTrigger(void)
{
    DAC1->CR |= DAC_CR_TEN2;
}

/*********************************************************************
*
*       DAC1_CH2_DisableTrigger
*
*  Function description
*   Disables hardware trigger on DAC channel 2.
*/
void DAC1_CH2_DisableTrigger(void)
{
    DAC1->CR &= ~DAC_CR_TEN2;
}

/*********************************************************************
*
*       DAC1_CH2_EnableDMA
*
*  Function description
*   Enables DMA requests from DAC channel 2.
*/
void DAC1_CH2_EnableDMA(void)
{
    DAC1->CR |= DAC_CR_DMAEN2;
}

/*********************************************************************
*
*       DAC1_CH2_DisableDMA
*
*  Function description
*   Disables DMA requests from DAC channel 2.
*/
void DAC1_CH2_DisableDMA(void)
{
    DAC1->CR &= ~DAC_CR_DMAEN2;
}

/*********************************************************************
*
*       DAC1_CH2_Write
*
*  Function description
*   Writes a 12-bit value to DAC channel 2 holding register.
*
*  Parameters
*   value - DAC code from 0 to 4095
*/
void DAC1_CH2_Write(uint16_t value)
{
    if (value > 4095)
    {
        value = 4095;
    }

    DAC1->DHR12R2 = value;
}