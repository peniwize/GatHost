/*!
    \file "GatPkt_StatusQueryRslt_SR81.cpp"

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


#include "Defs.hpp"
#include "GatPkt_StatusQueryRslt_SR81.hpp"


bool
GatPkt_StatusQueryRslt_SR81::parseResultPacket(uint8_t const *replyBytes, size_t const replySize)
{
    if (8 <= replySize &&
        (gatRqstToCode(GatRqst::SQ_01) | 0x80) == replyBytes[0] &&
        8 == replyBytes[1])
    {
        uint const versionInBcd = (static_cast<uint16_t>(replyBytes[2]) << 8) | static_cast<uint16_t>(replyBytes[3]);
        if (0x350 <= versionInBcd)
        {
            versionInBcd_ = versionInBcd;
            calculationInPgrs_ = 0x1 == (replyBytes[4] & 0x1);
            authResultsReady_ = 0x2 == (replyBytes[4] & 0x2);
            calculationStatus_ = static_cast<CalculationStatus>((replyBytes[4] >> 2) & 0x3);
            dataFormats_.clear();
            if (0x01 == (replyBytes[5] & 0x01)) { dataFormats_.push_back(GatDataFormat::PlainText); }
            if (0x02 == (replyBytes[5] & 0x02)) { dataFormats_.push_back(GatDataFormat::Xml); }

            return true;
        }
    }

    return false;
}


GatPkt_StatusQueryRslt_SR81::GatPkt_StatusQueryRslt_SR81()
    : calculationInPgrs_(false)
    , authResultsReady_(false)
    , calculationStatus_(CalculationStatus::Requested)
    , versionInBcd_(0)
{
    // Do nothing.
}


GatPkt_StatusQueryRslt_SR81::GatPkt_StatusQueryRslt_SR81(uint8_t const *pRplyBytes, size_t const replySize)
    : GatPkt_StatusQueryRslt_SR81()
{
    parseResultPacket(pRplyBytes, replySize);
}


/*
    End of "GatPkt_StatusQueryRslt_SR81.cpp"
*/
