/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include "stdint.h"
#include "stm32g0b1xx.h"
#include "clock.h"
#include "gpio.h"
#include "spi.h"
#include "adc.h"

#define DAC_MAX_VALUE     1023U
#define WAVE_STEP         16U
#define WAVE_UPDATE_MS    1U

#define SPI_DAC_A         0b00010000
#define SPI_DAC_B         0b10010000

#define SW1_PORT GPIOA
#define SW1_PIN  0

#define SW2_PORT GPIOA
#define SW2_PIN  1

#define SW3_PORT GPIOA
#define SW3_PIN  4

#define SW4_PORT GPIOB
#define SW4_PIN  1

#define BUTTON_PRESSED 0U
#define BOUNCE_MS      30U

typedef enum
{
  WAVE_SAWTOOTH = 0,
  WAVE_TRIANGLE,
  WAVE_COSINE,
  WAVE_ADC_FOLLOWER
} WaveformMode_t;

volatile uint8_t updateWaveformFlag = 0;
volatile uint16_t msCounter = 0;
volatile uint32_t systemMs = 0;

volatile WaveformMode_t currentMode = WAVE_SAWTOOTH;
#define COSINE_TABLE_SIZE 64U

const uint16_t cosineTable[COSINE_TABLE_SIZE] =
{
  1023, 1021, 1013, 1001, 984, 962, 936, 906,
  872, 834, 793, 749, 702, 654, 604, 553,
  512, 461, 410, 360, 312, 265, 221, 180,
  142, 108, 78, 52, 30, 13, 1, 0,
  1, 13, 30, 52, 78, 108, 142, 180,
  221, 265, 312, 360, 410, 461, 512, 553,
  604, 654, 702, 749, 793, 834, 872, 906,
  936, 962, 984, 1001, 1013, 1021, 1023, 1021
};

uint8_t cosineIndex = 0;
uint16_t sawValue = 0;
uint16_t triangleValue = 0;
uint8_t triangleGoingUp = 1;
uint16_t adcValue = 0;

void SPI_WriteDAC(unsigned char dac, unsigned int value);
void UpdateWaveform(void);
void UpdateSawtooth(void);
void UpdateTriangle(void);
void CheckButtons(void);
void UpdateCosine(void);
uint8_t ButtonIsPressed(GPIO_TypeDef *port, uint8_t pin);
void SetWaveformMode(WaveformMode_t newMode);
void UpdateAdcFollower(void);
void InitAdcFollower(void);
/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) 
{
  //Enable power interface clock
  RCC->APBENR1 |= RCC_APBENR1_PWREN;

  //Enable GPIO clocks
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
  
  //Enable USART2 and SPI2 clocks 
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;
  RCC->APBENR1 |= RCC_APBENR1_SPI2EN;

  //System clock to 40 MHz 
  Clock_InitPll(PLL_40MHZ);

  // 1 ms SysTick
  SysTick_Config(SystemCoreClock /1000);

  //USART2 for printf
  GPIO_InitAlternateF(GPIOA, 2, 1);
  GPIO_InitAlternateF(GPIOA, 3, 1);
  UART_Init(USART2, 115200, 0);
  
  printf("Function Generator\r\n");
  printf("Sawtooth\r\n");

  //Led 
  GPIO_InitOutput(GPIOA, 5);

  //SPI pins
  GPIO_InitOutput(GPIOC, 3); // CS -> PC3
  GPIO_Set(GPIOC, 3); //CS starts HIGH

  GPIO_InitAlternateF(GPIOB, 13, 0); //SPI2_SCK
  GPIO_InitAlternateF(GPIOB, 14, 0); //SPI2_MISO not using
  GPIO_InitAlternateF(GPIOB, 15, 0); //SPI2_MOSI
   
  //SPI2: 10MHz, CPOL = 1, CPHA = 1
  SPI_Init(SPI2,DIV_4, 1, 1);
  GPIO_InitInputPullUp(SW1_PORT, SW1_PIN);
  GPIO_InitInputPullUp(SW2_PORT, SW2_PIN);
  GPIO_InitInputPullUp(SW3_PORT, SW3_PIN);
  GPIO_InitInputPullUp(SW4_PORT, SW4_PIN);

  InitAdcFollower();
  while(1)
  {
    
    CheckButtons();

    if(updateWaveformFlag)
    {
      updateWaveformFlag = 0;
      UpdateWaveform();
    }
  }
}


