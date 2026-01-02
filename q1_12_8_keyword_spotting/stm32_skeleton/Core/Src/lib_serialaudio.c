#include "lib_serialaudio.h"
#include "main.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

SerialAudioStatus LIB_SERIALAUDIO_ReceiveMFCC(int16_t *out_mfcc_q15, uint32_t timeout_ms)
{
  uint8_t start = 0;
  if (HAL_UART_Receive(&huart2, &start, 1, timeout_ms) != HAL_OK) return SERIAL_AUDIO_ERR;
  if (start != 0xA5) return SERIAL_AUDIO_ERR;

  // 13 * int16 = 26 byte
  uint8_t buf[KW_IN * 2];
  if (HAL_UART_Receive(&huart2, buf, sizeof(buf), timeout_ms) != HAL_OK) return SERIAL_AUDIO_ERR;

  for (int i = 0; i < KW_IN; i++){
    // little-endian
    uint16_t lo = buf[i*2 + 0];
    uint16_t hi = buf[i*2 + 1];
    out_mfcc_q15[i] = (int16_t)((hi<<8) | lo);
  }
  return SERIAL_AUDIO_OK;
}

void LIB_SERIALAUDIO_SendClass(uint8_t class_id)
{
  HAL_UART_Transmit(&huart2, &class_id, 1, 100);
}
