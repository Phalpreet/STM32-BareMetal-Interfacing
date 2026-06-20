#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32g0b1xx.h"

// SPI baud-rate divider settings.
// These values go directly into SPI_CR1_BR bits.
#define DIV_2      0U
#define DIV_4      SPI_CR1_BR_0
#define DIV_8      SPI_CR1_BR_1
#define DIV_16     (SPI_CR1_BR_1 | SPI_CR1_BR_0)
#define DIV_32     SPI_CR1_BR_2
#define DIV_64     (SPI_CR1_BR_2 | SPI_CR1_BR_0)
#define DIV_128    (SPI_CR1_BR_2 | SPI_CR1_BR_1)
#define DIV_256    (SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0)

/**
 * @brief Initializes an SPI peripheral as master.
 * @param spi Pointer to SPI peripheral, for example SPI2.
 * @param baudDivider SPI baud divider, for example DIV_4.
 * @param cpol Clock polarity: 0 or 1.
 * @param cpha Clock phase: 0 or 1.
 */
void SPI_Init(SPI_TypeDef *spi, uint32_t baudDivider, uint8_t cpol, uint8_t cpha);

/**
 * @brief Sends one byte over SPI and reads the received byte.
 * @param spi Pointer to SPI peripheral, for example SPI2.
 * @param data Byte to transmit.
 * @return Byte received during transmission.
 */
uint8_t SPI_TxRxByte(SPI_TypeDef *spi, uint8_t data);

#endif