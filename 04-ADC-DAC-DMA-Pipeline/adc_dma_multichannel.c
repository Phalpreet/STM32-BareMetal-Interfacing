/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/


#include <stdint.h>
#include <stdio.h>
#include "clock.h"
#include "timer.h"
#include "gpio.h"

//#define DMA_MODE
volatile uint8_t update = 0;
volatile uint16_t msCounter = 0;


#ifndef  DMA_MODE
volatile uint16_t an0Value = 0;
volatile uint16_t an1Value = 0;

volatile uint8_t adcSeqIndex = 0;     // 0 = next EOC is AN0, 1 = next EOC is AN1

volatile uint32_t an0Sum = 0;
volatile uint32_t an1Sum = 0;
volatile uint16_t seqCount = 0;

volatile uint16_t an0Avg = 0;
volatile uint16_t an1Avg = 0;

volatile uint8_t avgReady =0;
#endif

#ifdef DMA_MODE

#define ADC_DMA_COUNT 2000

volatile uint16_t adcBuffer[ADC_DMA_COUNT];
volatile uint8_t dmaBlockReady = 0;

volatile uint16_t an0Avg = 0;
volatile uint16_t an1Avg = 0;

#endif
/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/

void SysTick_Handler(void)
{

  if(++msCounter > 249)
  {
    msCounter = 0;
    update =1;
  }
  
}

int main(void) 
  {
    //Initialization

    //enable power inteface
    RCC-> APBENR1 |= RCC_APBENR1_PWREN;

    //Clock Peripherals 
    RCC-> IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC-> IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC-> IOPENR |= RCC_IOPENR_GPIOCEN;

    RCC->APBENR1 |= RCC_APBENR1_TIM6EN; //timer clock
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    #ifdef DMA_MODE

    //Enabling DMA1 Clock 
    RCC->AHBENR |= RCC_AHBENR_DMA1EN; 
    #endif
    GPIO_InitOutput(GPIOA, 5);
    GPIO_Clear(GPIOA, 5);
    GPIO_InitOutput(GPIOA, 6);
    //clock speed
    Clock_InitPll(PLL_40MHZ);
    
    printf("SYSCLK = %u Hz\n",SystemCoreClock); 
    
    GPIO_InitAlternateF(GPIOA, 2,1);
    GPIO_InitAlternateF(GPIOA, 3,1);
    UART_Init(USART2, 115200, 0);

    //Enable Systick
    SysTick_Config(SystemCoreClock / 1000);
    
    /*             ADC SETTINGS   */
    //Enable ADC Clock

    RCC->APBENR2 |= RCC_APBENR2_ADCEN;

    //Disabe ADC
    ADC1->CR |= ADC_CR_ADDIS;
    while(ADC1->CR & ADC_CR_ADEN);

    //Select Channel
    ADC1->CHSELR = ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL1; //ADC_IN0 = PA0 
                                                           //ADC_IN1 = PA1
    //Write PreScale Value- This Changes the default Clock Speed for ADC 
    ADC1_COMMON->CCR &= ~ADC_CCR_PRESC_Msk; //Clear Settings
    ADC1_COMMON->CCR |= ADC_CCR_PRESC_2 | ADC_CCR_PRESC_0; //Prescale by 10, making it - 4MHz

    //Internal Refrene Enable (For voltage Refrence) 
    ADC1_COMMON ->CCR |= ADC_CCR_VREFEN;

    //Enable Calibration
    ADC1->CR |= ADC_CR_ADCAL; //enable
    while(ADC1->CR & ADC_CR_ADCAL); //wait
    
    //Configuring up external Trigger Selection Settings
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTSEL; //Clear external trigger settings
    ADC1->CFGR1 |= ADC_CFGR1_EXTSEL_2 | ADC_CFGR1_EXTSEL_0; //101 = TRG5 = TIM6_TRGO
    
    //Configuring external Trigger enable settings
    ADC1->CFGR1 &= ~ADC_CFGR1_EXTEN; //clear the settings
    ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0;  //dection on Rising Edge
    
    #ifdef DMA_MODE 

    /*   DMA SETUP      */
    //ADC Request Mapped to DMAMUX Channel( ID = 5, DMA1 Channel 1)
    DMAMUX1_Channel0->CCR = 5; //why dmamux_Channel0 => because it feeds tp DMA1_Channel1
                              //IN stm32g0 dmamux channel x feeds dma channel x+1.

    //Make Sure DMA Channel is OFF before Configuring 
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    //Tell DMA where the peripheral data comes from
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR; 

    //Tell DMA wher to place the data in memory
    DMA1_Channel1->CMAR = (uint32_t)adcBuffer;

    //Tell DMA how many transfers to do 
    DMA1_Channel1->CNDTR = ADC_DMA_COUNT;
    
    //Clear old DMA modes 
    DMA1_Channel1->CCR &= ~(DMA_CCR_MEM2MEM |
                        DMA_CCR_PL |
                        DMA_CCR_MSIZE |
                        DMA_CCR_PSIZE |
                        DMA_CCR_MINC |
                        DMA_CCR_PINC |
                        DMA_CCR_CIRC |
                        DMA_CCR_DIR);
    
   
    //Set Modes which we want
    // Peripheral -> memory
    // Memory increment on
    // Peripheral increment off
    // 16-bit memory size
    // 16-bit peripheral size

    DMA1_Channel1->CCR |= DMA_CCR_MINC|
                          DMA_CCR_MSIZE_0|
                          DMA_CCR_PSIZE_0;
    //Memory increment enable (MINC) - After each transfer, DMA moves to the next element in RAM
    //Memory Size (MSIZE-0/1) - Each write to memory is a half-word
    //Peripheral Size (PSIZE-0/1) - Each read from the peripheral is also half-word
    //Why 16-bit (half Word), Was asked in Assignment
    
    //Enable Transfer complete interrupt for DMA Channel 1
    DMA1_Channel1->CCR |= DMA_CCR_TCIE;
    
    //Enable the interrupt in the NVIC
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);//enables cpu side interrupt line for this DMA Channel
    
    //Enable ADC-to-DMA request
    ADC1->CFGR1 |= ADC_CFGR1_DMAEN; //tells adc to generate dma requests when its data is ready
