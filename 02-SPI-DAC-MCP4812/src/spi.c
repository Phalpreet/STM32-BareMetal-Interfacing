#include "spi.h"

/**
 * @brief Initializes SPI in master mode using software slave management.
 * @param spi Pointer to SPI peripheral, for example SPI2.
 * @param baudDivider SPI baud-rate divider.
 * @param cpol Clock polarity.
 * @param cpha Clock phase.
 */
void SPI_Init(SPI_TypeDef *spi, uint32_t baudDivider, uint8_t cpol, uint8_t cpha)
{
  // Disable SPI before configuration
  spi->CR1 &= ~SPI_CR1_SPE;

  // Clear baud rate, polarity, phase, master, and software NSS settings
  spi->CR1 &= ~(SPI_CR1_BR |
                SPI_CR1_CPOL |
                SPI_CR1_CPHA |
                SPI_CR1_MSTR |
                SPI_CR1_SSM |
                SPI_CR1_SSI |
                SPI_CR1_LSBFIRST);

  // Set baud rate divider
  spi->CR1 |= baudDivider;

  // Set clock polarity if needed
  if(cpol)
  {
    spi->CR1 |= SPI_CR1_CPOL;
  }

  // Set clock phase if needed
  if(cpha)
  {
    spi->CR1 |= SPI_CR1_CPHA;
  }

  // Master mode + software slave management
  spi->CR1 |= SPI_CR1_MSTR;
  spi->CR1 |= SPI_CR1_SSM;
  spi->CR1 |= SPI_CR1_SSI;

  // Configure 8-bit data size
  spi->CR2 &= ~SPI_CR2_DS;
  spi->CR2 |= SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;

  // RXNE flag becomes active after 8-bit received data
  spi->CR2 |= SPI_CR2_FRXTH;

  // Enable SPI
  spi->CR1 |= SPI_CR1_SPE;
}

/**
 * @brief Sends and receives one SPI byte.
 * @param spi Pointer to SPI peripheral.
 * @param data Byte to transmit.
 * @return Byte received.
 */
uint8_t SPI_TxRxByte(SPI_TypeDef *spi, uint8_t data)
{
  uint8_t rxData;

  // Wait until transmit register is empty
  while(!(spi->SR & SPI_SR_TXE))
  {
  }

  // Write 8-bit data to SPI data register
  *((__IO uint8_t *)(&spi->DR)) = data;

  // Wait until received data is available
  while(!(spi->SR & SPI_SR_RXNE))
  {
  }

  // Read received data
  rxData = *((__IO uint8_t *)(&spi->DR));

  // Wait until SPI is no longer busy
  while(spi->SR & SPI_SR_BSY)
  {
  }

  return rxData;
}