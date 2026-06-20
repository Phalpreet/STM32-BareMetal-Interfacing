//******************
// GPIO Library
//
// CREATED: [Phalpreet Singh]
//
// FILE: gpio.c
//
// PURPOSE: Implementation of GPIO library functions for STM32G0 series.
//
//******************
#include "stm32g0b1xx.h"
#include "gpio.h"
#include <stdio.h> //well it be used to sprint
#include <stdint.h>
#include <ctype.h>

static void GPIO_EnablePortClock(GPIO_TypeDef* pPort)
{
    if      (pPort == GPIOA) RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    else if (pPort == GPIOB) RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    else if (pPort == GPIOC) RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    else if (pPort == GPIOD) RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    else if (pPort == GPIOE) RCC->IOPENR |= RCC_IOPENR_GPIOEEN;
#if defined(GPIOF)
    else if (pPort == GPIOF) RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
#endif
}

//validating that is pin is within accepting range(0-15)
static int Pinvalid(uint16_t PinNumber)
{
  return PinNumber <=15 ;
  
}
void GPIO_InitInput(GPIO_TypeDef* pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)

    if(!Pinvalid(PinNumber)) return;
  
    //Use the MODER register of the provided port to configure as input mode.
    //To work with a pin, we use its 2 bit field in the Moder Register.
    //Input mode means value = 0b00.
    //Create a mask to clear only those 2 bits.
    //(For example for pin5): mask is 0b...1100 0000 0000, which is made by shifting 0b11(0b...0000 0000 0011) by 10 positions 
    //This 10 position's number came from 5*2(pinNumber * 2) i.e. 0b11 << 10 -> 0b...0000 0000 0011
    //code for this is bellow :
    GPIO_EnablePortClock(pPort);
    //mask 
    uint32_t mask = GPIO_MODER_MODE0 << (PinNumber * 2); //GPIO_MODER_MODE0 = 0b11

    //now setting those 2 bits to 00(Input) See datasheet for more 
    pPort->MODER &= ~mask;
} 

void GPIO_InitOutput(GPIO_TypeDef* pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)

    if(!Pinvalid(PinNumber)) return;

    GPIO_EnablePortClock(pPort);
    //Clearing the bits as same as part 1
    uint32_t clearing_mask = GPIO_MODER_MODE0 << (PinNumber * 2); 
    pPort->MODER &=~clearing_mask;

    //To set the bits to output mode(0b01)
    //EX for pin 5 : value(01) is shifted again by 10 positions > pinNumber *2
    uint32_t set_value = GPIO_MODER_MODE0_0 << (PinNumber*2); //GPIO_MODER_MODE0_0 is 0b01
    pPort->MODER |= set_value;
}

void GPIO_Toggle(GPIO_TypeDef* pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)
    if(!Pinvalid(PinNumber)) return;
    //Use the ODR register of the provided port to toggle the pin state (XOR operation)
    //use of xor
    // If the bit is 1, 1 ^ 1 = 0.
    // If the bit is 0, 0 ^ 1 = 1.
     uint32_t mask = (1u << PinNumber);
    if (pPort->ODR & mask) pPort->BSRR = (mask << 16u);  // was 1 -> reset
    else                 pPort->BSRR = mask;            // was 0 -> set
}

void GPIO_Set(GPIO_TypeDef * pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)
    if(!Pinvalid(PinNumber)) return;
    //Use the ODR register of the provided port to SET the pin state (OR operation)
    //pPort->ODR |= (1 << PinNumber);
    pPort->BSRR = (1U << PinNumber); 
}

void GPIO_Clear(GPIO_TypeDef * pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)
    if(!Pinvalid(PinNumber)) return;
    //Use the ODR register of the provided port to CLEAR the pin state (AND operation)
    //pPort->ODR &= ~(1 << PinNumber);
    pPort->BSRR = (1U << (PinNumber + 16U));
}

int GPIO_Read(GPIO_TypeDef * pPort, uint16_t PinNumber)
{
    //Validate that the port pin is valid (0-15)
     if(!Pinvalid(PinNumber)) return-1;
    //Use the IDR register of the provided port to fetch the pin state
    //The IDR (Input Data Register) holds the current state of the input pins.
    uint32_t mask = (1 << PinNumber);
    
    //Return a true value if the pin is set, return a false value if not
    if ((pPort->IDR & mask) != 0)
    {
        return 1; // Pin is high
    }
    else
    {
        return 0; // Pin is low
    }
    //(pPort->IDR & mask) will be non-zero (true) if the pin is high,
    //    and zero (false) if the pin is low.
}

