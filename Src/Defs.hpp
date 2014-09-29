/*!
    \file "Defs.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Application environment (and globals).

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef DEFS_HPP__7BEC1CCD_CF9D_44D8_8AAD_52605E924E2B__INCLUDED
#define DEFS_HPP__7BEC1CCD_CF9D_44D8_8AAD_52605E924E2B__INCLUDED


#include <iosfwd>
#include <iomanip>
#include <string>
#include <sstream>
#include <memory>
#include <deque>
#include <algorithm>
#include <QtContainerFwd>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QMessageBox>


uint const productMavorVersion = 1;
uint const productMinorVersion = 0;
extern char const *productBuildString;
extern char const *applicationName;

#define STRINGIZE(x) #x

typedef uint8_t sha1Hmac_type[20]; // 20 bytes.

template <typename tItem, size_t tCapacity> inline size_t
arycap(tItem (&)[tCapacity])
{
    return tCapacity;
}

inline uint32_t
monotonicClock32()
{
    timespec timeSpec = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &timeSpec);
    return static_cast<uint32_t>(timeSpec.tv_sec) * static_cast<uint32_t>(1000) +
           static_cast<uint32_t>(timeSpec.tv_nsec) / static_cast<uint32_t>(1000000ul);
}

template <typename tStream> tStream &
simpleHexDump(tStream &stream, void const *data, size_t dataSize)
{
    uint8_t const *bytes = reinterpret_cast<uint8_t const *>(data);
    for (size_t idx = 0; dataSize > idx; ++idx)
    {
        if (0 < idx) { stream << " "; }
        stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint>(bytes[idx]);
    }
    return stream;
}

inline ::std::string
toStdStr(QString value)
{
    QByteArray plainText = value.toLatin1();
    return ::std::string(plainText.constData());
}

inline QMessageBox::StandardButton
QInfoMsgBox(QString const &text, QWidget *parent = nullptr)
{
    return QMessageBox::information(parent, applicationName, text);
}

inline QMessageBox::StandardButton
QErrMsgBox(QString const &text, QWidget *parent = nullptr)
{
    return QMessageBox::critical(parent, applicationName, text);
}

size_t const GAT_MAX_PACKET_SIZE = 255u;
size_t const GAT_MAX_PYLD_SIZE = GAT_MAX_PACKET_SIZE - 4/* command + length + CRC */;

struct GatRqstCmd
{
    static uint const   SQ = 0x01;
    static uint const LASQ = 0x02;
    static uint const LARQ = 0x03;
    static uint const IACQ = 0x04;
};


enum class GatRqst : size_t
{
    Undefined,
    SQ_01,
    LASQ_02,
    LARQ_03,
    IACQ_04
};
static const size_t gatRqst_Count = static_cast<size_t>(GatRqst::IACQ_04) + 1;

uint gatRqstToCode(GatRqst value);
GatRqst gatRqstFromCode(uint value);


enum class GatCalcStatus : size_t
{
    Pending,    // Calculation requested, but not yet started.
    InProgress, // Requested calculation currently in progress.
    Complete,   // Requested calculation complete  (results ready).
    Error       // Requested calculation cannot be performed or has failed.
};
const size_t gatCalcStatus_Count = static_cast<size_t>(GatCalcStatus::Error) + 1;


enum class GatDataFormat : size_t
{
    Undefined,
    PlainText,
    Xml
};
const size_t gatDataFormat_Count = static_cast<size_t>(GatDataFormat::Xml) + 1;


GatDataFormat gatDataFormatCodeToId(uint code);
uint gatDataFormatIdToCode(GatDataFormat gatDataFormat);

uint16_t calcGatCrc16(void const *data, size_t dataSize, uint16_t seed = 0xffff);

uint const statusMessageDuration = 5000; // Milliseconds.

char const * errnoToStr(int errorNumber);


#endif // #ifndef DEFS_HPP__7BEC1CCD_CF9D_44D8_8AAD_52605E924E2B__INCLUDED


/*
    End of "Defs.hpp"
*/
