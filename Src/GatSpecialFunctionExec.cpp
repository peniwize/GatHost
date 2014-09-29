/*!
    \file "GatSpecialFunctionExec.cpp"

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


#include "GatSpecialFunctionExec.hpp"
#include "GatPkt_StatusQueryRslt_SR81.hpp"


//#define ENABLE_GAT_SPEC_FXN_DEBUG_TIMING


#ifdef ENABLE_GAT_SPEC_FXN_DEBUG_TIMING
    const int statusPollPeriodInMilliseconds = 5000;
    const int maxStatusPollDurationInMilliseconds = 5u/* min */ * 60u/* sec */ * 1000u/* ms */;
#else
    const int statusPollPeriodInMilliseconds = 1000;
    const int maxStatusPollDurationInMilliseconds = 20u/* min */ * 60u/* sec */ * 1000u/* ms */;
#endif


auto
GatSpecialFunctionExec::setState(StateId value) -> StateId
{
    if (currentState_ != value)
    {
        currentState_ = value;
        emit specialFunctionExecStateChanged(this, state());
    }

    return currentState_;
}


bool
GatSpecialFunctionExec::sendRequest(QStringList const &params)
{
    // Validate preconditions.
    if (nullptr == linkLayer()) { return false; }

/*
    bool const linkLayerIsBusy = CGatLinkLayer::EState::Ready != linkLayer()->state();
    if (linkLayerIsBusy) { return false; }
*/

    bool const thisIsBusy = StateId::Ready != state();
    if (thisIsBusy) { return false; }

    // Formulate authentication request.
    QString authParam;
    for (QStringList::const_iterator iter = params.begin(); params.end() != iter; ++iter)
    {
        if (params.begin() != iter) { authParam += "\t"; }
        authParam += *iter;
    }
    QByteArray asciiAuthParam("\xba");
    asciiAuthParam.append('\x00');
    asciiAuthParam.append(authParam.toLatin1());

    // Fail when the authentication request is too large.
    if (GAT_MAX_PYLD_SIZE < static_cast<size_t>(asciiAuthParam.size())) { return false; }

    clearResults();

    // Send authentication request.
    auto const sendRqstRslt = linkLayer()->sendRequest(GatRqst::IACQ_04, asciiAuthParam.constData(), asciiAuthParam.size());
    if (GatLinkLayer::RequestResult::Success != sendRqstRslt &&
        GatLinkLayer::RequestResult::Pending != sendRqstRslt)
    {
        return false;
    }

    setState(StateId::Requesting);

    return true;
}


void
GatSpecialFunctionExec::onLinkLayerStateChangedDuringRequest(GatLinkLayer::StateId newState)
{
    switch (newState)
    {
        // Request failed.
        case GatLinkLayer::StateId::Timeout:
        {
            setState(StateId::RequestFailed_Timeout);
            setState(StateId::Ready);

            break;
        }

        // Request complete.
        case GatLinkLayer::StateId::Reply:
        {
            // If request succeeded:
            if (GatLinkLayer::ResultType::Reply == linkLayer()->resultType())
            {
                // Start polling for completion status.
                startPollingForReplyReady();
            }
            else // Received invalid response, timeout, ...
            {
                setState(GatLinkLayer::ResultType::Timeout == linkLayer()->resultType()
                         ? StateId::RequestFailed_Timeout : StateId::RequestFailed);
                setState(StateId::Ready);
            }

            break;
        }

        default: break; // Prevent compiler warning.
    }
}


void
GatSpecialFunctionExec::onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId newState)
{
    bool const bHostIsLinkLayer = host == linkLayer();
    if (!bHostIsLinkLayer) { return; }

    switch (state())
    {
        // When sending auth request:
        case StateId::Requesting:
        {
            onLinkLayerStateChangedDuringRequest(newState);
            break;
        }

        // When polling for auth results availability:
        case StateId::WaitingForReply:
        {
            onLinkLayerStateChangedWhileWaitingForReply(newState);
            break;
        }

        default: break; // Prevent compiler warning.
    }
}


void
GatSpecialFunctionExec::startPollingForReplyReady()
{
    // Remember when polling for reply started (to determine when to stop).
    statusPollStartTime_ = monotonicClock32();

    setState(StateId::WaitingForReply);

    pollForReplyReady();
}


