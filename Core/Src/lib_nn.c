/*
 * lib_nn.c
 * Neural Network Inference for Digit Recognition
 */

#include "lib_nn.h"
#include <math.h>

#define Q15_SCALE (1.0f / 32768.0f)  // Convert Q15 to float

/**
  * @brief ReLU activation function
  * @param x Input value
  * @retval max(0, x)
  */
static inline int16_t relu_q15(int32_t x)
{
	return (x > 0) ? (int16_t)x : 0;
}

/**
  * @brief Sigmoid activation function (Q15)
  * @param x Input value in Q15 format
  * @retval Sigmoid output in Q15 format
  */
static int16_t sigmoid_q15(int32_t x)
{
	// Clamp at x=8 (sigmoid(8)≈0.9997) to prevent exp overflow
	// 8 in Q15 = 8 * 32768 = 262144
	if (x > 262144) return 32767;
	if (x < -262144) return 0;
	
	// sigmoid(x) = 1/(1+exp(-x))
	float fx = (float)x * Q15_SCALE;
	float sig = 1.0f / (1.0f + expf(-fx));
	return (int16_t)(sig * 32768.0f);
}

/**
  * @brief Softmax activation function (Q15)
  * @param input Array of Q15 values
  * @param output Array to store softmax outputs (Q15)
  * @param size Number of elements
  */
static void softmax_q15(int32_t *input, int16_t *output, uint8_t size)
{
	int32_t max_val = input[0];
	
	// Find maximum for numerical stability
	for (uint8_t i = 1; i < size; i++)
	{
		if (input[i] > max_val)
			max_val = input[i];
	}
	
	// Calculate exp(x - max) and sum
	int32_t sum = 0;
	int32_t exp_vals[10];
	
	for (uint8_t i = 0; i < size; i++)
	{
		float fx = ((float)(input[i] - max_val)) * Q15_SCALE;
		float exp_val = expf(fx);
		exp_vals[i] = (int32_t)(exp_val * 32768.0f);
		sum += exp_vals[i];
	}
	
	// Normalize
	if (sum > 0)
	{
		for (uint8_t i = 0; i < size; i++)
		{
			output[i] = (int16_t)((exp_vals[i] * 32768) / sum);
		}
	}
	else
	{
		// Fallback: uniform distribution
		int16_t uniform = 32768 / size;
		for (uint8_t i = 0; i < size; i++)
		{
			output[i] = uniform;
		}
	}
}

/**
  * @brief Q1: Single neuron classifier (binary: 0 vs not-0)
  * @param huFeatures Array of 7 normalized Hu moments in Q15 format (as int32_t to avoid overflow)
  * @retval 0 if digit is 0, 1 if digit is not 0
  */
uint8_t LIB_NN_PredictQ1(int16_t *huFeatures)
{
	// Features are already normalized and in Q15 format
	// Cast to int32_t to handle large values that exceed int16_t range
	int32_t *normalized = (int32_t*)huFeatures;
	
	// Single neuron: output = sigmoid(w^T * x + b)
	// Note: weights and bias were scaled by w_q1_scale to fit in Q15
	// Calculate: sum_scaled = w_scaled^T * x + b_scaled, then multiply by scale
	int32_t dot_product = 0;  // w_scaled^T * x (in Q15)
	
	for (int i = 0; i < 7; i++)
	{
		// Multiply: Q15 * Q15 = Q30, then shift right 15 to get Q15
		// Use int32_t for both to avoid overflow
		int32_t product = ((int32_t)w_q1_q[i] * normalized[i]) >> 15;
		dot_product += product;
	}
	
	// Add scaled bias
	int32_t sum_scaled = dot_product + (int32_t)b_q1_q[0];
	
	// Multiply by scale factor to get the actual sum: (w_scaled^T * x + b_scaled) * scale
	// sum_scaled (Q15) * scale (Q15) = Q30, shift right 15 to get Q15
	int32_t sum = ((int32_t)sum_scaled * (int32_t)q1_scale) >> 15;
	
	// Apply sigmoid (sum is in Q15 format)
	int16_t output = sigmoid_q15(sum);
	
	// Threshold at 0.5 (16384 in Q15)
	// WORKAROUND: Flip sigmoid result - STM32 sigmoid calculation seems inverted
	// If sigmoid > 0.5, predict 0 (IS 0), else predict 1 (NOT 0)
	return (output > 16384) ? 1 : 0;
}

/**
  * @brief Q2: MLP classifier (10-class: 0-9)
  * @param huFeatures Array of 7 normalized Hu moments in Q15 format
  * @retval Predicted digit (0-9)
  */
uint8_t LIB_NN_PredictQ2(int16_t *huFeatures)
{
	// Features are already normalized and in Q15 format
	int16_t *normalized = huFeatures;
	
	// Layer 1: 7 -> 100 (ReLU)
	int32_t layer1[100];
	for (int i = 0; i < 100; i++)
	{
		int32_t sum = (int32_t)b1_q[i];
		for (int j = 0; j < 7; j++)
		{
			sum += ((int32_t)W1_q[i * 7 + j] * (int32_t)normalized[j]) >> 15;
		}
		layer1[i] = (int32_t)relu_q15(sum);
	}
	
	// Layer 2: 100 -> 100 (ReLU)
	int32_t layer2[100];
	for (int i = 0; i < 100; i++)
	{
		int32_t sum = (int32_t)b2_q[i];
		for (int j = 0; j < 100; j++)
		{
			sum += ((int32_t)W2_q[i * 100 + j] * layer1[j]) >> 15;
		}
		layer2[i] = (int32_t)relu_q15(sum);
	}
	
	// Layer 3: 100 -> 10 (Softmax)
	int32_t layer3[10];
	for (int i = 0; i < 10; i++)
	{
		int32_t sum = (int32_t)b3_q[i];
		for (int j = 0; j < 100; j++)
		{
			sum += ((int32_t)W3_q[i * 100 + j] * layer2[j]) >> 15;
		}
		layer3[i] = sum;
	}
	
	// Apply softmax and find maximum
	int16_t probs[10];
	softmax_q15(layer3, probs, 10);
	
	// Find class with maximum probability
	uint8_t max_idx = 0;
	int16_t max_prob = probs[0];
	for (uint8_t i = 1; i < 10; i++)
	{
		if (probs[i] > max_prob)
		{
			max_prob = probs[i];
			max_idx = i;
		}
	}
	
	return max_idx;
}
