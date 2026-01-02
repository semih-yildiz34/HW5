# Q2 – Section 12.9  
## Handwritten Digit Recognition from Digital Images

**Course:** EE4065 – Embedded Digital Image Processing  
**Homework:** HW5  

**Authors:**  
- Semih Yıldız – 150721029  
- Rüzbar Batı Okay – 150722048  

---

This folder contains a **complete STM32CubeIDE project** implementing  
**Section 12.9 – Handwritten Digit Recognition from Digital Images**  
from the book:

> C. Ünsalan, B. Höke, E. Atmaca  
> *Embedded Machine Learning with Microcontrollers: Applications on STM32 Boards*,  
> Springer Nature, 2025.

---

## Overview

In this application, a multi-layer neural network is trained offline to recognize
handwritten digits (0–9) using Hu moment features extracted from grayscale images.

The trained model is exported as fixed-point C arrays and deployed on an STM32
microcontroller for real-time inference.

---

## Project Structure

The folder `stm32_project/` contains a **fully importable STM32CubeIDE project**:

```text

stm32_project/
├── Core/
│ ├── Inc/
│ └── Src/
├── Drivers/
│ ├── CMSIS/
│ └── STM32F4xx_HAL_Driver/
├── Startup/
├── image_transfer_nucleof446.ioc
├── STM32F446RETX_FLASH.ld
├── STM32F446RETX_RAM.ld
├── .project
└── .cproject

```


---

## Implementation Details

The embedded pipeline follows the steps described in Section 12.9:

1. Grayscale digit images (28×28) are received via serial communication
2. Hu moments (7 features) are computed on the STM32
3. Features are normalized using precomputed mean and standard deviation
4. A fixed-point multi-layer perceptron performs digit classification (0–9)
5. The predicted digit is transmitted back via serial

Key source files:

- `lib_image.c / .h` – Hu moment feature extraction
- `lib_nn.c / .h` – Fixed-point neural network inference
- `model_q2_int.h` – Exported neural network weights and biases
- `main.c` – Application control flow

---

## STM32 Configuration

- MCU: **STM32F446RE**
- IDE: **STM32CubeIDE**
- Framework: **STM32 HAL + CMSIS**
- Communication: UART (serial image transfer)

The `.ioc` file can be opened in STM32CubeMX to inspect or regenerate
the hardware configuration.

---

## How to Import

1. Open STM32CubeIDE  
2. `File → Import → Existing STM32CubeIDE Project`
3. Select the `stm32_project/` directory
4. Build and run

---

## Status

✔ Complete STM32CubeIDE project  
✔ HAL and CMSIS drivers included  
✔ Model inference verified  
✔ Matches Section 12.9 application description

