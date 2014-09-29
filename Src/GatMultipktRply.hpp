/*!
    \file "GatMultipktRply.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef GATMULTIPKTRPLY_HPP__9FBA66E5_3C5E_4846_AFB9_A1AE86C113C7__INCLUDED
#define GATMULTIPKTRPLY_HPP__9FBA66E5_3C5E_4846_AFB9_A1AE86C113C7__INCLUDED


#pragma once


#include "Defs.hpp"
#include "GatLinkLayer.hpp"
#include <QObject>
#include <QTimer>


class GatMultipktRply
    : public QObject
{
    Q_OBJECT

    //! \name State (of this state machine)
    //! @{
public:
    enum class StateId : size_t
    {
        Ready,           //!< Idle/ready.
        Transmit,        //!< Transmitting (in progress), i.e. sending request to GM.
        Receive,         //!< Receiving (in progress)
        Timeout,         //!< No response received from GM (timeout while waiting).
        InvalidResponse, //!< Invalid response received from GM.
        Reply,           //!< Response received from GM (and available).
        Undefined        // This must be last.
    };
    size_t const stateId_Count = static_cast<size_t>(StateId::Undefined) + 1;

    StateId state() const { return stateId_; }
    StateId setState(StateId value);

signals:
    void stateChanged(GatMultipktRply *host, GatMultipktRply::StateId newState);

private:
    StateId stateId_;
    //! @}

    //! \name Request
    //! @{
public slots:
    bool sendRequest(GatDataFormat dataFormat);

protected:
    virtual bool transmitLarq();
    virtual void onTimeout();
    virtual void onInvalidResponse();
    //! @}

    //! \name Results
    //! @{
public:
    typedef ::std::pair<void const *, uint> reply_type; // Data pointer + number of bytes addressed.
    reply_type reply() { return reply_type(&results_[0], results_.size()); }

    enum class ResultType : size_t {
        Reply,
        InvalidResponse,
        Timeout,
        Undefined // This must be last.
    };
    size_t const resultType_Count = static_cast<size_t>(ResultType::Undefined) + 1;

    ResultType resultType() { return resultType_; }
    uint frameNumber() const { return frameNumber_; }
    GatDataFormat dataFormat() const { return dataFormat_; }

protected:
    ResultType setResultType(ResultType value) { return (resultType_ = value); }
    virtual uint setFrameNumber(uint value) { return (frameNumber_ = value); }
    virtual GatDataFormat setDataFormat(GatDataFormat value) { return (dataFormat_ = value); }
    virtual void clearResults();

private:
    GatDataFormat dataFormat_;
    uint frameNumber_;
    typedef std::vector<uint8_t> results_type;
    results_type results_;
    ResultType resultType_;
    //! @}

    //! \name GAT Link Layer Association
    //! @{
public:
    GatLinkLayer * setLinkLayer(GatLinkLayer *linkLayer);
    GatLinkLayer * linkLayer() const { return linkLayer_; }

protected slots:
    void onGatLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId newState);

private:
    void subscribeToLinkLayer();
    void unsubscribeFromLinkLayer();

private:
    GatLinkLayer *linkLayer_;
    //! @}

    //! \name Construction, Destruction, and Assignment
    //! @{
public:
    GatMultipktRply(GatLinkLayer *linkLayer = nullptr, QObject *parent = nullptr);
    virtual ~GatMultipktRply();

private:
    GatMultipktRply(GatMultipktRply const&) = delete; //!< No cloning; leave unimplemented!
    GatMultipktRply& operator=(GatMultipktRply const&) = delete; //!< No cloning; leave unimplemented!
    //! @}
};


#endif // #ifndef GATMULTIPKTRPLY_HPP__9FBA66E5_3C5E_4846_AFB9_A1AE86C113C7__INCLUDED


/*
    End of "GatMultipktRply.hpp"
*/
