#pragma once
#include <stdint.h>

// ---- Model dims ----
#define KW_IN   13
#define KW_H1   20
#define KW_OUT  5

// ---- Normalization stats (Q15) ----
// mean=0, std=1 gibi düşün (dummy). İstersen güncellersin.
static const int16_t mfcc_mean_q15[KW_IN] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const int16_t mfcc_std_q15[KW_IN] = {
  32768,32768,32768,32768,32768,32768,32768,32768,32768,32768,32768,32768,32768
};

// ---- Weights (Q15) ----
// Not: Bunlar “gerçek eğitim” değil, örnek olsun diye deterministik küçük sayılar.
// W1: [KW_H1 x KW_IN]
static const int16_t W1_kw_q15[KW_H1 * KW_IN] = {
  // 20 satır x 13 kolon = 260 değer
  // Basit pattern: küçük katsayılar
  #define W1ROW(a) a,a,a,a,a,a,a,a,a,a,a,a,a
  W1ROW(  820), W1ROW(  410), W1ROW(  205), W1ROW( -205), W1ROW( -410),
  W1ROW(  300), W1ROW( -300), W1ROW(  150), W1ROW( -150), W1ROW(  600),
  W1ROW( -600), W1ROW(  250), W1ROW( -250), W1ROW(  100), W1ROW( -100),
  W1ROW(  700), W1ROW( -700), W1ROW(  350), W1ROW( -350), W1ROW(   50)
  #undef W1ROW
};

// b1: [KW_H1]
static const int16_t b1_kw_q15[KW_H1] = {
  200, -100, 50, 0, 75, -50, 25, 0, 0, 100,
  -80, 40, 0, 0, 20, 60, -60, 30, -30, 10
};

// W2: [KW_OUT x KW_H1] = 5 x 20 = 100
static const int16_t W2_kw_q15[KW_OUT * KW_H1] = {
  // class0
  400, 200, 100, -100, -200, 300, -300, 150, -150, 250,
  -250, 120, -120, 80, -80, 220, -220, 60, -60, 40,
  // class1
  -300, -150, -80, 80, 150, -220, 220, -120, 120, -200,
  200, -60, 60, -40, 40, -180, 180, -30, 30, -20,
  // class2
  100, 120, 140, 160, 180, 60, 80, 100, 120, 140,
  160, 180, 200, 220, 240, 50, 40, 30, 20, 10,
  // class3
  -100, -120, -140, -160, -180, -60, -80, -100, -120, -140,
  -160, -180, -200, -220, -240, -50, -40, -30, -20, -10,
  // class4
  250, -250, 220, -220, 180, -180, 140, -140, 100, -100,
  60, -60, 40, -40, 30, -30, 20, -20, 10, -10
};

// b2: [KW_OUT]
static const int16_t b2_kw_q15[KW_OUT] = { 50, -25, 10, -10, 0 };

// Keyword labels (opsiyonel)
static const char* kw_labels[KW_OUT] = {"yes","no","up","down","stop"};
