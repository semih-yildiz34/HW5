# EE4065.1 - Introduction to Embedded Image Processing

## Homework 4: Digit Recognition on STM32 Microcontroller

---

## Student Information

| Student Name | Student ID |
|-------------|------------|
| Rüzgar Batı Okay | 150722048 |
| Semih Yıldız | 150721029 |

 
**Course:** EE4065.1 - Introduction to Embedded Image Processing

---

Digit recognition system using two different neural networks (single neuron and MLP) with Hu moments feature extraction. Which is applied on a STM32-based board

## Project Structure

```
Homework 4/
├── python/
│   ├── training/              # Model training scripts
│   │   ├── q1_10_9_mnist_single_neuron.py
│   │   └── q2_11_8_mnist_mlp.py
│   ├── prep/                  # Model preparation scripts
│   │   ├── extract_models.py
│   │   ├── quantize_params.py
│   │   └── generate_headers.py
│   ├── transfer_stm32/        # STM32 communication & testing
│   │   ├── py_digit_recognition.py  # Main test script
│   │   ├── py_serialimg.py          # Serial communication
│   │   ├── analyze_results.py       # Results analysis
│   │   ├── stm32_test_results.csv   # Test results
│   │   └── test_images/              # Test digit images (0-9)
│   ├── models/                # Trained Keras models
│   │   ├── mnist_single_neuron.h5
│   │   └── mlp_mnist_model.h5
│   ├── params/                # Model parameters
│   │   ├── extracted_params.npz
│   │   ├── quantized_params_q15.npz
│   │   └── mnist_hu_norm_stats*.npz
│   └── headers/               # C headers for STM32
│       ├── model_q1_int.h
│       └── model_q2_int.h
└── stm32/                     # STM32 firmware
    └── Core/
        ├── Src/
        │   ├── main.c
        │   ├── lib_image.c
        │   ├── lib_nn.c
        │   └── lib_serialimage.c
        └── Inc/
            ├── lib_image.h
            ├── lib_nn.h
            └── lib_serialimage.h
```

## Test Images

Test images are located in: `python/transfer_stm32/test_images/`

- `digit_0.png` through `digit_9.png` (10 MNIST digit images)

## Results

**⚠️ Note: Current results have significant mistakes.**

### Training Results:

#### Q1 - Single Neuron Classifier (0 vs not-0):
![Single Neuron Classifier Confusion Matrix](python/outputs/Single%20Neuron%20Classifier%20Confusion%20Matrix%20(0%20vs%20not-0).jpg)

#### Q2 - MLP Classifier (0-9):
![MLP MNIST Confusion Matrix](python/outputs/MLP%20MNIST%20Confusion%20Matrix%20(0..9).jpg)

### STM32 Hardware Results:
- **File**: `python/transfer_stm32/stm32_test_results.csv`
- **Q1 (Binary: 0 vs NOT-0)**: 60.00% accuracy
- **Q2 (Digit Classification: 0-9)**: 20.00% accuracy

### Python Simulation Results:
- **File**: `python/transfer_stm32/python_simulation_results.csv`
- Run `test_python_simulation.py` to generate results using Python simulation of STM32 logic

### Error Patterns (STM32):
- Q1: Digits 2, 4, 5, 9 incorrectly classified as "0"
- Q2: Consistent misclassifications:
  - 0 -> 6, 1 -> 3, 2 -> 8, 3 -> 8
  - 5 -> 4, 6 -> 4, 7 -> 8, 9 -> 3
  - Only digits 4 and 8 are 100% correct

### Current Performance Comparison (100 test cases):

**STM32 Hardware:**
- Q1 Accuracy: 60/100 = 60.00%
- Q2 Accuracy: 20/100 = 20.00%

**Python Simulation:**
- Q1 Accuracy: 80/100 = 80.00%
- Q2 Accuracy: 60/100 = 60.00%

**Analysis:** Since the Python simulation performs better than STM32 hardware, we believe there are most likely implementation issues in STM32. However, we are not certain on the solutions.

### Analyze Results:
```bash
cd python/transfer_stm32
# Analyze both STM32 and Python simulation results
python analyze_results.py

# Or analyze specific CSV
python analyze_results.py stm32_test_results.csv
python analyze_results.py python_simulation_results.csv
```

## Code Locations

### Python Scripts:
- **Training**: `python/training/`
- **Model Prep**: `python/prep/`
- **STM32 Testing**: `python/transfer_stm32/`

### STM32 Firmware:
- **Main**: `stm32/Core/Src/main.c`
- **Image Processing**: `stm32/Core/Src/lib_image.c`
- **Neural Network**: `stm32/Core/Src/lib_nn.c`
- **Serial Communication**: `stm32/Core/Src/lib_serialimage.c`

## Models

### Trained Models:
- **Q1 (Single Neuron)**: `python/models/mnist_single_neuron.h5`
- **Q2 (MLP)**: `python/models/mlp_mnist_model.h5`

### Quantized Parameters:
- **Q15 Format**: `python/params/quantized_params_q15.npz`
- **C Headers**: `python/headers/model_q1_int.h`, `model_q2_int.h`

## Usage

### Test on STM32 Hardware:
```bash
cd python/transfer_stm32
# Batch test (10 cycles)
python py_digit_recognition.py

# Single image test
python py_digit_recognition.py test_images/digit_0.png
```

### Test Python Simulation:
```bash
cd python/transfer_stm32
# Run Python simulation (replicates STM32 logic)
python test_python_simulation.py
```

## Workflow

1. **Train Models**: Run scripts in `python/training/`
2. **Extract & Quantize**: Run scripts in `python/prep/`
3. **Generate Headers**: Headers are created in `python/headers/`
4. **Flash STM32**: Copy headers to `stm32/Core/Inc/` and rebuild
5. **Test**: Run `py_digit_recognition.py` to test on STM32

