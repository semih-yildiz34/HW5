#include "lib_nn_kw.h"
#include "model_kw_int.h"
#include <stdint.h>

static inline int16_t relu_q15_sat(int32_t x){
  if (x <= 0) return 0;
  if (x > 32767) return 32767;
  return (int16_t)x;
}

uint8_t LIB_NN_KW_Predict(const int16_t *mfcc_q15)
{
  // Layer1: 13 -> 20 (ReLU)
  int16_t h1[KW_H1];

  for (int i = 0; i < KW_H1; i++){
    int32_t sum = (int32_t)b1_kw_q15[i];
    for (int j = 0; j < KW_IN; j++){
      // Q15*Q15 -> Q30 >>15 -> Q15
      sum += ((int32_t)W1_kw_q15[i*KW_IN + j] * (int32_t)mfcc_q15[j]) >> 15;
    }
    h1[i] = relu_q15_sat(sum);
  }

  // Output logits: 20 -> 5
  int32_t logits[KW_OUT];
  for (int k = 0; k < KW_OUT; k++){
    int32_t sum = (int32_t)b2_kw_q15[k];
    for (int i = 0; i < KW_H1; i++){
      sum += ((int32_t)W2_kw_q15[k*KW_H1 + i] * (int32_t)h1[i]) >> 15;
    }
    logits[k] = sum;
  }

  // Argmax
  uint8_t best = 0;
  int32_t bestv = logits[0];
  for (uint8_t k = 1; k < KW_OUT; k++){
    if (logits[k] > bestv){
      bestv = logits[k];
      best = k;
    }
  }
  return best;
}