void
GatSpecialFunctionExec::pollForReplyReady()
{
    if (StateId::WaitingForReply == state())
    {
        // Stop status polling when max poll time has elapsed.
        if (monotonicClock32() - statusPollStartTime_ >= maxStatusPollDuration_)
        {
            setState(StateId::ReplyUnavailable_Timeout);
            setState(StateId::Ready);
        }
        else
        {
            GatLinkLayer::StateId linkLayerState = linkLayer()->state();
            bool const statusPollInPgrs = GatLinkLayer::StateId::WaitForNextTxTime == linkLayerState ||
                                          GatLinkLayer::StateId::Transmit == linkLayerState ||
                                          GatLinkLayer::StateId::Receive == linkLayerState;
            if (!statusPollInPgrs)
            {
                // Stop poll timer, if it's running.
                timer_.stop();

                // Send status query poll (SQ 0x01).
                auto const sendRqstRslt = linkLayer()->sendRequest(GatRqst::SQ_01, nullptr, 0);
                if (GatLinkLayer::RequestResult::Success == sendRqstRslt ||
                    GatLinkLayer::RequestResult::Pending == sendRqstRslt)
                {
                    // Start poll timer.
                    timer_.setInterval(statusPollPeriod_); // Time until timeout.
                    timer_.setSingleShot(true);
                    timer_.start();
                }
                else
                {
                    // Fail.
                    setState(StateId::ReplyFailed);
                    setState(StateId::Ready);
                }
            }
            else
            {
                // Do nothing.
                // (Poll currently in progress.
                //  Send another when response comes back and auth results are not ready yet.
                //  This happens in 'ProcessReplyReadyPollResponse()'.)
            }
        }
    }
}


void
GatSpecialFunctionExec::onLinkLayerStateChangedWhileWaitingForReply(GatLinkLayer::StateId newState)
{
    switch (newState)
    {
        // Request failed.
        case GatLinkLayer::StateId::Timeout:
        {
            // Stop poll timer, if it's running.
            timer_.stop();

            setState(StateId::ReplyUnavailable_Timeout);
            setState(StateId::Ready);

            break;
        }

        // Request complete.
        case GatLinkLayer::StateId::Reply:
        {
            // If request succeeded:
            if (GatLinkLayer::ResultType::Reply == linkLayer()->resultType())
            {
                // Start polling for completion status.
                processReplyReadyPollResponse();
            }
            else // Received invalid response, timeout, ...
            {
                // Stop poll timer, if it's running.
                timer_.stop();

                setState(StateId::ReplyFailed);
                setState(StateId::Ready);
            }

            break;
        }

        default: break; // Prevent compiler warning.
    }
}


void
GatSpecialFunctionExec::processReplyReadyPollResponse()
{
    bool failed = true;

    GatLinkLayer::ResultType const resultType = linkLayer()->resultType();
    if (GatLinkLayer::ResultType::Reply == resultType)
    {
        GatLinkLayer::reply_type reply(linkLayer()->reply());
        uint8_t const *replyBytes = reinterpret_cast<uint8_t const *>(reply.first);
        uint const replySize = reply.second;
        GatPkt_StatusQueryRslt_SR81 statusQueryResult;
        if (statusQueryResult.parseResultPacket(replyBytes, replySize))
        {
            bool const calculationInPgrs = statusQueryResult.calculationInProgress();
            bool const authResultsReady = statusQueryResult.authResultsReady();
            GatPkt_StatusQueryRslt_SR81::CalculationStatus const calculationStatus = statusQueryResult.calculationStatus();

            // If calculating (pending or in progress):
            if (calculationInPgrs ||
                GatPkt_StatusQueryRslt_SR81::CalculationStatus::Calculating == calculationStatus ||
                GatPkt_StatusQueryRslt_SR81::CalculationStatus::Requested == calculationStatus)
            {
                // If the status poll timer expired while waiting for a response then 'pollForReplyReady()'
                // will not have restarted it.  'pollForReplyReady()' assumes that the timer will be restarted
                // [here] when the poll response arrives.  This helps reduce the load on the GM by reducing
                // the number of [unnecessary] status polls sent to it.
                if (!timer_.isActive())
                {
                    // Start poll timer.
                    timer_.setInterval(statusPollPeriod_); // Time until timeout.
                    timer_.setSingleShot(true);
                    timer_.start();
                }
                Q_ASSERT(timer_.isActive());
                // Note: The timer will cause the next status query poll to be sent.
                failed = false;
            }
            // If results are ready:
            else if (authResultsReady &&
                     GatPkt_StatusQueryRslt_SR81::CalculationStatus::Finished == calculationStatus)
            {
                // Calculation complete; begin receiving result(s).
                if (gatMultipktReply_.sendRequest(GatDataFormat::Xml))
                {
                    setState(StateId::ReceivingReply);
                    failed = false;
                }
            }
        }
    }

    if (failed)
    {
        // Stop poll timer, if it's running.
        timer_.stop();

        setState(StateId::ReplyFailed);
        setState(StateId::Ready);
    }
}


