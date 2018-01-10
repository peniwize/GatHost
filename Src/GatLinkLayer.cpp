/*!
    \file "GatLinkLayer.cpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    GAT communication link layer.

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#include "GatLinkLayer.hpp"
#include <QMetaType>


#define ENABLE_GAT_LINK_LAYER_DEBUG_TIMING


#ifdef ENABLE_GAT_LINK_LAYER_DEBUG_TIMING
    const int millisecondsUntilReceiveTimeout = 50;
    const int millisecondsUntilRplyTimeout = 500;
    const int millisecondsUntilNextTransmitAllowedAfterReceive = 10;
    const int millisecondsUntilNextTransmitAllowedAfterTimeout = 25;
#else
    const int millisecondsUntilReceiveTimeout = 50;
    const int millisecondsUntilRplyTimeout = 201;
    const int millisecondsUntilNextTransmitAllowedAfterReceive = 10;
    const int millisecondsUntilNextTransmitAllowedAfterTimeout = 25;
#endif


auto
GatLinkLayer::setState(StateId value) -> StateId
{
    if (state_ != value)
    {
        state_ = value;
#ifdef DEBUG
        static char const *stateNames[] = {
            "Ready", // EState::Ready
            "WaitForNextTxTime", // EState::WaitForNextTxTime
            "Transmit", // EState::Transmit
            "Receive", // EState::Receive
            "Timeout", // EState::Timeout
            "Reply", // EState::Reply
            "Undefined", // EState::Undefined
        };
        Q_ASSERT(arycap(stateNames) == stateId_Count);
        size_t const valueAsInt = static_cast<size_t>(value);
        qDebug() << "CGatLinkLayer::SetState(EState "
                 << (arycap(stateNames) > valueAsInt ? stateNames[valueAsInt] : QString().arg(valueAsInt))
                 << ")";
#endif // #ifdef DEBUG
        emit stateChanged(this, state_);
    }

    if (StateId::Ready == state_)
    {
        transmitPendingRequest();
    }

    return state_;
}


auto
GatLinkLayer::sendRequest(GatRqst gatRequest, void const *data, uint dataSize) -> RequestResult
{
    if (nullptr == data && 0 != dataSize) { return RequestResult::InvalidDataSize; }
    if (GAT_MAX_PYLD_SIZE < dataSize) { return RequestResult::InvalidDataSize; }

    // Create the packet to send.
    requestDataSize_ = 0;
    requestData_[0] = static_cast<uint8_t>(gatRqstToCode(gatRequest)); // Command byte.
    requestData_[1] = dataSize + 4/* command + length + CRC */; // Length byte.
    if (nullptr != data && 0 < dataSize)
    {
        memcpy(requestData_ + 2, data, (std::min)(static_cast<uint>(GAT_MAX_PYLD_SIZE), dataSize)); // Payload bytes.
    }
    size_t const crcIdx = dataSize + 2/* command + length */;
    uint16_t const crc = calcGatCrc16(requestData_, crcIdx);
    requestData_[crcIdx + 0] = static_cast<uint8_t>((crc >> 8) & 0xff); // CRC word.
    requestData_[crcIdx + 1] = static_cast<uint8_t>((crc >> 0) & 0xff);
    requestDataSize_ = requestData_[1];

    return transmitPendingRequest();
}


auto
GatLinkLayer::transmitPendingRequest() -> RequestResult
{
    RequestResult result = RequestResult::Pending;

    if (0 < requestDataSize_)
    {
        // Join on state transition and next transmit time.
        bool const stateAllowsTransmit = StateId::Ready == state();
        bool const timeAllowsTransmit = !timer_.isActive();
        if (stateAllowsTransmit && timeAllowsTransmit)
        {
            // Discard old/previous results (if any).
            replyDataSize_ = 0;
            memset(replyData_, 0, sizeof(replyData_));
            setResultType(ResultType::Undefined);

            setState(StateId::Transmit);

            bool failed = true;
            if (nullptr != strategy())
            {
                uint const numberOfBytesWritten = strategy()->write(*this, requestData_, requestDataSize_);
                Q_ASSERT(sizeof(char) == sizeof(requestData_[0]));
                emit onTxPacket(this, QByteArray(reinterpret_cast<char const *>(requestData_), numberOfBytesWritten));
                failed = numberOfBytesWritten != requestDataSize_;
            }

            // Clear pending request data.
            lastRequestCmd_ = requestData_[0];
            memset(requestData_, 0, sizeof(requestData_));
            requestDataSize_ = 0;

            if (failed)
            {
                result = RequestResult::UndefinedFailure;
                setState(StateId::Ready);
            }
            else
            {
                result = RequestResult::Success;
                setState(StateId::Receive);
                timer_.setInterval(millisecondsUntilRplyTimeout); // Time until timeout.
                timer_.setSingleShot(true);
                timer_.start();
            }
        }
    }

    return result;
}


void
GatLinkLayer::onTimer()
{
    timer_.stop(); // Not sure, but this may be necessary to ensure timer is not active.

    transmitPendingRequest();
    receiveTimeout();
}


void
GatLinkLayer::onRxdTimer()
{
    receiveDataTimer_.stop();

    receiveDataTimeout();
}


