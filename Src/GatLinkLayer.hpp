/*!
    \file "GatLinkLayer.hpp"

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


#ifndef GATLINKLAYER_HPP__8F7E2472_D853_42C9_8131_D51D78B82002__INCLUDED
#define GATLINKLAYER_HPP__8F7E2472_D853_42C9_8131_D51D78B82002__INCLUDED


#pragma once


#include "Defs.hpp"
#include <QObject>
#include <QTimer>


class GatLinkLayer;


struct GatLinkLayerStrategyInterface
{
    virtual uint write(GatLinkLayer &host, void const *data, uint dataSizeInBytes) = 0;
    virtual void onLinkToHost(GatLinkLayer *host) = 0;
};


/*!
    \brief GAT link layer; transmission and reception of one request and its corresponding reply.
*/
class GatLinkLayer
    : public QObject
{
    Q_OBJECT

    //! \name States (of this state machine)
    //! @{
public:
    enum class StateId : size_t {
        Ready,             //!< Idle/ready.
        WaitForNextTxTime, //!< Waiting for next transmit opportunity (in progress)
        Transmit,          //!< Transmitting (in progress)
        Receive,           //!< Receiving (in progress)
        Timeout,           //!< No response received from GM (timeout while waiting).
        Reply,             //!< Response received from GM (and available).
        Undefined          // This must be last.
    };
    size_t const stateId_Count = static_cast<size_t>(StateId::Undefined) + 1;

    StateId state() const { return state_; }

protected:
    StateId setState(StateId value);

signals:
    void stateChanged(GatLinkLayer *host, GatLinkLayer::StateId state);

private:
    StateId state_;
    //! @}

    //! \name Request
    //! @{
public:
    enum class RequestResult : size_t {
        Success,
        Pending,
        UndefinedFailure,
        InvalidDataSize // This must be last.
    };
    size_t const requestResult_Count = static_cast<size_t>(RequestResult::InvalidDataSize) + 1;

    bool requestPending() const { return 0 < requestDataSize_; }

signals:
    void onTxPacket(GatLinkLayer *host, QByteArray packetData);

public slots:
    RequestResult sendRequest(GatRqst gatRequest, void const *data, uint dataSize);

protected slots:
    virtual void onTimer();
    virtual void onRxdTimer();

protected:
    RequestResult transmitPendingRequest();

private:
    uint8_t requestData_[GAT_MAX_PACKET_SIZE];
    uint requestDataSize_; // Number of items in 'mvRequestData' (size of content).
    uint8_t lastRequestCmd_; // "Cmd" field of last request sent.
    QTimer timer_;
    //! @}

    //! \name Receive Data (RxD)
    //! @{
public:
    void receiveData(void const *data, size_t dataSizeInBytes); //!< Client calls whenever data arrives (at any time).

protected:
    void receiveTimeout(); //!< Handle timeout [when] in receive state.
    void receiveDataTimeout();

signals:
    void onReceiveData(GatLinkLayer *host, QByteArray rawData);
    void onReceivePacket(GatLinkLayer *host, QByteArray packetData, bool invalidPacket);

private:
    QTimer receiveDataTimer_;
    //! @}

    //! \name Results (reply)
    //! @{
public:
    typedef ::std::pair<void const *, uint> reply_type; // Data pointer + number of bytes addressed.
    reply_type reply() { return reply_type(replyData_, replyDataSize_); }

    enum class ResultType : size_t {
        Reply,
        InvalidResponse,
        Timeout,
        Undefined // This must be last.
    };
    size_t const resultType_Count = static_cast<size_t>(ResultType::Undefined) + 1;

    ResultType resultType() { return resultType_; }

protected:
    ResultType setResultType(ResultType value) { return (resultType_ = value); }

private:
    uint8_t replyData_[GAT_MAX_PACKET_SIZE];
    uint replyDataSize_; // Number of items in 'replyData_' (size of content).
    ResultType resultType_;
    //! @}

    //! \name Strategy
    //! @{
public:
    GatLinkLayerStrategyInterface * setStrategy(GatLinkLayerStrategyInterface *newStrategy);
    GatLinkLayerStrategyInterface * strategy() const { return strategy_; }

private:
    GatLinkLayerStrategyInterface *strategy_;
    //! @}

    //! \name Construction, Destruciton, and Assignment
    //! @{
public:
    GatLinkLayer(QObject *parent = nullptr);
    virtual ~GatLinkLayer();

private:
    GatLinkLayer(GatLinkLayer const&) = delete; //!< No cloning; leave unimplemented!
    GatLinkLayer& operator=(GatLinkLayer const&) = delete; //!< No cloning; leave unimplemented!
    //! @}
};


#endif // #ifndef GATLINKLAYER_HPP__8F7E2472_D853_42C9_8131_D51D78B82002__INCLUDED


/*
    End of "GatLinkLayer.hpp"
*/
