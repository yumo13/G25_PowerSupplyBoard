#include "cpp_main.h"
#include "RgbLed.h"
#include "LedTest.h"
#include "CanController.h"
#include "tim.h"
#include "can.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern CAN_HandleTypeDef hcan;

void cpp_main(void)
{
    RgbLed led(&htim3);
    led.init();

    // CANコントローラを初期化
    CanController::initialize(&hcan);

    // CANメッセージハンドラを設定
    CanController::onReceive(0x100, [&led](const CanMessage &msg)
                             {
    // ID 0x100: RGB LED制御
    if (msg.data.size() >= 3) {
        led.setColor(msg.data[0], msg.data[1], msg.data[2]);
    } });

    // 非常停止解除
    HAL_GPIO_WritePin(EMERGENCY_GPIO_Port, EMERGENCY_Pin, GPIO_PIN_SET);

    while (1)
    {
    }
}
