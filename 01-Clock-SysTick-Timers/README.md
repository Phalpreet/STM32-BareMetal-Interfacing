# Clock Optimization & Thread-Safe Timekeeping

## 📌 Project Overview
This module establishes the timing foundation required for stable embedded systems. It demonstrates how to initialize the core processor to its maximum stable frequency and utilize system timers for precise, conflict-free scheduling and uptime tracking.

## 🚀 Technical Highlights

### 1. Atomic Critical Sections
Tracking continuous uptime (Days, Hours, Minutes, Seconds) using the $1\text{ ms}$ SysTick interrupt introduces the risk of data corruption if the main loop tries to read a variable exactly when the interrupt updates it.
* **Solution:** Implemented atomic blocks (`__disable_irq();` / `__enable_irq();`) around volatile variable reads in the main execution thread. This prevents race conditions and ensures reliable timekeeping string formatting.

### 2. LSE Crystal Calibration
Utilized **TIM14** to capture the oscillations of the Low-Speed External (LSE) $32.768\text{ kHz}$ RTC crystal, calculating delta ticks to measure and calibrate internal oscillator drift.
