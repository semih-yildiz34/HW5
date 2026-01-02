# Q1 – Section 12.8  
## Keyword Spotting from Audio Signals (STM32-side Implementation)

**Course:** EE4065 – Embedded Digital Image Processing  
**Homework:** HW5

**Authors:**  
- Semih Yıldız – 150721029  
- Rüzbar Batı Okay – 150722048  

---

This folder contains the STM32-side implementation of **Section 12.8 – Keyword Spotting from Audio Signals**  
from the book:

> C. Ünsalan, B. Höke, E. Atmaca  
> *Embedded Machine Learning with Microcontrollers: Applications on STM32 Boards*,  
> Springer Nature, 2025.

---

## Overview

In this application, a keyword spotting neural network is trained offline using Python.
The trained model parameters are then exported as fixed-point C arrays and used for
inference on an STM32 microcontroller.

This repository focuses on the **embedded inference and data flow logic**, rather than
a full STM32CubeIDE project.

---

## Implementation Details

The STM32-side implementation includes:

- Receiving feature vectors (e.g. MFCCs) via serial communication
- Fixed-point neural network inference
- Decision logic for keyword classification

The following components are provided:

```text
stm32_skeleton/Core/
├── Inc/
│ ├── lib_nn_kw.h
│ ├── lib_serialaudio.h
│ └── model_kw_int.h
└── Src/
├── main.c
├── lib_nn_kw.c
└── lib_serialaudio.c


- `model_kw_int.h` contains the trained neural network weights and biases
  exported as fixed-point C arrays.
- `lib_nn_kw.c` implements the forward pass of the neural network.
- `lib_serialaudio.c` handles serial data reception.
- `main.c` connects data acquisition, inference, and output.

---

## Python Tools

The `python_tools/` folder contains helper scripts used for testing and development:

- `generate_dummy_mfcc.py`: Generates dummy MFCC-like feature vectors
- `send_features_mock.py`: Sends feature vectors to the STM32 over serial

These tools are used only for validation and are **not part of the embedded runtime**.

---

## Notes

- This is **not a complete STM32CubeIDE project**.
- HAL drivers, startup files, and linker scripts are intentionally omitted.
- The focus is on demonstrating the keyword spotting inference pipeline
  described in Section 12.8.

---

## Status

✔ Embedded inference logic implemented  
✔ Model integration completed  
✔ Matches Section 12.8 application description

