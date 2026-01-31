#include "RgbLed.h"

extern "C" {
#include "tim.h"
}

RgbLed::RgbLed(TIM_HandleTypeDef *htim)
    : htim_(htim), red_channel_(TIM_CHANNEL_4), green_channel_(TIM_CHANNEL_2), blue_channel_(TIM_CHANNEL_1),
      pwm_max_value_(1000) {}

RgbLed::RgbLed(TIM_HandleTypeDef *htim, uint32_t red_channel, uint32_t green_channel, uint32_t blue_channel,
               uint32_t pwm_max_value)
    : htim_(htim), red_channel_(red_channel), green_channel_(green_channel), blue_channel_(blue_channel),
      pwm_max_value_(pwm_max_value) {}

void RgbLed::init() {
    HAL_TIM_PWM_Start(htim_, red_channel_);
    HAL_TIM_PWM_Start(htim_, green_channel_);
    HAL_TIM_PWM_Start(htim_, blue_channel_);

    __HAL_TIM_SET_COMPARE(htim_, red_channel_, pwm_max_value_);
    __HAL_TIM_SET_COMPARE(htim_, green_channel_, pwm_max_value_);
    __HAL_TIM_SET_COMPARE(htim_, blue_channel_, pwm_max_value_);
}

void RgbLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t r_pulse = (pwm_max_value_ * (255 - r)) / 255;
    uint32_t g_pulse = (pwm_max_value_ * (255 - g)) / 255;
    uint32_t b_pulse = (pwm_max_value_ * (255 - b)) / 255;

    __HAL_TIM_SET_COMPARE(htim_, red_channel_, r_pulse);
    __HAL_TIM_SET_COMPARE(htim_, green_channel_, g_pulse);
    __HAL_TIM_SET_COMPARE(htim_, blue_channel_, b_pulse);
}

void RgbLed::setColorDirect(uint16_t r, uint16_t g, uint16_t b) {
    uint32_t r_pulse = (r > pwm_max_value_) ? pwm_max_value_ : r;
    uint32_t g_pulse = (g > pwm_max_value_) ? pwm_max_value_ : g;
    uint32_t b_pulse = (b > pwm_max_value_) ? pwm_max_value_ : b;

    __HAL_TIM_SET_COMPARE(htim_, red_channel_, r_pulse);
    __HAL_TIM_SET_COMPARE(htim_, green_channel_, g_pulse);
    __HAL_TIM_SET_COMPARE(htim_, blue_channel_, b_pulse);
}

void RgbLed::turnOff() {
    __HAL_TIM_SET_COMPARE(htim_, red_channel_, pwm_max_value_);
    __HAL_TIM_SET_COMPARE(htim_, green_channel_, pwm_max_value_);
    __HAL_TIM_SET_COMPARE(htim_, blue_channel_, pwm_max_value_);
}