void GPIO_WritePin(GPIO_TypeDef *port, uint32_t pin, uint8_t value)
{
    if (value) GPIO_Set(port, pin);
    else       GPIO_Clear(port, pin);
}

void GPIO_InitAlternateF(GPIO_TypeDef* pPort, uint16_t PinNumber, uint16_t AF_Value)
{
    if (PinNumber > 15 || AF_Value > 15) return; // Basic error checking

    GPIO_EnablePortClock(pPort); 
    // Step 1: Set the pin's mode to 'Alternate Function' (10) in the MODER register.
    pPort->MODER &= ~(0x3 << (PinNumber * 2)); // Clear the 2 bits for the pin
    pPort->MODER |=  (0x2 << (PinNumber * 2)); // Set to '10' for AF mode

    // Step 2: Set the specific alternate function in the AFR registers.
    // There are two 32-bit AFR registers, AFR[0] (AFRL) and AFR[1] (AFRH).
    if (PinNumber < 8) // Use AFRL for pins 0-7
    {
        // Each pin gets 4 bits in the AFR register.
        pPort->AFR[0] &= ~(0xF << (PinNumber * 4));      // Clear the 4 bits for the pin
        pPort->AFR[0] |=  (AF_Value << (PinNumber * 4)); // Set the AF value
    }
    else // Use AFRH for pins 8-15
    {
        // The bit shift calculation is adjusted for the high register.
        pPort->AFR[1] &= ~(0xF << ((PinNumber - 8) * 4));
        pPort->AFR[1] |=  (AF_Value << ((PinNumber - 8) * 4));
    }
   // pPort->OSPEEDR |= (3U << (PinNumber * 2));
}
void delay_ms(volatile uint32_t ms)
{
    
    //ms *= 1600; 
    //ms *= 1900;
    //ms *= 1850;
    //ms *= 1840;
    ms *= 1779.8;
    while (ms--);
}

void GPIO_InitInputPullUp(GPIO_TypeDef *pPort, uint16_t PinNumber)
{
  if (!Pinvalid(PinNumber)) return;

  GPIO_EnablePortClock(pPort);            
  //Set the pin's mode to 'Input' (00) in the MODER register.
  
  uint32_t moder_mask = GPIO_MODER_MODE0 << (PinNumber * 2);
  pPort->MODER &= ~moder_mask;
  // The PUPDR registeruses 2 bits per pin.
    // '00' = No pull-up/pull-down
    // '01' = Enable Pull-up
    // '10' = Enable Pull-down
    // First, clear the 2 bits for this pin
  pPort->PUPDR &= ~(0x3 << (PinNumber * 2));
  pPort->PUPDR |=  (0x1 << (PinNumber * 2));//set the 2 bits to '01' for pull-up
}

//UART Library
//This assumes you have already Set pins TX and RX pins to the proper alternate function
//Ensure that the port clock is turned on for GPIO    
//Ensure that the peripheral clock is enabled for USART2 (RCC_APBENR1 (RM::5.4.15))
void UART_Init(USART_TypeDef* pUart, uint32_t baud_rate, char use_interrupt)
{
    /*Some steps here might no necessarily be placed inside this function*/  
    //Turn on the transmitter and receiver for the USART (USART_CR1 (RM::33.8.1))
    //Set the BAUD rate for the UART, using 10x fixed point arithmetic conversion with the known bus rate and 
    //the desired BAUD rate in the calculation. (USART_BRR (RM::33.8.5))
    //Turn on the USART (USART_CR1 (RM::33.8.1)) (must be done after configuration) 
    
    // Disable the USART peripheral before configuring("&= ~.. ": Clear the bit) 
    pUart->CR1 &= ~USART_CR1_UE;// (USART_CR1_UE is the bitmask for "USART Enable") 
                                // CR1 is Control registor 1 of USART Peripheral.
    
    //Set the baud rate (baud - bits per second)
    pUart->BRR = SystemCoreClock / baud_rate;//"pUart->BRR" access the baud rate registor
                                          //Systemclock/baud_rate was given in notes.

    //Enable the Transmistter(TE) and Receier(RE)
    pUart->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    //Enable the USART peripheral after the configuration
    pUart->CR1 |= USART_CR1_UE;
}