void
GatLinkLayer::receiveData(void const *data, size_t dataSizeInBytes)
{
    if (nullptr == data || 0 == dataSizeInBytes) { return; }

    // [Re]start end-of-packet inter-character period measurement timer.
    receiveDataTimer_.setInterval(millisecondsUntilReceiveTimeout); // Time until timeout.
    receiveDataTimer_.setSingleShot(true);
    receiveDataTimer_.start();

    Q_ASSERT(sizeof(char) == sizeof(uint8_t));
    emit onReceiveData(this, QByteArray(static_cast<char const *>(data), dataSizeInBytes));

    bool const responseAlreadyPending = ResultType::Undefined != resultType();
    if (responseAlreadyPending) { return; }

    // Append new data to buffer.
    Q_ASSERT(arycap(replyData_) > replyDataSize_);
    uint const byteCountToConsume = (std::min)(dataSizeInBytes, arycap(replyData_) - replyDataSize_);
    memcpy(replyData_ + replyDataSize_, data, byteCountToConsume);
    replyDataSize_ += byteCountToConsume;

    // Analyze buffer contents to detect when a complete packet arrives.
    bool invalidResponse = false;
    if (4 <= replyDataSize_)
    {
        uint8_t const expectedResponseCmd = lastRequestCmd_ | 0x80;
        if (expectedResponseCmd != replyData_[0])
        {
            invalidResponse = true;
        }
        else
        {
            size_t const length = replyData_[1];
            if (length <= replyDataSize_)
            {
                uint16_t const packetCrc = static_cast<uint16_t>((static_cast<uint16_t>(replyData_[length - 2]) << 8) |
                                                                 (static_cast<uint16_t>(replyData_[length - 1]) << 0));
                uint16_t const calculatedCrc = calcGatCrc16(replyData_, length - 2/* CRC */);
                if (packetCrc == calculatedCrc)
                {
                    replyDataSize_ = length;
                    setResultType(ResultType::Reply);

                    setState(StateId::Reply);
                    setState(StateId::Ready);

                    // Set timer to signal when next transmission can be sent.
                    timer_.setInterval(millisecondsUntilNextTransmitAllowedAfterReceive);
                    timer_.setSingleShot(true);
                    timer_.start();
                }
                else
                {
                    invalidResponse = true;
                }

                receiveDataTimer_.stop(); // Stop watching for end of transmission.

                Q_ASSERT(sizeof(char) == sizeof(replyData_[0]));
                emit onReceivePacket(this, QByteArray(reinterpret_cast<char const *>(replyData_), replyDataSize_), invalidResponse);
            }
        }
    }
    if (invalidResponse)
    {
        // Invalid response received.
        std::fill_n(replyData_, arycap(replyData_), 0);
        replyDataSize_ = 0;
        setResultType(ResultType::InvalidResponse);

        setState(StateId::Reply);
        setState(StateId::Ready);
    }
}


void
GatLinkLayer::receiveTimeout()
{
    if (StateId::Receive == state() &&
        !timer_.isActive())
    {
        std::fill_n(replyData_, arycap(replyData_), 0);
        replyDataSize_ = 0;
        setResultType(ResultType::Timeout);

        setState(StateId::Timeout);
        setState(StateId::Ready);

        // Set timer to signal when next transmission can be sent.
        timer_.setInterval(millisecondsUntilNextTransmitAllowedAfterTimeout);
        timer_.setSingleShot(true);
        timer_.start();
    }
}


void
GatLinkLayer::receiveDataTimeout()
{
    // Invalid response received.

    receiveDataTimer_.stop(); // Stop watching for end of transmission.

    bool const responseAlreadyPending = ResultType::Undefined != resultType();
    if (responseAlreadyPending || 0 == replyDataSize_) { return; }

    Q_ASSERT(sizeof(char) == sizeof(replyData_[0]));
    emit onReceivePacket(this, QByteArray(reinterpret_cast<char const *>(replyData_), replyDataSize_), true);

    std::fill_n(replyData_, arycap(replyData_), 0);
    replyDataSize_ = 0;
    setResultType(ResultType::InvalidResponse);

    setState(StateId::Reply);
    setState(StateId::Ready);
}


GatLinkLayerStrategyInterface *
GatLinkLayer::setStrategy(GatLinkLayerStrategyInterface *newStrategy)
{
    // Unlinked from current strategy (if any).
    while (nullptr != strategy_)
    {
        GatLinkLayerStrategyInterface *oldStrategy = strategy_;
        strategy_ = nullptr;
        oldStrategy->onLinkToHost(nullptr);
        setState(StateId::Undefined);
    }

    // Link to new strategy.
    strategy_ = newStrategy;
    if (nullptr != strategy_)
    {
        strategy_->onLinkToHost(this);
        setState(StateId::Ready);
    }

    return strategy_;
}


GatLinkLayer::GatLinkLayer(QObject *parent)
    : QObject(parent)
    , state_(StateId::Undefined)
    , requestDataSize_(0)
    , lastRequestCmd_(0)
    , replyDataSize_(0)
    , resultType_(ResultType::Undefined)
    , strategy_(nullptr)
{
    //qRegisterMetaType<CGatLinkLayer>("CGatLinkLayer");

    memset(requestData_, 0, sizeof(requestData_));
    memset(replyData_, 0, sizeof(replyData_));
    timer_.setParent(this);
    receiveDataTimer_.setParent(this);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    connect(&receiveDataTimer_, SIGNAL(timeout()), this, SLOT(onRxdTimer()));
}


GatLinkLayer::~GatLinkLayer()
{
    // Ensure this is unlinked from its strategy.
    while (nullptr != strategy_)
    {
        GatLinkLayerStrategyInterface *oldStrategy = strategy_;
        strategy_ = nullptr;
        oldStrategy->onLinkToHost(nullptr);
    }

    disconnect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    disconnect(&receiveDataTimer_, SIGNAL(timeout()), this, SLOT(onRxdTimer()));
}


/*
    End of "GatLinkLayer.cpp"
*/
