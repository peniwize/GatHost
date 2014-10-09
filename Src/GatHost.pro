#-------------------------------------------------
#
# Project created by QtCreator 2012-12-06T14:39:39
#
#-------------------------------------------------

QT       += core gui xml

lessThan(QT_MAJOR_VERSION, 5): CONFIG += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = GatHost
TEMPLATE = app

#####version.target = Version.hpp
#####version.commands = ../genVersion.sh
#version.depends =

#####QMAKE_EXTRA_TARGETS += version
#####PRE_TARGETDEPS += Version.hpp
QMAKE_CXXFLAGS += -std=c++0x

CONFIG(release, debug|release) {
    #message(Release)
}

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -DDEBUG
    #message(Debug)
}

SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    GatHost.cpp \
    AboutBox.cpp \
    GatLinkLayer.cpp \
    Defs.cpp \
    SelectSerialPortDlg.cpp \
    GatMultipktRply.cpp \
    GatSpecialFunctionExec.cpp \
    GatPkt_StatusQueryRslt_SR81.cpp \
    GatCmdSpec.cpp

HEADERS  += \
    MainWindow.hpp \
    GatHost.hpp \
    AboutBox.hpp \
    Defs.hpp \
    GatLinkLayer.hpp \
    SelectSerialPortDlg.hpp \
    GatMultipktRply.hpp \
    GatSpecialFunctionExec.hpp \
    GatPkt_StatusQueryRslt_SR81.hpp \
    GatCmdSpec.hpp

FORMS    += \
    MainWindow.ui \
    AboutBox.ui \
    SelectSerialPortDlg.ui \
    GatCmdSpec.ui

OTHER_FILES += \
    ../Notes.txt \
    ../README \
    ../LICENSE

RESOURCES += \
    GatHost.qrc