void UART_TxByte(USART_TypeDef* pUart, uint8_t data)
{
    //Wait for the TXFNF flag of USART_ISR to set (USART_ISR (RM::33.8.9))
    //or in other words, Wait until the Transmit data registor is empty.
     while (!(pUart->ISR & USART_ISR_TXE_TXFNF)); //"pUart->ISR" Access the Interrupt and Status Registor,
                                            //which contains all the hardware status flags
                                            //"USART_ISR_TXE_TXFNF" is the bitmask for "Transmit DATA Registor Empty" flag
                                            //The hardware sets its value to 1 when registor is empty and ready for the next byte.
    //Note: the TXFNF register name is odd in the derivative file!
    //When the value gets set to 1 then this code will run or it will remain in the empty loop
    //Write the data to USART_TDR (USART_TDR (RM::33.8.13))(Transmit data registor)
    pUart->TDR = data;
}

uint8_t UART_RxByte(USART_TypeDef* pUart , uint8_t* pData)
{
    //If the RXFNE flag is set in USART_ISR, read USART_RDR and place the result at the target of the provided 
    //data pointer (pData). Return a value indicating that the data target is valid. Note: the RXFNE register 
    //name is odd in the derivative file! Otherwise, if the RXFNE flag is clear in USART_ISR, 
    // return a value indicating that the data target is not valid.
    if(pUart->ISR & USART_ISR_RXNE_RXFNE)//"USART_ISR_RXNE_RXFNE" the hardware sets this value to 1 when 
                                   // new byte has arrived and is waiting
    {
        *pData = pUart->RDR; //"pUart->RDR" Accesses the Receive Data Registor, which holds the new byte.
        return 1;//there is data available
    }
    return 0;//no data available
}


void UART_TxStr(USART_TypeDef* pUart, const char* pData)
{
    //Transmits a string starting from *pData until the end of the string (null character)
    //Loop until we find the end-of-string character
    while (*pData != '\0')//pData is a pointer to a character(like h in hello), *pData is the value itself(h).
    {
        UART_TxByte(pUart, *pData);//Send out the character the pointer is pointing at
        //Move the pointer to the next character in memory(i.e h to e)
        pData++;

    }

    
}

//This is useful for sending raw data that might contain \0 characters, where UART_TxStr would fail.
void UART_TxBuffer(USART_TypeDef* pUart, const uint8_t* pData, uint16_t size)
{
    //Transmits an array of "size" bytes starting from *pData
    //Loop 'size' times
    for(uint16_t i = 0; i < size; i++)
    {
        UART_TxByte(pUart, (uint8_t)pData[i]);//Send the byte at the current index of the array
        //UART_TxByte(pUart, (uint8_t)*pData+1 );
    }
    //in summary the loop will send pData[0], then pData[1], then pData[2], etc., until all size bytes have been sent.

}


//Sends escape code to clear screen and sets the cursor position to the starting position

void TERM_ClearScreen(USART_TypeDef *pUSART)
{
  //From given table 
  //\x1b[2J 	Erase the complete of display.
  UART_TxStr(pUSART, "\x1b[2J");

  //\x1b[0;0H 	Go to position (0,0)

  UART_TxStr(pUSART, "\x1b[0;0H");

}

//Sends escape code to move cursor to specific position i.e X,Y coordinate
//Terminals setup is mostly Row,Col.
void TERM_GotoXY(USART_TypeDef *pUSART, int iCol, int iRow)
{
  char buffer[20];//hold the string value ie \x1b[0;22H

  sprintf(buffer,"\x1b[%d;%dH",iRow,iCol); //x1b[x;yH is the command (H is the set cursor thing) 
                                           //%d are placeholders for intezers 
  
  //go to x,y position
  UART_TxStr(pUSART, buffer);

}

//Moves to xy position and send the text
void TERM_TxStringXY(USART_TypeDef *pUSART, int iCol, int iRow, char *pStr)
{
  //moves to xy
  TERM_GotoXY(pUSART, iCol, iRow);

  //print string there
  UART_TxStr(pUSART, pStr);
}


//// Optional:


//unsigned char UART_RxByteB (USART_TypeDef * pUSART)
//{
//  Block waiting for USART_ISR_RXNE_RXFNE to set
//  Return the value RDR
//}


