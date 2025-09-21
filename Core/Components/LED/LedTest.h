#ifndef LED_TEST_H
#define LED_TEST_H

#include "RgbLed.h"

void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b);
void testLedBasicColors(RgbLed &led);
void testLedFade(RgbLed &led);
void testLedRainbow(RgbLed &led);
void testLedAll(RgbLed &led);

#endif