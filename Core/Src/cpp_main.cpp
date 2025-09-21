#include "cpp_main.h"
#include "RgbLed.h"
#include "LedTest.h"
#include "tim.h"

extern TIM_HandleTypeDef htim3;

void cpp_main(void)
{
    RgbLed led(&htim3);
    led.init();

    while (1)
    {
        testLedAll(led);
        HAL_Delay(1000);
    }
}