//stop and wait till a character is pressed
//the other one was not waiting it was just catching everything
unsigned char UART_RxByteB(USART_TypeDef *pUSART)
{
  while(!(pUSART->ISR & USART_ISR_RXNE_RXFNE));//"USART_ISR_RXNE_RXFNE" the hardware sets this value to 1 when 
                                   // new byte has arrived and is waiting

  return (unsigned char)pUSART->RDR;//"pUart->RDR" Accesses the Receive Data Registor, which holds the new byte.
}

// Optional:
//int UART_RxString (USART_TypeDef * pUSART, unsigned char * pTargetBuffer, unsigned short iBufferLength, _USART_RX_ENFORCE EnforceType)
//{
 //If the target buffer is NULL or the buffer length is < 1, return 0
 // While the target index (starts at 0) is less than the buffer length - 1:
 //   Block waiting for a character from the terminal
 //   If the character is ‘backspace’ (consult ASCII table), backup the target index (if possible, and echo the backspace character to the terminal
 //   If the character is ‘enter’, set the current buffer target to NUL and return the target position
 //   Depending on the enforcement type, if the character is valid, set the current buffer target to the received character, echo it, and advance the target position.
 //   If the target position equals the buffer length - 1, set the target buffer at the target position to NUL (or just blindly set the last buffer position to NUL).
 // Return the target position

int UART_RxString(USART_TypeDef *pUSART, unsigned char *pTargetBuffer, unsigned short iBufferLength, _USART_RX_ENFORCE EnforceType)
{
  //If the target buffer is NULL or the buffer length is < 1, return 0
  if(pTargetBuffer ==NULL || iBufferLength <1) return 0 ;

  unsigned short index = 0;
  unsigned char received_char;
  
  // While the target index (starts at 0) is less than the buffer length - 1:
  //or
  //loop all untill the total buffen space and leave one space as buffer always has staring ending character '\0'
  while(index < (iBufferLength-1))
  {
  //  Block waiting for a character from the terminal
  //get the chracter using the above made functions 
    received_char = UART_RxByteB(pUSART);
  

   // If the character is ‘enter’, set the current buffer target to NUL and return the target position
  // or
  // check if enter is pressed and 
    if(received_char == '\r')
    {
      break; //stop receiving characters and exit the loop
    }
  //   If the character is ‘backspace’ (consult ASCII table),
  //   backup the target index (if possible, and echo the backspace character to the terminal
    //or
    //check backspace pressed
    else if(received_char == '\b')
    {
      if(index>0)
      {
        index--;// Move the buffer index back one spot

        UART_TxStr(pUSART, "\x1b[1D"); // "\x1b[1D": Move cursor left 1 column
        UART_TxByte(pUSART, ' ');     // Overwrite with a space
        UART_TxStr(pUSART, "\x1b[1D"); // "\x1b[1D": Move cursor left 1 column
        
        //CAN USE SIMPLE 
        //UART_TxByte(pUSART, '\b');
        //UART_TxByte(pUSART, ' ');
        //UART_TxByte(pUSART, '\b');
      }
    }

    //   Depending on the enforcement type, if the character is valid,
    // set the current buffer target to the received character, echo it, and advance the target position.
    else
    {
      int isValid =0;//invalid untill proven

      switch (EnforceType)
      {
        case ENFORCE_ANY:
          //check if it is printable 
          if(isprint(received_char)){isValid=1;}
          break;

        case ENFORCE_DIGIT:
          //check if it is 0-9
          if(isdigit(received_char)){isValid=1;}
          break;
        
        case ENFORCE_APLHA:
          //check if it is letter 
          if(isalpha(received_char)){isValid=1;}
          break;
        
        case ENFORCE_HEX:
          //check if it is hex 
          if(isxdigit(received_char)){isValid=1;}
          break;
      }
    //only if valid character
      if(isValid)
      {
        //store
        pTargetBuffer[index] = received_char;//set the current buffer target to the received character, e
        UART_TxByte(pUSART,received_char);//echo it,
        index++;//advance the target position.
      }

        
    }
  }
  //If the target position equals the buffer length - 1,
  //set the target buffer at the target position to NUL (or just blindly set the last buffer position to NUL).
  //or
  // When the loop is done (break or full),
  // add the NUL(\0) to make it a valid C-string.
  pTargetBuffer[index]= '\0';
  UART_TxStr(pUSART, "\r\n");

  // Return the target position
  return index;
}