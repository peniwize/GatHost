/*!
    \file "GatPkt_StatusQueryRslt_SR81.hpp"

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


#ifndef GATPKT_STATUSQUERYRSLT_SR81_HPP__B28F6E0E_3DB3_4C4E_9556_04EDFD8B74C7__INCLUDED
#define GATPKT_STATUSQUERYRSLT_SR81_HPP__B28F6E0E_3DB3_4C4E_9556_04EDFD8B74C7__INCLUDED


#pragma once


class GatPkt_StatusQueryRslt_SR81
{
public:
    enum class CalculationStatus : size_t
    {
        Requested,
        Finished,
        Calculating,
        Error
    };
    size_t const calculationStatus_Count = static_cast<size_t>(CalculationStatus::Error) + 1;

    typedef std::vector<GatDataFormat> data_formats_type;

    bool calculationInProgress() const { return calculationInPgrs_; }
    bool authResultsReady() const { return authResultsReady_; }
    CalculationStatus calculationStatus() const { return calculationStatus_; }
    uint versionInBcd() const { return versionInBcd_; }
    data_formats_type const& dataFormats() const { return dataFormats_; }

    bool parseResultPacket(uint8_t const *replyBytes, size_t const replySize);

    GatPkt_StatusQueryRslt_SR81();
    GatPkt_StatusQueryRslt_SR81(uint8_t const *replyBytes, size_t const replySize);

private:
    bool calculationInPgrs_;
    bool authResultsReady_;
    CalculationStatus calculationStatus_;
    uint versionInBcd_;
    data_formats_type dataFormats_;
};


#endif // #ifndef GATPKT_STATUSQUERYRSLT_SR81__HPP_B28F6E0E_3DB3_4C4E_9556_04EDFD8B74C7__INCLUDED


/*
    End of "GatPkt_StatusQueryRslt_SR81.hpp"
*/