#endif

    //Enable ADC
    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY)); //Wait
  
    
    //Config TIM6
    Timer_Init(TIM6, 39, 999);
    //TRGO settings (TRG5 gets triggered)
    TIM6->CR2 &= ~TIM_CR2_MMS_Msk; //clear settings
    TIM6->CR2 |= TIM_CR2_MMS_1; //010 -> On TIM6 Update event trigger TRG5   
    //Enable TIM6
     
    //Enable Timer Interrupt
    Timer_EnableInterrupt(TIM6, TIM6_DAC_LPTIM1_IRQn , TimUIE);
    
    #ifndef  DMA_MODE
    //Enable END of Sequence of Conversion Interrupt And End of Conversion Interrupt
    ADC1->IER |= ADC_IER_EOSIE | ADC_IER_EOCIE; //End of Sequence of Conversion isr| End of Conversion isr 
   
    //Enable its IRQn
    NVIC_EnableIRQ(ADC1_COMP_IRQn);
    #endif
    
  

    #ifdef DMA_MODE
    ADC1->IER |= ADC_IER_EOSIE;
    NVIC_EnableIRQ(ADC1_COMP_IRQn);

    //Enable DMA Channel
    DMA1_Channel1->CCR |= DMA_CCR_EN;
    #endif

    ADC1->CR |=  ADC_CR_ADSTART; //Start converstion as soon you see external trigger
    Timer_SetEnable(TIM6,1);
   
    TERM_ClearScreen(USART2);
    printf("all setting done");
  
    #ifdef  DMA_MODE    
    char buffer[100];
    #else
    char buffer[100];
    #endif

    while (1) 
    {
      
      if(update)
      {
        
        update = 0;
        #ifndef DMA_MODE
        if(avgReady)
        {
          avgReady = 0;
          
          sprintf(buffer,"AVG AN0: %.2f[V], AVG AN1: %.2f[V]", an0Avg * 3.3 / 4095.0f, an1Avg * 3.3 / 4095.0f);
          TERM_TxStringXY(USART2, 10, 10, buffer);     
        }
        #endif
        
      }
      #ifdef DMA_MODE
        if(dmaBlockReady)
        {
          dmaBlockReady =0;

          uint32_t sum0 = 0;
          uint32_t sum1 = 0;
          
          for(uint16_t i =0; i< ADC_DMA_COUNT; i+=2)
          {
            sum0 += adcBuffer[i]; //AN0
            sum1 += adcBuffer[i+1];
          }

          an0Avg = (uint16_t)(sum0 / 1000);
          an1Avg = (uint16_t)(sum1 / 1000);

          sprintf(buffer, "DMA AVG AN0 : %.2f[V], DMA AVG AN1 : %.2f[V]",
                  an0Avg * 3.3 / 4095.0f,
                  an1Avg * 3.3 / 4095.0f);
          TERM_TxStringXY(USART2, 10, 10, buffer);

          //One-shot Rearm Logic
          DMA1_Channel1->CCR &= ~DMA_CCR_EN;
          DMA1_Channel1->CNDTR = ADC_DMA_COUNT;
          DMA1_Channel1->CCR |= DMA_CCR_EN;
          ADC1->CR |= ADC_CR_ADSTART;
        }
        #endif
        
    }
}