void SysTick_Handler(void)
{
  systemMs++;
  msCounter++;

  if(msCounter >= WAVE_UPDATE_MS)
  {
    msCounter = 0;
    updateWaveformFlag = 1;
  }
}
void UpdateWaveform(void)
{
  switch (currentMode)
  {
    case WAVE_SAWTOOTH:
    UpdateSawtooth();
    break;
    
    case WAVE_TRIANGLE:
    UpdateTriangle();
    break;
    
    case WAVE_COSINE:
    UpdateCosine();
    break;
    
    case WAVE_ADC_FOLLOWER:
    UpdateAdcFollower();
    break;
    
    default :
    currentMode = WAVE_SAWTOOTH;
    break;
  }
}
void UpdateAdcFollower(void)
{
  uint16_t dacValue;

  // Start ADC conversion
  ADC1->CR |= ADC_CR_ADSTART;

  // Wait until end of sequence
  while(!(ADC1->ISR & ADC_ISR_EOS))
  {
  }

  // Read 12-bit ADC result
  adcValue = ADC1->DR;

  // Clear EOS flag by writing 1 to it
  ADC1->ISR = ADC_ISR_EOS;

  // Convert 12-bit ADC value 0-4095 to 10-bit DAC value 0-1023
  dacValue = adcValue >> 2;

  // Send converted value to MCP4812
  SPI_WriteDAC(SPI_DAC_A, dacValue);
}
void UpdateCosine(void)
{
  SPI_WriteDAC(SPI_DAC_A, cosineTable[cosineIndex]);

  cosineIndex++;

  if(cosineIndex >= COSINE_TABLE_SIZE)
  {
    cosineIndex = 0;
  }
}
void UpdateSawtooth(void)
{
  sawValue += WAVE_STEP;
  if(sawValue > DAC_MAX_VALUE)
  {
    sawValue = 0;
  }

  SPI_WriteDAC(SPI_DAC_A, sawValue);
}

void UpdateTriangle(void)
{
  if(triangleGoingUp)
  {
    if((triangleValue + WAVE_STEP) >= DAC_MAX_VALUE)
    {
      triangleValue = DAC_MAX_VALUE;
      triangleGoingUp = 0;
    }
    else
    {
      triangleValue += WAVE_STEP;
    }
  }
  else
  {
    if(triangleValue <= WAVE_STEP)
    {
      triangleValue = 0;
      triangleGoingUp = 1;
    }
    else
    {
      triangleValue -= WAVE_STEP;
    }
  }

  SPI_WriteDAC(SPI_DAC_A, triangleValue);
}

void SPI_WriteDAC(unsigned char dac, unsigned int value)
{
  unsigned char byte1, byte2;

  value &= 0x3FF;  // Keep only 10 bits
  value <<= 2;     // Move DAC value into bits 11:2

  byte1 = dac;
  byte2 = (unsigned char)value;

  byte1 |= ((unsigned char)(value >> 8) & 0x0F);

  GPIO_Clear(GPIOC, 3);      // CS low
  SPI_TxRxByte(SPI2, byte1); // Send upper byte
  SPI_TxRxByte(SPI2, byte2); // Send lower byte
  GPIO_Set(GPIOC, 3);        // CS high
}

void CheckButtons(void)
{
  static uint32_t lastButtonTime = 0;

  // Basic bounce protection
  if((systemMs - lastButtonTime) < BOUNCE_MS)
  {
    return;
  }

  if(ButtonIsPressed(SW1_PORT, SW1_PIN))
  {
    lastButtonTime = systemMs;
    SetWaveformMode(WAVE_SAWTOOTH);
  }
  else if(ButtonIsPressed(SW2_PORT, SW2_PIN))
  {
    lastButtonTime = systemMs;
    SetWaveformMode(WAVE_COSINE);
  }
  else if(ButtonIsPressed(SW3_PORT, SW3_PIN))
  {
    lastButtonTime = systemMs;
    SetWaveformMode(WAVE_TRIANGLE);
  }
  else if(ButtonIsPressed(SW4_PORT, SW4_PIN))
  {
    lastButtonTime = systemMs;
    SetWaveformMode(WAVE_ADC_FOLLOWER);
  }
}
void SetWaveformMode(WaveformMode_t newMode)
{
  if(currentMode == newMode)
  {
    return;
  }

  currentMode = newMode;

  printf("Function Generator\r\n");

  switch(currentMode)
  {
    case WAVE_SAWTOOTH:
      printf("Sawtooth\r\n");
      break;

    case WAVE_TRIANGLE:
      printf("Triangle\r\n");
      break;

    case WAVE_COSINE:
      printf("Offset Cosine\r\n");
      break;

    case WAVE_ADC_FOLLOWER:
      printf("ADC Follower\r\n");
      break;

    default:
      printf("Unknown\r\n");
      break;
  }
}
uint8_t ButtonIsPressed(GPIO_TypeDef *port, uint8_t pin)
{
  if(GPIO_Read(port, pin) == BUTTON_PRESSED)
  {
    return 1;
  }

  return 0;
}

void InitAdcFollower(void)
{
  // Set PB0 as analog input
  GPIOB->MODER |= (0x3U << (0 * 2));

  // No pull-up/pull-down for analog input
  GPIOB->PUPDR &= ~(0x3U << (0 * 2));

  ADC1_Init();

  
  ADC1_SetPrescalerDiv10();

  ADC1_Calibrate();

  ADC1_SelectChannels(ADC_CHSELR_CHSEL8);

  ADC1_Enable();
}
/*************************** End of file ****************************/
