#include "CanController.h"

CanController &CanController::getInstance() {
    static CanController instance;
    return instance;
}

void CanController::initialize(CAN_HandleTypeDef *hcan) { getInstance().initInternal(hcan); }

void CanController::initInternal(CAN_HandleTypeDef *hcan) {
    hcan_ = hcan;

    // CANフィルター設定（全メッセージ受信）
    CAN_FilterTypeDef canfilterconfig;
    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 0;
    canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilterconfig.FilterIdHigh = 0x0000;
    canfilterconfig.FilterIdLow = 0x0000;
    canfilterconfig.FilterMaskIdHigh = 0x0000;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(hcan_, &canfilterconfig);
    HAL_CAN_Start(hcan_);
    enableInterrupts();

    isInitialized_ = true;
}

bool CanController::send(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType) {
    return getInstance().sendInternal(id, data, idType, false);
}

bool CanController::sendAsync(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType) {
    auto &instance = getInstance();
    if (!instance.isInitialized_)
        return false;

    CanMessage msg(id, data, idType);
    instance.txQueue_.push(msg);
    instance.processTxQueue();

    return true;
}

bool CanController::sendRtr(uint32_t id, uint8_t length, CanIdType idType) {
    std::vector<uint8_t> data(length, 0);
    return getInstance().sendInternal(id, data, idType, true);
}

// メッセージハンドラ登録
void CanController::onReceive(uint32_t id, CanMessageHandler handler) { getInstance().messageHandlers_[id] = handler; }

void CanController::onReceiveAny(CanMessageHandler handler) { getInstance().globalHandler_ = handler; }

void CanController::onError(CanErrorHandler handler) { getInstance().errorHandler_ = handler; }

// 内部送信実装（同期送信）
bool CanController::sendInternal(uint32_t id, const std::vector<uint8_t> &data, CanIdType idType, bool isRtr) {
    if (!isInitialized_ || data.size() > 8)
        return false;

    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;

    if (idType == CanIdType::STANDARD) {
        if (id > 0x7FF)
            return false;
        tx_header.StdId = id;
        tx_header.ExtId = 0;
        tx_header.IDE = CAN_ID_STD;
    } else {
        if (id > 0x1FFFFFFF)
            return false;
        tx_header.StdId = 0;
        tx_header.ExtId = id;
        tx_header.IDE = CAN_ID_EXT;
    }

    tx_header.RTR = isRtr ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    tx_header.DLC = data.size();
    tx_header.TransmitGlobalTime = DISABLE;

    uint8_t dataArray[8] = {0};
    for (size_t i = 0; i < data.size(); ++i) {
        dataArray[i] = data[i];
    }

    if (HAL_CAN_AddTxMessage(hcan_, &tx_header, dataArray, &tx_mailbox) == HAL_OK) {
        txCounter_++;
        return true;
    }

    return false;
}

// 割り込み処理
void CanController::handleRxInterrupt(uint32_t fifo) { processRxFifo(fifo); }

void CanController::handleTxInterrupt(uint32_t mailbox) {
    processTxQueue(); // 次の送信があれば処理
}

void CanController::handleErrorInterrupt() {
    uint32_t errorCode = HAL_CAN_GetError(hcan_);
    errorCounter_++;

    if (errorHandler_) {
        errorHandler_(errorCode);
    }
}

void CanController::processTxQueue() {
    if (txQueue_.empty())
        return;

    // 空いている送信メールボックスがあるかチェック
    uint32_t freeMailboxes = 0;
    if ((hcan_->Instance->TSR & CAN_TSR_TME0) != 0)
        freeMailboxes++;
    if ((hcan_->Instance->TSR & CAN_TSR_TME1) != 0)
        freeMailboxes++;
    if ((hcan_->Instance->TSR & CAN_TSR_TME2) != 0)
        freeMailboxes++;

    if (freeMailboxes > 0) {
        CanMessage msg = txQueue_.front();
        txQueue_.pop();
        sendInternal(msg.id, msg.data, msg.idType, msg.isRtr);
    }
}

void CanController::enableInterrupts() {
    HAL_CAN_ActivateNotification(hcan_, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_ActivateNotification(hcan_, CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_ActivateNotification(hcan_, CAN_IT_TX_MAILBOX_EMPTY);
    HAL_CAN_ActivateNotification(hcan_, CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF);
}

void CanController::processRxFifo(uint32_t fifo) {
    CAN_RxHeaderTypeDef rx_header;
    uint8_t dataArray[8];

    while (HAL_CAN_GetRxFifoFillLevel(hcan_, fifo) > 0) {
        if (HAL_CAN_GetRxMessage(hcan_, fifo, &rx_header, dataArray) == HAL_OK) {
            CanMessage msg;
            msg.fifo = fifo;
            msg.timestamp = HAL_GetTick();
            msg.isRtr = (rx_header.RTR == CAN_RTR_REMOTE);

            if (rx_header.IDE == CAN_ID_STD) {
                msg.id = rx_header.StdId;
                msg.idType = CanIdType::STANDARD;
            } else {
                msg.id = rx_header.ExtId;
                msg.idType = CanIdType::EXTENDED;
            }

            msg.data.clear();
            msg.data.reserve(rx_header.DLC);
            for (uint8_t i = 0; i < rx_header.DLC; ++i) {
                msg.data.push_back(dataArray[i]);
            }

            rxCounter_++;
            processReceivedMessage(msg);
        }
    }
}

void CanController::processReceivedMessage(const CanMessage &message) {
    // ID別ハンドラをチェック
    auto it = messageHandlers_.find(message.id);
    if (it != messageHandlers_.end() && it->second) {
        it->second(message);
    }

    // グローバルハンドラを呼び出し
    if (globalHandler_) {
        globalHandler_(message);
    }
}

// HAL割り込みコールバック
extern "C" {

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleRxInterrupt(CAN_RX_FIFO0);
    }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleRxInterrupt(CAN_RX_FIFO1);
    }
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleTxInterrupt(0);
    }
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleTxInterrupt(1);
    }
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleTxInterrupt(2);
    }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    if (CanController::isReady()) {
        CanController::getInstance().handleErrorInterrupt();
    }
}

} // extern "C"