void
GatSpecialFunctionExec::onMultipktRplyStateChanged(GatMultipktRply *host, GatMultipktRply::StateId newState)
{
    bool const hostIsMultipktReply = host == &gatMultipktReply_;
    if (!hostIsMultipktReply) { return; }

    switch (state())
    {
        // When sending auth request:
        case StateId::ReceivingReply:
        {
            onMultipktRplyStateChangedWhileReceivingReply(newState);
            break;
        }

        default: break; // Prevent compiler warning.
    }
}


void
GatSpecialFunctionExec::onMultipktRplyStateChangedWhileReceivingReply(GatMultipktRply::StateId newState)
{
    switch (newState)
    {
        // Request failed.
        case GatMultipktRply::StateId::Timeout:
        case GatMultipktRply::StateId::InvalidResponse:
        {
            setState(StateId::ReplyFailed);
            setState(StateId::Ready);

            break;
        }

        // Request complete.
        case GatMultipktRply::StateId::Reply:
        {
            // If request succeeded:
            if (GatMultipktRply::ResultType::Reply == gatMultipktReply_.resultType())
            {
                // If result is good data (non-error):
                GatMultipktRply::ResultType const resultType = gatMultipktReply_.resultType();
                if (GatMultipktRply::ResultType::Reply == resultType)
                {
                    // Record results.
                    GatMultipktRply::reply_type reply(gatMultipktReply_.reply());
                    uint8_t const *rplyBytes = reinterpret_cast<uint8_t const *>(reply.first);
                    uint const replySize = reply.second;
                    std::copy(rplyBytes, rplyBytes + replySize, std::back_inserter(results_));

                    // Move to next state.
                    setState(StateId::ReplyReady);
                    setState(StateId::Ready);
                }
                else
                {
                    setState(StateId::ReplyFailed);
                    setState(StateId::Ready);
                }
            }
            else // Received invalid response, timeout, ...
            {
                setState(StateId::ReplyFailed);
                setState(StateId::Ready);
            }

            break;
        }

        default: break; // Prevent compiler warning.
    }
}


void
GatSpecialFunctionExec::onTimer()
{
    timer_.stop(); // Not sure, but this may be necessary to ensure timer is not active.

    pollForReplyReady();
}


void
GatSpecialFunctionExec::clearResults()
{
    setDataFormat(GatDataFormat::Undefined);
    setResultType(ResultTypeId::Undefined);
    results_.clear();
}


GatLinkLayer *
GatSpecialFunctionExec::setLinkLayer(GatLinkLayer *value)
{
    if (linkLayer_ != value)
    {
        // Unsubscribe from signals from old link layer.
        if (nullptr != linkLayer_)
        {
            disconnect(linkLayer_, SIGNAL(stateChanged(GatLinkLayer *, GatLinkLayer::StateId)),
                       this, SLOT(onLinkLayerStateChanged(GatLinkLayer *, GatLinkLayer::StateId)));
            linkLayer_ = nullptr;
            setState(StateId::Undefined);
            gatMultipktReply_.setLinkLayer(nullptr);
        }

        // Link to new link layer (and unlink from old).
        linkLayer_ = value;

        // Subscribe to signals from new link layer.
        if (nullptr != linkLayer_)
        {
            connect(linkLayer_, SIGNAL(stateChanged(GatLinkLayer *, GatLinkLayer::StateId)),
                    this, SLOT(onLinkLayerStateChanged(GatLinkLayer *, GatLinkLayer::StateId)));
            setState(StateId::Ready);
        }

        gatMultipktReply_.setLinkLayer(linkLayer_);
    }

    return linkLayer_;
}


GatSpecialFunctionExec::GatSpecialFunctionExec(GatLinkLayer *linkLayer, QObject *parent)
    : QObject(parent)
    , currentState_(StateId::Undefined)
    , statusPollPeriod_(statusPollPeriodInMilliseconds)
    , maxStatusPollDuration_(maxStatusPollDurationInMilliseconds)
    , statusPollStartTime_(0)
    , gatDataFormat_(GatDataFormat::Undefined)
    , resultTypeId_(ResultTypeId::Undefined)
    , linkLayer_(nullptr)
{
    gatMultipktReply_.setParent(this);
    timer_.setParent(this);
    setLinkLayer(linkLayer);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    connect(&gatMultipktReply_, SIGNAL(stateChanged(GatMultipktRply *, GatMultipktRply::StateId)),
            this, SLOT(onMultipktRplyStateChanged(GatMultipktRply *, GatMultipktRply::StateId)));
}


GatSpecialFunctionExec::~GatSpecialFunctionExec()
{
    disconnect(&gatMultipktReply_, SIGNAL(stateChanged(GatMultipktRply *, GatMultipktRply::StateId)),
               this, SLOT(onMultipktRplyStateChanged(GatMultipktRply *, GatMultipktRply::StateId)));
    disconnect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    setLinkLayer(nullptr);
}


/*
    End of "GatSpecialFunctionExec.cpp"
*/
