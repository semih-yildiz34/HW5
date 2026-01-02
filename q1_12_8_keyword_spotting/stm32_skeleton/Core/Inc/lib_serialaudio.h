#pragma once
#include <stdint.h>

typedef enum {
  SERIAL_AUDIO_OK = 0,
  SERIAL_AUDIO_ERR = -1
} SerialAudioStatus;

SerialAudioStatus LIB_SERIALAUDIO_ReceiveMFCC(int16_t *out_mfcc_q15, uint32_t timeout_ms);
void LIB_SERIALAUDIO_SendClass(uint8_t class_id);
