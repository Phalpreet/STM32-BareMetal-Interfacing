# STM32G031 Hardware Interfacing & Debugging

## 📌 Project Overview
This repository contains custom embedded C configurations and hardware interfacing solutions for the STM32G031 microcontroller. The project focuses on establishing reliable UART communication, configuring Direct Memory Access (DMA), and ensuring precise system timing through custom clock libraries. 

The primary goal of this project was to move beyond standard auto-generated code to deeply understand and troubleshoot the hardware-software boundary.

## 🛠️ Technologies & Tools
* **Microcontroller:** STM32G031
* **Language:** Embedded C
* **Protocols & Peripherals:** UART, DMA, GPIO
* **Concepts:** System Clock Configuration, Bare-metal Debugging, Terminal Rendering

## 🚀 Key Features & Technical Highlights

### 1. Custom PLL Clock Configuration
Accurate system timing is critical for peripheral functionality. During development, a discrepancy in the standard header file's Phase-Locked Loop (PLL) configuration formula was preventing the proper initialization of clock speeds. 
* **Solution:** Identified the faulty logic in the source formula, recalculated the correct multipliers/dividers, and implemented a custom clock library to ensure the STM32G031 ran at the precise required frequency for stable DMA and UART operations.

### 2. UART Terminal Rendering & Coordinate Logic
Developed a terminal interface relying on precise UART data transmission. A persistent display bug was causing UI elements—specifically closing brackets—to fail to render on the terminal.
* **Solution:** Traced the issue to coordinate overwriting within the rendering loop. Refactored the display logic to correctly manage cursor positioning and coordinate tracking, resulting in a flawless terminal output.

## 📂 Repository Structure
* `/src` - Source files (`.c`), including main application logic and custom peripheral drivers.
* `/inc` - Header files (`.h`), including the corrected clock configuration libraries.


## 💡 What I Learned
This project reinforced the importance of thoroughly verifying datasheet specifications against provided software libraries. Troubleshooting the clock formula and UART coordinate logic significantly improved my ability to trace memory and timing issues at the register level.
