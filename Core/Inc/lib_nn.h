/*
 * lib_nn.h
 * Neural Network Inference for Digit Recognition
 */

#ifndef INC_LIB_NN_H_
#define INC_LIB_NN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "model_q1_int.h"
#include "model_q2_int.h"

#define NN_OK			((int8_t)0)
#define NN_ERROR		((int8_t)-1)

// Q1: Single neuron classifier (binary: 0 vs not-0)
// Returns: 0 if digit is 0, 1 if digit is not 0
// Note: huFeatures should be int32_t[7] in Q15 format (cast to int16_t* for compatibility)
uint8_t LIB_NN_PredictQ1(int16_t *huFeatures);

// Q2: MLP classifier (10-class: 0-9)
// Returns: predicted digit (0-9)
uint8_t LIB_NN_PredictQ2(int16_t *huFeatures);

#ifdef __cplusplus
}
#endif

#endif /* INC_LIB_NN_H_ */
