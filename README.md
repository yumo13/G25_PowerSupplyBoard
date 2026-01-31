# PowerSupplyBoard

STM32F303K8T6マイコン向けの電源基板ファームウェアです。

## 主な構成
- `Core/Src/main.c` : HAL 初期化と `cpp_main()` 呼び出し
- `Core/Src/cpp_main.cpp` : C++ 側のメイン処理（CAN/LED/ブザーなど）
- `Core/Components` : アプリケーション機能モジュール
  - `LED` / `CAN` など

## ピン配置（現状）
`Core/Inc/main.h` と `Core/Src/gpio.c` に基づく定義です。

| 信号名 | ポート/ピン | 用途 | 備考 |
| --- | --- | --- | --- |
| `EMERGENCY` | GPIOA PA0 | 非常停止制御 | `HAL_GPIO_WritePin` で制御 |
| `CAN_STBY` | GPIOA PA10 | CAN スタンバイ制御 | GPIO 出力 |
| `LED_B` | GPIOB PB4 | RGB LED (B) | TIM3 CH1 (PWM) |
| `LED_G` | GPIOB PB5 | RGB LED (G) | TIM3 CH2 (PWM) |
| `LED_R` | GPIOB PB7 | RGB LED (R) | TIM3 CH4 (PWM) |
| (Buzzer) | GPIOA PA9 | ブザー PWM 出力 | TIM2 CH3 (PWM) |

## タイマー使用状況
| タイマー | チャンネル | 役割 | 関連ファイル |
| --- | --- | --- | --- |
| TIM2 | CH3 | ブザー PWM | `cpp_main.cpp` (`setBuzzerFrequency`) |
| TIM3 | CH1/CH2/CH4 | RGB LED PWM | `RgbLed.cpp` / `tim.c` |
