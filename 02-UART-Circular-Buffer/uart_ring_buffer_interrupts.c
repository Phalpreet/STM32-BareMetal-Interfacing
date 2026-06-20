#include <stdio.h>
#include <stdint.h>
#include "gpio.h"
#include "clock.h"
//#define Sol 
#define original

static void App_InitHardware(void)
{
    // Set the MCU system clock to 40 MHz
    Clock_InitPll(PLL_40MHZ);

    // Configure PA5 as output for the Nucleo LED
    GPIO_InitOutput(GPIOA, 5);

    // Make sure the LED starts off
    GPIO_Clear(GPIOA, 5);

    // Configure PC13 as input with pull-up for the user button
    GPIO_InitInputPullUp(GPIOC, 13);

    // Configure PA2 as alternate function AF1 for USART2 TX
    GPIO_InitAlternateF(GPIOA, 2, 1);

    // Configure PA3 as alternate function AF1 for USART2 RX
    GPIO_InitAlternateF(GPIOA, 3, 1);

    // Enable the peripheral clock for USART2
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    // Initialize USART2 for 115200 baud
    UART_Init(USART2, 115200, 1);

    // Enable receive interrupt on USART2
    USART2->CR1 |= USART_CR1_RXNEIE_RXFNEIE;

    // Enable UART error interrupt generation
    USART2->CR3 |= USART_CR3_EIE;

    // Enable the shared USART2 / LPUART2 interrupt line in the NVIC
    NVIC_EnableIRQ(USART2_LPUART2_IRQn);
}
#ifdef original

volatile uint8_t g_lastReceivedChar = 0;
volatile uint8_t g_receivedCount = 0;
volatile uint8_t g_newCharflag = 0; // tells main that a new ch is received



void USART2_LPUART2_IRQHandler(void)
{
  //snapshot of USART2 status register. => helps us inspect RX and error flag
  uint32_t isr = USART2->ISR;

  //check for uart error :
  // FE = framing error, NE = noise error, ORE = overrun error
  if(isr & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE))
  {
    GPIO_Set(GPIOA, 5); //led on

    //clear the error flasgs 
     USART2->ICR = USART_ICR_FECF | USART_ICR_NECF| USART_ICR_ORECF;
  }

  //if new byte received and is waiting in RDR
  if(isr & USART_ISR_RXNE_RXFNE)
  {
    uint8_t data = (uint8_t) USART2->RDR; //reading also removes byte from hardware receive register

    //save as global
    g_lastReceivedChar = data;

    //count increased
    g_receivedCount++;

    g_newCharflag = 1;
  }
}

int main(void)
{
    // Local buffer used to format the text response
    char txText[80];

    // Local snapshot of the last received character
    uint8_t localChar;

    // Local snapshot of the total received count
    uint32_t localCount;

    // Initialize all common hardware
    App_InitHardware();

    // Send a startup message to show the original version is running
    UART_TxStr(USART2, "\r\n[Original single-char version active]\r\n");

    // Main loop runs forever
    while (1)
    {
        // Check if the user button is pressed (active low on PC13)
        if (GPIO_Read(GPIOC, 13) == 0)
        {
            // Disable interrupts while updating the shared counter
            __disable_irq();

            // Clear the count back to zero
            g_receivedCount = 0;

            // Clear the new-character flag
            g_newCharflag = 0;

            // Re-enable interrupts
            __enable_irq();

            // Send a short message to confirm the clear action
            UART_TxStr(USART2, "\r\n[Count cleared]\r\n");

            // Wait until the button is released
            while (GPIO_Read(GPIOC, 13) == 0)
            {
                // Stay here until release
            }
        }

        // If a new character arrived, take a stable snapshot and print it
        if (g_newCharflag)
        {
            // Disable interrupts while copying shared variables
            __disable_irq();

            // Copy the last received character into a local variable
            localChar = g_lastReceivedChar;

            // Copy the current count into a local variable
            localCount = g_receivedCount;

            // Clear the ready flag because this event is now being handled
            g_newCharflag = 0;

            // Re-enable interrupts
            __enable_irq();

            // Format the response string
            snprintf(txText, sizeof(txText),
                     "Last char: '%c'   Count: %lu\r\n",
                     (char)localChar,
                     (unsigned long)localCount);

            // Send the response string back to the PC
            UART_TxStr(USART2, txText);
        }

        // Sleep until the next interrupt
        __WFI();
    }
}

