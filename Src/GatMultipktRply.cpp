/*!
    \file "GatMultipktRply.cpp"

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


#include "GatMultipktRply.hpp"


auto
GatMultipktRply::setState(StateId value) -> StateId
{
    if (stateId_ != value)
    {
        stateId_ = value;
        emit stateChanged(this, stateId_);
    }

    return stateId_;
}


bool
GatMultipktRply::sendRequest(GatDataFormat dataFormat)
{
    // Validate preconditions.
    if (nullptr == linkLayer()) { return false; }

/*
    bool const linkLayerIsBusy = CGatLinkLayer::EState::Ready != linkLayer()->state();
    if (linkLayerIsBusy) { return false; }
*/

    bool const thisIsBusy = StateId::Ready != state();
    if (thisIsBusy) { return false; }

    // Start reading data from GM.
    clearResults();
    setDataFormat(dataFormat);
    setFrameNumber(1);
    subscribeToLinkLayer();
    bool const bSuccess = transmitLarq();

    if (!bSuccess)
    {
        unsubscribeFromLinkLayer();
        clearResults();
    }

    return bSuccess;
}


bool
GatMultipktRply::transmitLarq()
{
    uint8_t larqPayload[] = {
        static_cast<uint8_t>(gatDataFormatIdToCode(dataFormat())),
        static_cast<uint8_t>((frameNumber() >> 8) & 0xff),
        static_cast<uint8_t>((frameNumber() >> 0) & 0xff),
    };

    auto const sendRequestResult = linkLayer()->sendRequest(GatRqst::LARQ_03, larqPayload, sizeof(larqPayload));
    if (GatLinkLayer::RequestResult::Success != sendRequestResult &&
        GatLinkLayer::RequestResult::Pending != sendRequestResult)
    {
        return false;
    }

    setState(StateId::Transmit);

    return true;
}


void
GatMultipktRply::onTimeout()
{
    unsubscribeFromLinkLayer();
    clearResults();
    setResultType(ResultType::Timeout);
    setState(StateId::Timeout);
    setState(StateId::Ready);
}


void
GatMultipktRply::onInvalidResponse()
{
    unsubscribeFromLinkLayer();
    clearResults();
    setResultType(ResultType::InvalidResponse);
    setState(StateId::InvalidResponse);
    setState(StateId::Ready);
}


void
GatMultipktRply::clearResults()
{
    setDataFormat(GatDataFormat::Undefined);
    setFrameNumber(0);
    setResultType(ResultType::Undefined);
    results_.clear();
}


GatLinkLayer *
GatMultipktRply::setLinkLayer(GatLinkLayer *linkLayer)
{
    if (linkLayer_ != linkLayer)
    {
        // Stop observing old link layer.
        if (nullptr != linkLayer_)
        {
            unsubscribeFromLinkLayer();
            linkLayer_ = nullptr;
            setState(StateId::Undefined);
        }

        // Link to new link layer (and unlink from old).
        linkLayer_ = linkLayer;

        // Start observing new link layer.
        if (nullptr != linkLayer_)
        {
            setState(StateId::Ready);
        }
    }

    return linkLayer_;
}


void
GatMultipktRply::subscribeToLinkLayer()
{
    connect(linkLayer_, SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
            this, SLOT(onGatLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));
}


void
GatMultipktRply::unsubscribeFromLinkLayer()
{
    disconnect(linkLayer_, SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
               this, SLOT(onGatLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));
}


void
GatMultipktRply::onGatLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId newState)
{
    bool const eventIsFromMyHost = linkLayer() == host;
    bool const operationIsInProgress = StateId::Ready != state() && StateId::Undefined != state();
    if (eventIsFromMyHost && operationIsInProgress)
    {
        switch (newState)
        {
            case GatLinkLayer::StateId::Transmit:
            {
                setState(StateId::Transmit);
                break;
            }

            case GatLinkLayer::StateId::Receive:
            {
                setState(StateId::Receive);
                break;
            }

            case GatLinkLayer::StateId::Timeout:
            {
                onTimeout();
                break;
            }

            case GatLinkLayer::StateId::Reply:
            {
                switch (linkLayer()->resultType())
                {
                    case GatLinkLayer::ResultType::Reply:
                    {
                        // Get reply data from link layer.
                        GatLinkLayer::reply_type const reply(linkLayer()->reply());
                        void const *replyData = reply.first;
                        uint const replySize = reply.second;
                        if (nullptr == replyData ||
                            7 > replySize ||
                            GatLinkLayer::ResultType::Reply != linkLayer()->resultType())
                        {
                            onInvalidResponse();
                            break;
                        }

                        // Parse status and frame number.
                        uint8_t const *replyBytes = reinterpret_cast<uint8_t const *>(replyData);
                        bool const gatError = 0 != (replyBytes[2] & 0x1);
                        bool const lastFrame = 0 != (replyBytes[2] & 0x2);
                        uint const packetFrameNumber = (static_cast<uint>(replyBytes[3] & 0xff) << 8)
                                                       | static_cast<uint>(replyBytes[4] & 0xff);
                        if (frameNumber() != packetFrameNumber ||
                            gatError)
                        {
                            onInvalidResponse();
                            break;
                        }

                        // Append frame data to accumulated.
                        std::copy(replyBytes + 5, replyBytes + 5 + replySize - 7, std::back_inserter(results_));

                        if (lastFrame)
                        {
                            unsubscribeFromLinkLayer();
                            setResultType(ResultType::Reply);
                            setState(StateId::Reply);
                            setState(StateId::Ready);
                        }
                        else
                        {
                            // Move to next frame.
                            setFrameNumber(frameNumber() + 1);
                            transmitLarq();
                        }

                        break;
                    }

                    case GatLinkLayer::ResultType::InvalidResponse:
                    default:
                    {
                        onInvalidResponse();
                        break;
                    }

                    case GatLinkLayer::ResultType::Timeout:
                    {
                        onTimeout();
                        break;
                    }
                }

                break;
            }

            default: break; // Do nothing; prevent compiler warning.
        }
    }
}


GatMultipktRply::GatMultipktRply(GatLinkLayer *linkLayer, QObject *parent)
    : QObject(parent)
    , stateId_(StateId::Undefined)
    , dataFormat_(GatDataFormat::Undefined)
    , frameNumber_(0)
    , resultType_(ResultType::Undefined)
    , linkLayer_(nullptr)
{
    setLinkLayer(linkLayer);
}


GatMultipktRply::~GatMultipktRply()
{
    setLinkLayer(nullptr);
}


/*
    End of "GatMultipktRply.cpp"
*/
