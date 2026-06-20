//******************
//GPIO Library
//
// CREATED: 10/27/2023, by Carlos Estay
//
// FILE: gpio.h
//
#include "stm32g0b1xx.h"

#ifndef GPIO_H
#define GPIO_H

typedef enum GPIO_IO_PullModeTypedef__
    {
        PullMode_None = 0b00,
        PullMode_PullUp = 0b01,
        PullMode_PullDown = 0b10,
        PullMode_Mask = 0b11
    }IO_PullMode;

    typedef enum GPIO_IO_TypeTypedef__
    {
        Type_PushPull = 0,
        Type_OpenDrain = 1,
    }IO_Type;
  /******Prototypes******/
  


  /// @param GPIO Port
  /// @param GPIO pin
  void GPIO_InitInput(GPIO_TypeDef*, uint16_t);

  /// @brief Set GPIO pin as input
  /// @param GPIO Port
  /// @param GPIO pin
  void GPIO_InitOutput(GPIO_TypeDef*, uint16_t);

  /// @brief Set pull-up or pull-down
  /// @param GPIO Port
  /// @param GPIO pin
  /// @param mode
  void GPIO_SetPullMode(GPIO_TypeDef*, uint16_t, IO_PullMode);

  /// @brief Set type of Output: Push-Pull or Open Collector
  /// @param GPIO Port
  /// @param GPIO pin
  /// @param type
  void GPIO_SetType(GPIO_TypeDef*, uint16_t, IO_Type);

  /// @brief Set alternate function (AFx MUX)
  /// @param GPIO Port
  /// @param GPIO pin
  /// @param AF
  void GPIO_InitAlternateF(GPIO_TypeDef*, uint16_t, uint16_t);

  /// @brief Set bit in Port
  /// @param Port
  /// @param pin
  void GPIO_Set(GPIO_TypeDef*, uint16_t);

  /// @brief Clear bit in Port
  /// @param Port
  /// @param pin
  void GPIO_Clear(GPIO_TypeDef*, uint16_t);

  /// @brief Toggle bit in Port
  /// @param Port
  /// @param pin
  void GPIO_Toggle(GPIO_TypeDef* , uint16_t);

  /// @brief Reads pin in Port
  /// @param Port
  /// @param pin
  int GPIO_Read(GPIO_TypeDef*, uint16_t);

  /// @brief Set alternate function (AFx MUX) for a pin.
  /// @param pPort 
  /// @param PinNumber 
  /// @param AF_Value

  /// @brief Set alternate function (AFx MUX) for a pin.
  /// @param pPort GPIO Port (e.g., GPIOA)
  /// @param PinNumber The pin number (0-15)
  /// @param AF_Value The alternate function value (0-15)
  void GPIO_InitAlternateF(GPIO_TypeDef*, uint16_t, uint16_t);

  /// @brief Time delay function
  /// @param ms time to delay in milliseconds
  void delay_ms(volatile uint32_t ms);

  void GPIO_InitInputPullUp(GPIO_TypeDef* pPort, uint16_t PinNumber);

#endif /* GPIO_H */

//******************
//UART Library
//
// CREATED: 11/05/2023, by Carlos Estay
// Used by Phalpreet Singh  
// FILE: uart.h
//
#ifndef UART_H
#define UART_H

typedef enum
{
  ENFORCE_ANY,    // permit any printable character
  ENFORCE_DIGIT,  // permit only 0-9
  ENFORCE_HEX,     // permit only 0-9,a-f,A-F
  ENFORCE_APLHA     //permit only a-z,A-Z
} _USART_RX_ENFORCE;

/// @brief 
/// @param  UartStructPtr
/// @param  BaudRate
/// @param  Interrupt 
void UART_Init(USART_TypeDef*, uint32_t,  char use_interrupt);

/// @brief 
/// @param  UartStructPtr
/// @param  byte
void UART_TxByte(USART_TypeDef*, uint8_t);

/// @brief 
/// @param  string
void UART_TxStr(USART_TypeDef*, const char*);

/// @brief transmit a buffer of a given size
/// @param  uart
/// @param  buffer
/// @param  size
void UART_TxBuffer(USART_TypeDef*, const uint8_t*, uint16_t);

/// @brief read a byte, non-blocking,
/// @param  pointer to a character
/// @return 1 if byte read, 0 if not
uint8_t UART_RxByte(USART_TypeDef* , uint8_t*); 

/// @brief Clears the entire terminal screen and moves cursor to starting (1,1).
/// @param pUSART The USART peripheral USART1/2
void TERM_ClearScreen(USART_TypeDef* pUSART);

/// @brief Moves the terminal cursor to a specific (column, row) position.
/// @param pUSART The USART peripheral USART1/2
/// @param iCol The column (X coordinate)
/// @param iRow The row (Y coordinate)
void TERM_GotoXY(USART_TypeDef* pUSART, int iCol, int iRow);

/// @brief Moves the cursor and prints a string at that location.
/// @param pUSART The USART peripheral USART1/2
/// @param iCol The column (X coordinate)
/// @param iRow The row (Y coordinate)
/// @param pStr The NUL-terminated string to print
void TERM_TxStringXY(USART_TypeDef* pUSART, int iCol, int iRow, char* pStr);

/// @brief Blocks (waits) until a byte is received, then returns it.
/// @param pUSART UART
/// @return The 8-bit byte that was received.
unsigned char UART_RxByteB(USART_TypeDef* pUSART);

/// @brief Reads a string from the terminal, stores it in a buffer.
/// @param pUSART The USART 
/// @param pTargetBuffer A pointer to the buffer to store the string in.
/// @param iBufferLength The maximum size of the buffer.
/// @param EnforceType type of allowed input.
/// @return The number of characters received (not including NUL).
int UART_RxString(USART_TypeDef* pUSART, unsigned char* pTargetBuffer, unsigned short iBufferLength, _USART_RX_ENFORCE EnforceType);

#endif /* UART_H */ 