# Bare-Metal SPI Instrumentation (MCP4812)

## 📌 Project Overview
This acts as the capstone project for this repository: a comprehensive, standalone arbitrary waveform generator. It integrates ADC readings, tactile button inputs, and serial console readouts, culminating in a custom bare-metal protocol driver to control an external dual-channel DAC chip.

## 🚀 Technical Highlights

### 1. Synchronous Protocol Driver Design
Rather than relying on abstraction layers, this project utilizes custom C libraries to configure the SPI peripheral at the register level to communicate with an external MCP4812 12-bit DAC.
* **Implementation:** Configured the SPI Baud Rate, Clock Polarity (CPOL), and Clock Phase (CPHA) to strictly match the IC's datasheet timing requirements. Built highly efficient bit-shifting logic to package 12-bit variable data alongside 4 command bits into a precise 16-bit payload.

### 2. Strict Chip Select (CS) Management
Data integrity in SPI communication requires precise control over the Chip Select line.
* **Implementation:** Engineered GPIO toggling logic to pull the CS line low at the exact start of the transmission and drive it high immediately upon the `SPI_SR_TXE` flag clearing, ensuring the external DAC correctly latches the incoming data packets.
