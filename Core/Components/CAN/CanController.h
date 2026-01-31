#ifndef CAN_CONTROLLER_H
#define CAN_CONTROLLER_H

#include "can.h"
#include "main.h"
#include <cstdint>
#include <functional>
#include <map>
#include <queue>
#include <vector>

enum class CanIdType {
    STANDARD = 0, // 11-bit ID (0x000 - 0x7FF)
    EXTENDED = 1  // 29-bit ID (0x00000000 - 0x1FFFFFFF)
};

struct CanMessage {
    uint32_t id;
    std::vector<uint8_t> data;
    CanIdType idType = CanIdType::STANDARD;
    bool isRtr = false;
    uint32_t timestamp = 0;
    uint32_t fifo = 0;

    CanMessage() = default;
    CanMessage(uint32_t msg_id, const std::vector<uint8_t> &msg_data, CanIdType type = CanIdType::STANDARD)
        : id(msg_id), data(msg_data), idType(type), timestamp(HAL_GetTick()) {}
};

// コールバック関数型
using CanMessageHandler = std::function<void(const CanMessage &message)>;
using CanErrorHandler = std::function<void(uint32_t errorCode)>;

class CanController {
public:
    // シングルトンパターン
    static CanController &getInstance();
    static void initialize(CAN_HandleTypeDef *hcan);

    // 送信関数
    static bool send(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType = CanIdType::STANDARD);
    static bool sendAsync(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType = CanIdType::STANDARD);
    static bool sendRtr(uint32_t id, uint8_t length, CanIdType idType = CanIdType::STANDARD);

    // メッセージハンドラ登録
    static void onReceive(uint32_t id, CanMessageHandler handler);
    static void onReceiveAny(CanMessageHandler handler); // 全メッセージ
    static void onError(CanErrorHandler handler);

    // ステータス取得
    static uint32_t getTxCount() { return getInstance().txCounter_; }
    static uint32_t getRxCount() { return getInstance().rxCounter_; }
    static uint32_t getErrorCount() { return getInstance().errorCounter_; }
    static bool isReady() { return getInstance().isInitialized_; }

    // 内部使用（割り込みハンドラから呼び出し）
    void handleRxInterrupt(uint32_t fifo);
    void handleTxInterrupt(uint32_t mailbox);
    void handleErrorInterrupt();
    void processTxQueue();

private:
    CanController() = default;
    ~CanController() = default;
    CanController(const CanController &) = delete;
    CanController &operator=(const CanController &) = delete;

    void initInternal(CAN_HandleTypeDef *hcan);
    void enableInterrupts();
    bool sendInternal(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType, bool isRtr = false);
    void processRxFifo(uint32_t fifo);
    void processReceivedMessage(const CanMessage &message);

    CAN_HandleTypeDef *hcan_ = nullptr;
    bool isInitialized_ = false;

    // メッセージハンドラ
    std::map<uint32_t, CanMessageHandler> messageHandlers_; // ID別ハンドラ
    CanMessageHandler globalHandler_;                       // 全メッセージハンドラ
    CanErrorHandler errorHandler_;

    // 送信キュー
    std::queue<CanMessage> txQueue_;

    // 統計情報
    uint32_t txCounter_ = 0;
    uint32_t rxCounter_ = 0;
    uint32_t errorCounter_ = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

// HALコールバック関数
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);

#ifdef __cplusplus
}
#endif

#endif