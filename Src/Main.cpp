/*!
    \file "Main.cpp"

    Copyright (c) 2012 Matt Ervin / imp software
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)

    Program entry point.
*/


#include <QApplication>
#include "Defs.hpp"
#include "MainWindow.hpp"
#include "GatHost.hpp"
#include "GatLinkLayer.hpp"
#include "Version.hpp"


char const *productBuildString = SVN_VERSION;
char const *applicationName = "GatHost";


//#define GENERATE_GAT_PACKET_FOR_TESTING


#ifdef GENERATE_GAT_PACKET_FOR_TESTING


#include <fstream>


// This generates a Status Response (0x81 SR) packet for use in development testing.
static void
GeneratePacket_SR81()
{
    uint8_t const iCS = 0x01; // 0x01 = calculating
    uint8_t const iLAR = 0x01; // 0x01 = results available
    uint8_t const iCC = 0x03; // 0x00 = requested, 0x01 = finished, 0x02 = calculating, 0x03 = error.
    uint8_t const iDF = 0x03; // 0x01 = plain text, 0x02 = xml
    uint8_t vBuf[] = { 0x81, 0x08, 0x03, 0x50, iCS | iLAR | (iCC << 2), iDF, 0x00, 0x00 };
    uint16_t const iCrc = CalcGatCrc16(vBuf, AryDim(vBuf) - 2);
    vBuf[AryDim(vBuf) - 2] = scast<uint8_t>((iCrc >> 8) & 0xff);
    vBuf[AryDim(vBuf) - 1] = scast<uint8_t>((iCrc >> 0) & 0xff);
    std::ostringstream oss;
    oss << "Packet: ";
    SimpleHexDump(oss, vBuf, AryDim(vBuf));
    qDebug() << oss.str().c_str();
    std::ofstream ofs("packet.bin", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    Q_ASSERT(sizeof(uint8_t) == sizeof(char));
    ofs.write(rcast<char const *>(vBuf), AryDim(vBuf));
    ofs.close();
}


#endif // #ifdef GENERATE_GAT_PACKET_FOR_TESTING


int
main(int argc, char *argv[])
{
#ifdef GENERATE_GAT_PACKET_FOR_TESTING
    GeneratePacket_SR81();
#endif // #ifdef GENERATE_GAT_PACKET_FOR_TESTING

    QApplication application(argc, argv);
    qRegisterMetaType<GatHostStartupStateId>("GatHostStartupStateId");
    qRegisterMetaType<GatLinkLayer::StateId>("GatLinkLayer::StateId");
    qRegisterMetaType<GatHostCmd::CmdState>("GatHostCmd::CmdState");
    MainWindow mainWnd;
    mainWnd.show();
    return application.exec();
}


/*
    End of "Main.cpp"
*/
