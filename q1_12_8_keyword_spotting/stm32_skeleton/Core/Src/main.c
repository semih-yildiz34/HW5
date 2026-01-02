#include "main.h"
#include "lib_serialaudio.h"
#include "lib_nn_kw.h"
#include "model_kw_int.h"

UART_HandleTypeDef huart2;

static void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);
static void MX_GPIO_Init(void);

static inline int16_t clamp_q15(int32_t v){
  if (v > 32767) return 32767;
  if (v < -32768) return -32768;
  return (int16_t)v;
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  int16_t mfcc_q15[KW_IN];

  while (1)
  {
    if (LIB_SERIALAUDIO_ReceiveMFCC(mfcc_q15, 2000) == SERIAL_AUDIO_OK)
    {
      // Normalize: (x-mean)/std  (hepsi Q15)
      // (x-mean) Q15, divide std ~ Q15 -> burada dummy stats var; basitçe mean/std üzerinden gösteriyoruz.
      for (int i = 0; i < KW_IN; i++){
        int32_t x = mfcc_q15[i];
        int32_t mean = mfcc_mean_q15[i];
        int32_t std  = mfcc_std_q15[i];  // dummy 32768 => /1
        int32_t num = (x - mean) << 15;  // Q15 -> Q30 (for division)
        int32_t y = (std != 0) ? (num / std) : 0; // back to Q15
        mfcc_q15[i] = clamp_q15(y);
      }

      uint8_t cls = LIB_NN_KW_Predict(mfcc_q15);
      LIB_SERIALAUDIO_SendClass(cls);
    }
  }
}
