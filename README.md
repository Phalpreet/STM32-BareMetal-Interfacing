# STM32G0 Bare-Metal Firmware Engineering Portfolio

Welcome to my embedded systems portfolio. This repository contains a collection of low-level, bare-metal firmware applications developed for the **STM32G031** microcontroller architecture. 

The primary objective of these projects is to bypass heavy Hardware Abstraction Layers (HAL) to program control registers directly. This approach demonstrates a deep understanding of hardware-software boundaries, memory-mapped peripherals, timing optimization, and inter-peripheral hardware acceleration.

## 🛠️ Core Technical Competencies
* **Protocol Driving:** Bare-metal SPI and UART transceiver management, including custom packet framing.
* **Hardware Acceleration:** Utilizing inter-peripheral triggers (TRGO) and Direct Memory Access (DMA) to stream data with zero CPU utilization.
* **Low-Latency Interrupts:** Advanced Nested Vector Interrupt Controller (NVIC) configurations, including asynchronous ring buffers and hardware debouncing.
* **Clock & Power Management:** Register-level Phase-Locked Loop (PLL) configuration and optimization of flash latency for high-speed core execution.

## 📂 Production Repository Structure

| Module | Core Peripherals | Engineering Highlights |
| :--- | :--- | :--- |
| ⚙️ [**Core_Drivers**](./Core_Drivers) | Custom HAL | Lightweight, reusable C libraries built from scratch for GPIO, Timers, EXTI, and precise System Clock (PLL) initialization. |
| 📁 [**01: Clock, SysTick & Timers**](./01-Clock-SysTick-Timers) | SysTick, TIM6, LSE | High-speed $64\text{ MHz}$ PLL configuration and a thread-safe uptime tracker using atomic critical sections. |
| 📁 [**02: UART Circular Buffer**](./02-UART-Circular-Buffer) | USART, NVIC | Zero-drop, interrupt-driven serial transceiver featuring an asynchronous ring buffer to protect incoming high-speed data. |
| 📁 [**03: Input Capture & EXTI**](./03-Input-Capture-EXTI) | TIM15, EXTI | Real-time PWM signal profiling using dual-edge hardware capture and custom debouncing matrices for external switches. |
| 📁 [**04: ADC-DAC-DMA Pipeline**](./04-ADC-DAC-DMA-Pipeline) | ADC, DAC, Circular DMA | High-speed Direct Digital Synthesis (DDS) driving a 256-sample lookup table via zero-overhead hardware triggers. |
| 📁 [**05: SPI Function Generator**](./05-SPI-Function-Generator) | SPI, MCP4812 DAC | Standalone instrumentation system controlling an external 12-bit DAC over SPI with strict chip-select management. |

---
