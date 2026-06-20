# Custom Bare-Metal Hardware Abstraction Layer

## 📌 Overview
This directory contains the foundational, reusable C libraries used across all projects in this repository. Rather than relying on standard vendor-provided abstraction layers (which can mask underlying hardware behavior), these drivers were written from scratch by directly manipulating the memory-mapped registers of the STM32G0.

## 🚀 Key Engineering Highlights

### 1. Register-Level PLL Clock Correction (`clock.c`)
Accurate system timing is the backbone of reliable firmware. During development, I identified a discrepancy in a standard header file's Phase-Locked Loop (PLL) configuration formula that prevented proper clock initialization.
* **Solution:** I recalculated the correct multipliers and dividers based on the STM32 reference manual and implemented a custom initialization sequence. This guarantees the core bus runs at exactly $64\text{ MHz}$, ensuring stable baud rates and precise timer scaling.

### 2. Modular Peripheral Management (`gpio.c` / `exti.c`)
Developed a lightweight, highly efficient modular architecture. 
* **Implementation:** Utilized precise bitwise masking to configure `MODER`, `PUPDR`, and EXTI registers safely, ensuring that configuring one pin does not inadvertently overwrite the state of neighboring pins on the same port.