//TIM6  every 1ms checks if trigger happened
void TIM6_DAC_LPTIM1_IRQHandler(void)
{
  if(TIM6->SR & TIM_SR_UIF)
  {
    TIM6->SR &= ~TIM_SR_UIF;
    GPIO_Set(GPIOA, 5);  // set pa5 high at trigger instant
  }
  
  NVIC_ClearPendingIRQ(TIM6_DAC_LPTIM1_IRQn);
}

#ifndef DMA_MODE
//End of Conversion Reached 
void ADC1_COMP_IRQHandler(void)
{
  //End of Conversion
  if(ADC1->ISR & ADC_ISR_EOC)
  {
    uint16_t sample = (uint16_t) ADC1->DR; //reading the ADC_DR clears the EOC flag

    if(adcSeqIndex == 0) //that means 1st conversion
    {
      an0Value = sample;
      adcSeqIndex = 1; 
    }
    else  //that means 2nd conversion
    {
      an1Value = sample;
      adcSeqIndex = 0;
    }
    
  }

  //End of Sequence 
  if(ADC1->ISR & ADC_ISR_EOS)
  {
    ADC1->ISR = ADC_ISR_EOS; //clear EOS
    adcSeqIndex = 0; //forces next seq at an0
    
    //accumalate completed pair
    an0Sum += an0Value;
    an1Sum += an1Value;
    seqCount++;

    //Average every 1000 sequence
    if(seqCount >= 1000)
    {
      an0Avg =  (uint16_t)(an0Sum/1000);
      an1Avg =  (uint16_t)(an1Sum/1000);
      
      an0Sum = 0;
      an1Sum = 0; 
      seqCount = 0;
      
      avgReady = 1;
    }
    GPIO_Clear(GPIOA, 5);
    GPIO_Toggle(GPIOA, 6);
  }
  NVIC_ClearPendingIRQ(ADC1_COMP_IRQn);

}
#endif

#ifdef DMA_MODE
void ADC1_COMP_IRQHandler(void)
{
  if(ADC1->ISR & ADC_ISR_EOS)
  {
    ADC1->ISR = ADC_ISR_EOS;
    GPIO_Clear(GPIOA, 5);
    GPIO_Toggle(GPIOA, 6);
  }
  NVIC_ClearPendingIRQ(ADC1_COMP_IRQn);

}
void DMA1_Channel1_IRQHandler(void)
{
  if(DMA1->ISR & DMA_ISR_TCIF1)
  {
    DMA1->IFCR = DMA_IFCR_CTCIF1; //clear transfer complete(only the individual flag
    DMA1_Channel1->CCR &= ~DMA_CCR_EN; //disable, (cause we are not using circular modes)

    dmaBlockReady = 1;
   
  }
  NVIC_ClearPendingIRQ(DMA1_Channel1_IRQn);
}
#endif


/*************************** End of file ****************************/
