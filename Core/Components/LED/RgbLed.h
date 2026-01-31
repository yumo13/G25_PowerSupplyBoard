#ifndef INC_RGBLED_H_
#define INC_RGBLED_H_

#include "main.h"
#include "tim.h"

// PWM制御のRGB LEDドライバ（アクティブロー）
class RgbLed {
public:
    // デフォルト配線: TIM3 CH4/CH2/CH1、最大デューティ1000
    RgbLed(TIM_HandleTypeDef *htim);

    // チャンネル割り当てとPWM分解能を指定
    RgbLed(TIM_HandleTypeDef *htim, uint32_t red_channel, uint32_t green_channel, uint32_t blue_channel,
           uint32_t pwm_max_value = 1000);

    // PWM出力開始しLEDを消灯状態にする
    void init();
    // 0-255で色指定（0=消灯, 255=最大）
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    // PWM比較値を直接指定（0..pwm_max_value）
    void setColorDirect(uint16_t r, uint16_t g, uint16_t b);
    // 消灯（全チャンネルOFF）
    void turnOff();

private:
    TIM_HandleTypeDef *htim_;
    const uint32_t red_channel_;
    const uint32_t green_channel_;
    const uint32_t blue_channel_;
    const uint32_t pwm_max_value_;
};

#endif /* INC_RGBLED_H_ */
