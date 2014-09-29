/*!
    \file "GatSpecialFunctionExec.hpp"

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


#ifndef GATSPECIALFUNCTIONEXEC_HPP__9BC29110_DE0D_4976_B597_7CD130CED923__INCLUDED
#define GATSPECIALFUNCTIONEXEC_HPP__9BC29110_DE0D_4976_B597_7CD130CED923__INCLUDED


#pragma once


#include "Defs.hpp"
#include "GatMultipktRply.hpp"
#include <QTimer>


/*!
    \brief GAT special function execution state machine.

    This state machine executes a GAT special function and retrieves its result(s).

    1) Formulate authentication parameter for 0x04, i.e. the request.
    2) Send request to GM via linkLayer()->sendRequest(EGatRqst::IACQ_04 ...) and link layer events;
    3) Switch to polling state and wait for GM to report that data is ready (or timeout).
    4) Receive data via gatMultipktRply_.sendRequest(EGatDataFormat::Xml) and its events.
*/
class GatSpecialFunctionExec
    : public QObject
{
    Q_OBJECT

    //! \name State (of this state machine)
    //! @{
public:
    enum class StateId : size_t
    {
        Ready,                    //!< Ready to send operation request (idle).
        Requesting,               //!< Sending operation request to GM.
        RequestFailed,            //!< Error while requesting (bad reply, etc)
        RequestFailed_Timeout,    //!< Error while requesting (timeout)
        WaitingForReply,          //!< Polling GM for operation results status until results are ready.
        ReplyUnavailable_Timeout, //!< Operation results not made available in timely fashion.
        ReceivingReply,           //!< Receiving operation results from GM.
        ReplyFailed,              //!< Error while receiving operation results (timeout, bad reply, etc).
        ReplyReady,               //!< Reply received from GM and ready for client.
        Undefined                 // This _must_ be last.
    };
    size_t const stateId_Count = static_cast<size_t>(StateId::Undefined) + 1;

    StateId state() const { return currentState_; }

signals:
    void specialFunctionExecStateChanged(GatSpecialFunctionExec *host, GatSpecialFunctionExec::StateId newState);

protected:
    StateId setState(StateId value);

private:
    StateId currentState_;
    //! @}

    //! \name Request
    //! @{
public slots:
    bool sendRequest(QStringList const &params);

protected slots:
    virtual void onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId newState);
    virtual void onMultipktRplyStateChanged(GatMultipktRply *host, GatMultipktRply::StateId newState);
    virtual void onTimer();

protected:
    virtual void onLinkLayerStateChangedDuringRequest(GatLinkLayer::StateId newState);
    virtual void onLinkLayerStateChangedWhileWaitingForReply(GatLinkLayer::StateId newState);
    virtual void onMultipktRplyStateChangedWhileReceivingReply(GatMultipktRply::StateId newState);

private:
    void startPollingForReplyReady();
    void pollForReplyReady();
    void processReplyReadyPollResponse();

private:
    GatMultipktRply gatMultipktReply_;
    QTimer timer_;
    uint statusPollPeriod_;        // Milliseconds between status polls (1/frequency).
    uint maxStatusPollDuration_;   // Milliseconds; maximum duration to perform status polling.
    uint32_t statusPollStartTime_; // GetMonotonicClock32() when polling starts.
    //! @}

    //! \name Results
    //! @{
public:
    typedef ::std::pair<void const *, uint> reply_type; // Data pointer + number of bytes addressed.
    reply_type reply() { return reply_type(&results_[0], results_.size()); }

    enum class ResultTypeId : size_t
    {
        Reply,
        InvalidResponse,
        Timeout,
        Undefined // This must be last.
    };
    size_t const resultTypeId_Count = static_cast<size_t>(ResultTypeId::Undefined) + 1;

    ResultTypeId resultType() { return resultTypeId_; }
    GatDataFormat dataFormat() const { return gatDataFormat_; }

protected:
    ResultTypeId setResultType(ResultTypeId value) { return (resultTypeId_ = value); }
    virtual GatDataFormat setDataFormat(GatDataFormat value) { return (gatDataFormat_ = value); }
    virtual void clearResults();

private:
    GatDataFormat gatDataFormat_;
    typedef std::vector<uint8_t> results_type;
    results_type results_;
    ResultTypeId resultTypeId_;
    //! @}

    //! \name GAT Link Layer Association
    //! @{
public:
    GatLinkLayer * setLinkLayer(GatLinkLayer *value);
    GatLinkLayer * linkLayer() const { return linkLayer_; }

private:
    GatLinkLayer *linkLayer_;
    //! @}

    //! \name Construction, Destruciton, and Assignment
    //! @{
public:
    GatSpecialFunctionExec(GatLinkLayer *linkLayer = nullptr, QObject *parent = nullptr);
    virtual ~GatSpecialFunctionExec();

private:
    GatSpecialFunctionExec(GatSpecialFunctionExec const&) = delete; //!< No cloning; leave unimplemented!
    GatSpecialFunctionExec& operator=(GatSpecialFunctionExec const&) = delete; //!< No cloning; leave unimplemented!
    //! @}
};


#endif // #ifndef GATSPECIALFUNCTIONEXEC_HPP__9BC29110_DE0D_4976_B597_7CD130CED923__INCLUDED


/*
    End of "GatSpecialFunctionExec.hpp"
*/
