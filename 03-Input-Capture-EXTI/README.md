# Hardware Signal Profiling & EXTI Debouncing

## 📌 Project Overview
This module demonstrates how to handle unpredictable external real-world signals. It features two distinct subsystems: a hardware-driven PWM analyzer and an asynchronous matrix button debouncer, both designed to execute without blocking the main CPU loop.

## 🚀 Technical Highlights

### 1. Dual-Edge Input Capture (TIM15)
Measuring high-frequency signals using software polling is highly inaccurate. 
* **Implementation:** Configured TIM15 with dual-channel input capture (Channel 1 tracking rising edges, Channel 2 tracking falling edges). This allows the microcontroller to calculate the exact frequency and duty cycle of an incoming PWM signal completely in hardware, updating variables via interrupts only when a full period is captured.

### 2. Asynchronous Switch Debouncing (EXTI)
Mechanical switches inherently "bounce," causing false multi-click triggers.
* **Implementation:** Configured External Interrupts (EXTI) across multiple GPIO ports. Instead of using `delay()` functions that freeze the processor, I implemented a time-stamp based software debouncing matrix using the SysTick timer. This cleanly filters out mechanical noise and successfully differentiates between short clicks and long-press events.
