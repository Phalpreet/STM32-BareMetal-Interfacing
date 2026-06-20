# Zero-Overhead DSP Pipeline (ADC, DAC & Circular DMA)

## 📌 Project Overview
This project showcases advanced inter-peripheral communication and hardware acceleration. It progresses from basic interrupt-driven analog conversions to a fully automated Direct Digital Synthesis (DDS) pipeline that streams continuous waveforms without utilizing the CPU.

## 🚀 Technical Highlights

### 1. Hardware-Triggered Analog Loopback
Bridged the input and output boundaries of the microcontroller by linking peripherals.
* **Implementation:** Configured a basic timer (TIM6) to generate internal Trigger Output (TRGO) events at $10\text{ kHz}$. These events directly trigger the ADC1 hardware to take a reading, and upon completion, the ADC interrupt fires to instantly pipe that reading out of DAC1.

### 2. Circular DMA Sine Wave Generation
Generating high-frequency analog signals manually consumes massive CPU bandwidth. 
* **Implementation:** Designed a 256-sample sine wave lookup table in RAM. Configured DMA1 Channel 1 in Circular Mode to listen to the TIM6 TRGO line. On every trigger, the DMA controller automatically fetches the next data point from RAM and writes it to the DAC register. This achieves a perfectly clean $100\text{ kHz}$ analog signal with **0% CPU utilization**.
