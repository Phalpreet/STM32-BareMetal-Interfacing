# STM32 SPI Interfacing & Signal Generation (MCP4812)

## 📌 Project Overview
This repository demonstrates bare-metal SPI (Serial Peripheral Interface) communication between an STM32G031 microcontroller and an MCP4812 12-bit Digital-to-Analog Converter (DAC). The project focuses on low-level register configuration, precise data formatting, and peripheral synchronization to generate accurate analog voltage outputs from digital signals.

## 🛠️ Technologies & Tools
* **Microcontroller:** STM32G031
* **Peripheral Device:** MCP4812 (12-bit Dual DAC)
* **Language:** Embedded C
* **Protocols:** SPI (Serial Peripheral Interface)
* **Concepts:** Bitmasking, Packet Formatting, Bare-metal Driver Development

## 🚀 Key Features & Technical Highlights

### 1. Bare-Metal SPI Driver Development
Rather than relying on hardware abstraction layers (HAL), this project utilizes custom C libraries to configure the SPI peripheral at the register level.
* **Implementation:** Configured the SPI Baud Rate control, Clock Polarity (CPOL), and Clock Phase (CPHA) to strictly match the timing requirements detailed in the MCP4812 datasheet. 

### 2. Payload Formatting & Control Logic
The MCP4812 requires a specific 16-bit transfer sequence, combining 4 control bits (channel selection, gain, shutdown) with 12 bits of data.
* **Implementation:** Developed highly efficient bit-shifting and masking logic to correctly package the 12-bit variable data alongside the necessary command bits before initiating the SPI transmission.

### 3. Manual Chip Select (CS) Management
To ensure data integrity during transmission, the Chip Select line must be tightly controlled.
* **Implementation:** Implemented precise GPIO toggling logic to pull the CS line low exactly before transmission and drive it high immediately upon completion, ensuring the DAC correctly latches the incoming data.

## 📂 Repository Structure
* `/src` - Source files (`.c`), including the main loop and SPI transmission functions.
* `/inc` - Header files (`.h`), containing register definitions and DAC control macros.
* `/docs` - MCP4812 Datasheet and system wiring diagrams.

## 💡 What I Learned
Developing this driver deepened my understanding of synchronous serial communication. Translating the timing diagrams and command registers from the MCP4812 datasheet into functional C code significantly improved my ability to integrate external integrated circuits with microcontrollers.
