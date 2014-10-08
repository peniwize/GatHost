/*!
    \file "MainWindow.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Application main window (form).

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef MAINWINDOW_HPP__147B1EEC_5181_4D4F_8CBF_D837EEBD8786__INCLUDED
#define MAINWINDOW_HPP__147B1EEC_5181_4D4F_8CBF_D837EEBD8786__INCLUDED


#include "Defs.hpp"
#include "GatHost.hpp"
#include <QMainWindow>
#include <QTableWidgetItem>


namespace Ui {
    class MainWindow;
}


class MainWindow
    : public QMainWindow
{
    Q_OBJECT

public:
    void setResult_StatusQuery(GatHostCmd *command, QByteArray const &result);
    void setResult_LastAuthStatusQuery(GatHostCmd *command, QByteArray const &result);
    void setResult_GetSpecialFunctions(GatHostCmd *command, QByteArray const &result);
    void setResult_GetComponent(GatHostCmd *command, QByteArray const &result);
    void setResult_GetFile(GatHostCmd *command, const QByteArray &result);
    void operationFailed(GatHostCmd *command, QString const &specialFuncionName, QString const &description,
                         QString const &operationResultDescription);

    template <typename tBoundFxn> void scheduleDpc(tBoundFxn functor)
    {
        std::unique_ptr<QEvent> event(createNewDpc(functor));
        postEvent(event.get());
        event.release();
    }

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    enum class LocalEventType : int
    {
        Dpc = QEvent::User
    };

    class Dpc : public QEvent
    {
    public:
        Dpc() : QEvent(static_cast<QEvent::Type>(LocalEventType::Dpc)) {}
        virtual void operator()() {}
    };

    // tBoundFxn <- std::bind(...)
    // Use std::ref or std::cref with bind() arguments to pass by reference.
    template <typename tBoundFxn> class GenericDpc : public Dpc
    {
    public:
        GenericDpc(tBoundFxn functor) : boundFxn_(functor) {}
        virtual void operator()() { boundFxn_(); }
    private:
        tBoundFxn boundFxn_;
    };

    template <typename tBoundFxn> Dpc * createNewDpc(tBoundFxn functor)
    { return new MainWindow::GenericDpc<tBoundFxn>(functor); }

    void postEvent(QEvent *event);
    void customEvent(QEvent *event);

    void selectSerialDevice();
    void openCloseGatDevice();
    void syncUiWidgets();
    void populateOperationResults(QString const &value);
    void showOperationInProgressUiWidgets(bool show);

    void keyPressEvent(QKeyEvent *event);

private:
    void setResult_StatusQuery_Dpc(GatHostCmd *command, QByteArray const &result);
    void setResult_LastAuthStatusQuery_Dpc(GatHostCmd *command, QByteArray const &result);
    void setResult_GetSpecialFunctions_Dpc(GatHostCmd *command, QByteArray const &result);
    void setResult_GetComponent_Dpc(GatHostCmd *command, QByteArray const &result);
    void setResult_GetFile_Dpc(GatHostCmd *command, QByteArray const &result);
    void operationFailed_Dpc(GatHostCmd *command, QString const &specialFunctionName, QString const &description,
                             QString const &operationResultDescription);

    void initSpecialFunctionsView(QList<QStringList> const &functionDefinitions, uint columns);
    void populateSpecialFunctionsView(QList<QStringList> const &functionDefinitions);

    bool scheduleGatCommand(GatHost::gat_host_cmd_ptr_type newCommand);

    void writeToLog(QString value, bool writeToIoLog = true);

    void onGetComponentOrFile();

    MainWindow(MainWindow const&) = delete; //!< No cloning; leave unimplemented!
    MainWindow& operator=(MainWindow const&) = delete; //!< No cloning; leave unimplemented!

private slots:
    void onGatHostShutdown();
    void onGatHostStartupState(GatHost *host, GatHostStartupStateId startupState, QString const &description);
    void onGatHostRunStateChange();

    void onTxPacket(GatLinkLayer *host, QByteArray pktData);
    void onRxD(GatLinkLayer *host, QByteArray rawData);
    void onRxPacket(GatLinkLayer *host, QByteArray pktData, bool invalidPkt);

    void on_actionOpenSerialDevice_triggered();
    void on_actionExitApplication_triggered();
    void on_actionAboutApplication_triggered();
    void on_openCloseButton_clicked();
    void on_getSpecialFunctionsButton_clicked();
    void on_showSerialDevicesButton_clicked();
    void on_actionShowSerialDevices_triggered();
    void on_actionGetSpecialFunctions_triggered();
    void on_actionStatusQuery_triggered();
    void on_actionLastAuthenticationStatusQuery_triggered();
    void on_actionGetComponent_triggered();
    void on_actionGetFile_triggered();
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_cellDoubleClicked(int aiRow, int aiColumn);
    void on_btnIoLogClear_clicked();
    void on_tabWidget_currentChanged(int aiTabIndex);

private:
    Ui::MainWindow *ui;
    GatHost gatHost_;
    GatHost::gat_host_cmd_ptr_type activeGatCmd_;
    uint ioLogMsgSerialNumber_;
    int edtIoLogLineHeight_;
    int edtLogTextLineHeight_;

    friend class StatusQueryCmd;
    friend class GetSpecialFunctionsCmd;
};


// This is being placed in the header only to simplify interaction with moc,
// which expects declarations in headers and not cpp files.
class StatusQueryCmd
    : public GatHostCmd
{
    Q_OBJECT

public:
    virtual QString gatSpecialFunctionName() const;
    StatusQueryCmd(MainWindow &mainWindow, GatHost &host);

protected:
    virtual void onCmdStateChanged();

private:
    MainWindow &mainWindow_;
};


// This is being placed in the header only to simplify interaction with moc,
// which expects declarations in headers and not cpp files.
class LastAuthStatusQueryCmd
    : public GatHostCmd
{
    Q_OBJECT

public:
    virtual QString gatSpecialFunctionName() const;
    LastAuthStatusQueryCmd(MainWindow &mainWindow, GatHost &host);

protected:
    virtual void onCmdStateChanged();

private:
    MainWindow &mainWindow_;
};


// This is being placed in the header only to simplify interaction with moc,
// which expects declarations in headers and not cpp files.
class GetSpecialFunctionsCmd
    : public GatHostGetSpecialFunctionsCmd
{
    Q_OBJECT

public:
    GetSpecialFunctionsCmd(MainWindow &mainWindow, GatHost &host);

protected:
    virtual void onCmdStateChanged();

private:
    MainWindow &mainWindow_;
};


// This is being placed in the header only to simplify interaction with moc,
// which expects declarations in headers and not cpp files.
class getComponentCmd
    : public GatHostGetComponentCmd
{
    Q_OBJECT

public:
    getComponentCmd(MainWindow &mainWindow, GatHost &host, QString const &componentName, QStringList const *params);

protected:
    virtual void onCmdStateChanged();

private:
    MainWindow &mainWindow_;
};


// This is being placed in the header only to simplify interaction with moc,
// which expects declarations in headers and not cpp files.
class GetFileCmd
    : public GatHostGetFileCmd
{
    Q_OBJECT

public:
    GetFileCmd(MainWindow &mainWindow, GatHost &host, QString const &componentName, QStringList const *params);

protected:
    virtual void onCmdStateChanged();

private:
    MainWindow &mainWindow_;
};


#endif // #ifndef MAINWINDOW_HPP__147B1EEC_5181_4D4F_8CBF_D837EEBD8786__INCLUDED


/*
    End of "MainWindow.hpp"
*/
