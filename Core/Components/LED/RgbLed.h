#ifndef INC_RGBLED_H_
#define INC_RGBLED_H_

#include "main.h"
#include "tim.h"

class RgbLed {
public:
    RgbLed(TIM_HandleTypeDef *htim);

    RgbLed(TIM_HandleTypeDef *htim, uint32_t red_channel, uint32_t green_channel, uint32_t blue_channel,
           uint32_t pwm_max_value = 1000);

    void init();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setColorDirect(uint16_t r, uint16_t g, uint16_t b);
    void turnOff();

private:
    TIM_HandleTypeDef *htim_;
    const uint32_t red_channel_;
    const uint32_t green_channel_;
    const uint32_t blue_channel_;
    const uint32_t pwm_max_value_;
};

#endif /* INC_RGBLED_H_ */
