#ifndef LED_TEST_H
#define LED_TEST_H

#include "RgbLed.h"

// HSV(0-360, 0-1, 0-1) -> RGB(0-255) 変換
void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);
// 基本色の点灯シーケンス
void testLedBasicColors(RgbLed &led);
// 各チャンネルのフェード
void testLedFade(RgbLed &led);
// 色相スイープ
void testLedRainbow(RgbLed &led);
// 一通りのテスト
void testLedAll(RgbLed &led);

#endif