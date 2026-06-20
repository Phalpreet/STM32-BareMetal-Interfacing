# Interrupt-Driven UART & Asynchronous Ring Buffer

## 📌 Project Overview
Standard polling-based serial communication often results in dropped bytes if the CPU is busy executing other tasks when data arrives. This project solves that by decoupling the hardware reception from the software processing using an asynchronous architecture.

## 🚀 Technical Highlights

### 1. Thread-Safe Circular Queue
Developed a custom ring buffer to handle incoming USART2 streams at $115200\text{ baud}$. 
* **Implementation:** The receive interrupt (`RXNEIE`) dumps payload bytes directly into the buffer array, automatically advancing the `Head` index and handling wrap-around logic. The main loop safely pulls data from the `Tail` index at its own pace, completely eliminating data loss during rapid transmission bursts.

### 2. UI Coordinate Debugging
During the development of the serial terminal interface, a logic bug caused specific UI elements (closing brackets) to overwrite themselves and fail to render.
* **Solution:** Debugged the rendering loop via the serial console, identified the coordinate mismanagement, and refactored the cursor positioning logic to ensure flawless terminal output.