#endif

#ifdef Sol
// Size of the circular receive buffer used in the buffered solution
#define RX_BUFFER_SIZE 256

// Number of bytes to send back to the PC at one time in the buffered solution
#define TX_CHUNK_SIZE 64

//circular buffer array that stores every received byte untill main process it
volatile uint8_t g_rxBuffer[RX_BUFFER_SIZE];

//where main ISR writes next byte
volatile uint16_t g_rxHead = 0;

//where main reads next byte
volatile uint16_t g_rxTail = 0;

//total bytes received
volatile uint16_t g_receivedCount = 0;

volatile uint8_t g_dataReady=0; //tells main if buffered data is available

volatile uint8_t g_bufferOverflow = 0;
//function to advance index and wrap around to 0
static uint16_t NextIndex(uint16_t index)
{
  //move to next and wrap around at the end
  return (uint16_t)((index + 1U) % RX_BUFFER_SIZE);
}

//interrupt handler for usart2
void USART2_LPUART2_IRQHandler(void)
{
  //read status and store to local
  uint32_t isr = USART2->ISR;

  //chcek for framing, noise, overrun errors
  if (isr & (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE))
  {
      // Turn on the LED if any UART error occurs
      GPIO_Set(GPIOA, 5);

      // Clear the UART error flags
      USART2->ICR = USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF;
  }
  
  //while data waiting in receive register
  while(USART2->ICR & USART_ISR_RXNE_RXFNE)
  {
    //read one byte
    uint8_t data = (uint8_t) USART2-> RDR;

    //increase count
    g_receivedCount++;

    //compute next head position
    uint16_t nextHead = NextIndex(g_rxHead);
    
    // If the next head is not equal to the tail, the buffer still has room
    if (nextHead != g_rxTail)
    {
       g_rxBuffer[g_rxHead] = data;// Store the received byte into the current head location

      // Advance the head index
       g_rxHead = nextHead;
      
      g_dataReady = 1;//tell main data ready to be processed
    }
    else
    {
      //buffer is full
      g_bufferOverflow = 1;
      GPIO_Set(GPIOA, 5);
    }
  }

}

int main(void)
{
  uint8_t txChunk[TX_CHUNK_SIZE];
  uint16_t chunkLen;
  App_InitHardware();
  UART_TxStr(USART2, "\r\n[Buffered solution]\r\n");

  while(1)
  {
     // Check if the user button is pressed (active low on PC13)
    if (GPIO_Read(GPIOC, 13) == 0)
    {
        // Disable interrupts while changing shared variables
        __disable_irq();

        // Clear the total received count
        g_receivedCount = 0;

        // Reset the circular buffer by making head and tail equal
        g_rxHead = 0;
        g_rxTail = 0;

        // Clear the data-ready flag because the buffer is now empty
        g_dataReady = 0;

        // Clear the overflow flag
        g_bufferOverflow = 0;

        // Re-enable interrupts
        __enable_irq();

        // Send a short message to confirm the clear action
      UART_TxStr(USART2, "\r\n[Count and buffer cleared]\r\n");

          // Wait until the button is released
          while (GPIO_Read(GPIOC, 13) == 0)
          {
              // Stay here}
          }
     }
  if(g_dataReady)
  {
    chunkLen = 0;

    __disable_irq();
    while((g_rxTail != g_rxHead) && (chunkLen < TX_CHUNK_SIZE))
    {
       // Copy one byte from the tail location into the local transmit chunk
      txChunk[chunkLen] = g_rxBuffer[g_rxTail];

      chunkLen++;
      g_rxTail = NextIndex(g_rxTail);

    }

    if(g_rxTail == g_rxHead) // If tail caught up to head, the ring buffer is empty now
    {
      // No more data is waiting
      g_dataReady = 0;
    }
    __enable_irq();
    
    if(chunkLen >0)
    {
      UART_TxBuffer(USART2, txChunk, chunkLen);
    }
   }


  }
}
#endif

