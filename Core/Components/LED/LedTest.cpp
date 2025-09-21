#include "LedTest.h"
#include <math.h>

void hsvToRgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    int i = (int)(h / 60.0f) % 6;
    float f = (h / 60.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float red, green, blue;

    switch (i)
    {
    case 0:
        red = v;
        green = t;
        blue = p;
        break;
    case 1:
        red = q;
        green = v;
        blue = p;
        break;
    case 2:
        red = p;
        green = v;
        blue = t;
        break;
    case 3:
        red = p;
        green = q;
        blue = v;
        break;
    case 4:
        red = t;
        green = p;
        blue = v;
        break;
    case 5:
        red = v;
        green = p;
        blue = q;
        break;
    default:
        red = 0;
        green = 0;
        blue = 0;
        break;
    }

    *r = (uint8_t)(red * 255.0f);
    *g = (uint8_t)(green * 255.0f);
    *b = (uint8_t)(blue * 255.0f);
}

void testLedBasicColors(RgbLed &led)
{
    led.setColor(255, 0, 0);
    HAL_Delay(1000);

    led.setColor(0, 255, 0);
    HAL_Delay(1000);

    led.setColor(0, 0, 255);
    HAL_Delay(1000);

    led.turnOff();
    HAL_Delay(500);
}

void testLedFade(RgbLed &led)
{
    const int delay_ms = 5;

    for (int i = 0; i <= 255; ++i)
    {
        led.setColor(i, 0, 0);
        HAL_Delay(delay_ms);
    }

    for (int i = 255; i >= 0; --i)
    {
        led.setColor(i, 0, 0);
        HAL_Delay(delay_ms);
    }

    for (int i = 0; i <= 255; ++i)
    {
        led.setColor(0, i, 0);
        HAL_Delay(delay_ms);
    }

    for (int i = 255; i >= 0; --i)
    {
        led.setColor(0, i, 0);
        HAL_Delay(delay_ms);
    }

    for (int i = 0; i <= 255; ++i)
    {
        led.setColor(0, 0, i);
        HAL_Delay(delay_ms);
    }

    for (int i = 255; i >= 0; --i)
    {
        led.setColor(0, 0, i);
        HAL_Delay(delay_ms);
    }
}

void testLedRainbow(RgbLed &led)
{
    const int delay_ms = 25;

    for (int cycle = 0; cycle < 3; ++cycle)
    {
        for (float hue = 0.0f; hue < 360.0f; hue += 2.0f)
        {
            uint8_t r, g, b;
            hsvToRgb(hue, 1.0f, 1.0f, &r, &g, &b);
            led.setColor(r, g, b);
            HAL_Delay(delay_ms);
        }
    }
}

void testLedAll(RgbLed &led)
{
    testLedBasicColors(led);
    testLedFade(led);
    testLedRainbow(led);
    led.turnOff();
    HAL_Delay(500);
}