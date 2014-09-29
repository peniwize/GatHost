/*!
    \file "GatHost.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    GAT protocol host (as opposed to GM).

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef GATHOST_HPP__BDE65372_4E6A_46CB_8155_B0A2A7D5BE33__INCLUDED
#define GATHOST_HPP__BDE65372_4E6A_46CB_8155_B0A2A7D5BE33__INCLUDED


#include "Defs.hpp"
#include "GatLinkLayer.hpp"
#include "GatSpecialFunctionExec.hpp"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QEvent>
#include <QTimer>


class GatHost;


struct GatHostPrivilegesForGatHostCmdInterface
{
    virtual GatLinkLayer& gatLinkLayer() = 0;

    friend class GatHostCmd;
};


class GatHostCmd
    : public QObject
{
    Q_OBJECT

    //! \name Command Events (state machine events)
    //! @{
public:
    virtual void begin();
    virtual void cancel();
    virtual void fail();
    virtual void processGmResponse(uint8_t const *data, size_t dataSize);
    //! @}

    //! \name Operation Result(s)
    //! @{
public:
    QByteArray lastGatOpResult() const; //!< Returns by value for thread safety.

protected:
    QByteArray const& setLastGatOpResult(QByteArray const &value) { return (operationResult_ = value); }

private:
    QByteArray operationResult_;
    //! @}

    //! \name Miscellaneous
    //! @{
public:
    virtual QString gatSpecialFunctionName() const; //!< Returns by value for thread safety.

protected:
    GatLinkLayer::RequestResult sendRequest(GatRqst gatRequest) { return sendRequest(gatRequest, nullptr, 0); }
    GatLinkLayer::RequestResult sendRequest(GatRqst gatRequest, void const *data, uint dataSize);
    void scheduleFailureDpc();

    enum class ELocalEventType : int
    {
        FailThisDpc = QEvent::User
    };

    void customEvent(QEvent *event);
    //! @}

    //! \name Command State Tracking
    //! @{
public:
    enum class CmdState {
        Started,
        Canceled,
        Failed_Timeout,
        Failed,
        Completed,
        Undefined // Must always be last.
    };
    static size_t const cmdState_Count = static_cast<size_t>(CmdState::Undefined) + 1;

signals:
    void gatHostCmdStateChanged(GatHostCmd *cmd, GatHostCmd::CmdState cmdState); //!< Signals execute in thread context of creator.

protected:
    virtual CmdState setCmdState(CmdState newCmdState);
    virtual CmdState cmdState() const { return cmdState_; } // Atomic operation.
    virtual void onCmdStateChanged();

private:
    CmdState cmdState_;
    //! @}

    //! \name Link Layer Management
    //! @{
protected slots:
    virtual void onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId state);
    //! @}

    //! \name Host Association
    //! @{
public:
    GatHost & host() { return host_; } // Atomic operation.
    GatHost const & host() const { return host_; } // Atomic operation.

    GatHostPrivilegesForGatHostCmdInterface& hostPrivileges(); // Atomic operation.
    GatHostPrivilegesForGatHostCmdInterface const& hostPrivileges() const; // Atomic operation.

private:
    GatHost &host_;
    //! @}

    //! \name Synchronization Domain
    //! @{
public:
    QMutex * syncDomainGuard() const; // Atomic operation.
    //! @}

    //! \name Construction, Destruction, and Assignment
    //! @{
public:
    GatHostCmd(GatHost &gatHost);
    virtual ~GatHostCmd();
    //! @}
};


class GatHostSpecialFxnCmd
    : public GatHostCmd
{
    Q_OBJECT

public:
    virtual void begin();
    GatSpecialFunctionExec& gatSpecialFunctionExec() { return specialFxnExec_; }

    virtual QString gatSpecialFunctionName() const; //!< Returns by value for thread safety.

    GatHostSpecialFxnCmd(GatHost &host);
    virtual ~GatHostSpecialFxnCmd();

protected slots:
    virtual void onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId state);
    virtual void onSpecialFunctionExecStateChanged(GatSpecialFunctionExec *host,
                                                   GatSpecialFunctionExec::StateId newState);

protected:
    virtual void gatSpecFxnParams(QStringList & /*arDest*/) {}

private:
    GatSpecialFunctionExec specialFxnExec_;
};


class GatHostGetSpecialFunctionsCmd
    : public GatHostSpecialFxnCmd
{
    Q_OBJECT

public:
    virtual QString gatSpecialFunctionName() const; //!< Returns by value for thread safety.

    GatHostGetSpecialFunctionsCmd(GatHost &host) : GatHostSpecialFxnCmd(host) {}

protected:
    virtual void gatSpecFxnParams(QStringList &result);
};


class GatHostGetFileCmd
    : public GatHostSpecialFxnCmd
{
    Q_OBJECT

public:
    virtual QString gatSpecialFunctionName() const; //!< Returns by value for thread safety.

    QString fileName() const { return QString().append(fileName_); } //!< Returns by value for thread safety.
    QStringList params() const; //!< Returns by value for thread safety.

    enum class FileName : size_t {
        AuthResponse,
        ProgId,
        None // Must _always_ be last.
    };
    static size_t const fileName_Count = static_cast<size_t>(FileName::None) + 1;

    static QString fileName(FileName fileName); //!< Standard GAT file names (for convenience).

    GatHostGetFileCmd(GatHost &host, QString const &fileName, QStringList const *params)
        : GatHostSpecialFxnCmd(host), fileName_(fileName), params_(nullptr == params ? QStringList() : *params) {}

protected:
    virtual void gatSpecFxnParams(QStringList &result);

private:
    QString fileName_;
    QStringList params_;
};


class GatHostGetComponentCmd
    : public GatHostSpecialFxnCmd
{
    Q_OBJECT

public:
    virtual QString gatSpecialFunctionName() const; //!< Returns by value for thread safety.

    QString componentName() const { return QString().append(componentName_); } //!< Returns by value for thread safety.
    QStringList params() const; //!< Returns by value for thread safety.

    GatHostGetComponentCmd(GatHost &host, QString const &componentName, QStringList const *params)
        : GatHostSpecialFxnCmd(host)
        , componentName_(componentName)
        , params_(nullptr == params ? QStringList() : *params) {}

protected:
    virtual void gatSpecFxnParams(QStringList &result);

private:
    QString componentName_;
    QStringList params_;
};


enum class GatHostStartupStateId : size_t
{
    Success,
    Fail // Must always be last.
};
size_t const gatHostStartupState_Count = static_cast<size_t>(GatHostStartupStateId::Fail) + 1;


/*!
    \brief The GatHost class

    See 'http://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/'
    for details about using QThread.

    Sending a QEvent to another thread:
    http://www.qtcentre.org/threads/40985-How-to-send-custom-events-from-QThread-run-()-to-application

    GAT v3.5 rules:
    ---------------
      * Response messages from the GM must start less than 200ms from the last byte of the command message from this.
      * Suggested inter-byte timeout value is 5ms, which starts when the first byte of the response from the GM
        is received.  Timer expiration can be used to trigger response message evaluation.  Otherwise, the
        message [fragment] (if any) must be evaluated 200ms after the last byte of the command message was sent.
      * Another command message must not be sent any sooner than 225ms from the time the last byte of the
        most recent command message was sent, when the GM does not respond, i.e. gap between bytes in two different
        consecutive command messages must be no shorter than 225ms.
      * Another command message must not be sent any sooner than 10ms from the time of the last byte of the response
        message from the GM, when the GM does respond.
      * Commands from the host to the GM are serialized and no more than _one_ command may be in-progress/active
        at any point in time.  When the host sends a command to the GM while a command is in-progress on the GM,
        the command in progress is cancelled and superceeded by the most recent command received, which is started.
      * The GM _only_ sends messages to the host in response to a command message from the host.
        The GM _never_ sends unsolicited messages to the host.  The host must poll the GM to determine if
        results for the most recent command are available and to obtain them.
      * Communication is full duplex.  Both host and GM may be transmitting simultaneously, however protocol
        state machine rules seem to prevent this sceanrio (excluding malfunction scenarios).
*/
class GatHost
    : public QThread
    , public GatHostPrivilegesForGatHostCmdInterface
    , public GatLinkLayerStrategyInterface
{
    Q_OBJECT

    //! \name GAT Operations
    //! @{
public:
    typedef std::shared_ptr<GatHostCmd> gat_host_cmd_ptr_type;

    void schedule(gat_host_cmd_ptr_type operationCommand); //!< This takes ownership of the operation!

protected slots:
    virtual void onGatHostCmdStateChanged(GatHostCmd *cmd, GatHostCmd::CmdState cmdState);

protected:
    enum class LocalEventType : int
    {
        CmdQueueChanged = QEvent::User,
        ReleaseCmdInPgrs
    };

    void customEvent(QEvent *event);

private:
    void failPendingGatCmds(uint commandCountToLeaveInQueue = 0);
    void cleanupAllGatCommands() throw();
    void scheduleCmdQueueService();

    typedef std::deque<gat_host_cmd_ptr_type> cmds_type;
    cmds_type cmds_;
    gat_host_cmd_ptr_type cmdInProgress_;

    friend class GatHostGetSpecialFunctionsCmd;
    friend class GatHostGetFileCmd;
    friend class GatHostGetComponentCmd;
    //! @}

    //! \name GatHostPrivilegesForGatHostCmdInterface
    //! @{
public:
    virtual GatLinkLayer& gatLinkLayer() { return gatLinkLayer_; }

protected:
    virtual void cancelGatCmdInPgrs();
    //! @}

    //! \name GatLinkLayerStrategyInterface
    //! @{
protected:
    virtual uint write(GatLinkLayer &host, void const *data, uint dataSizeInBytes);
    virtual void onLinkToHost(GatLinkLayer *host);
    //! @}

    //! \name Synchronization Domain
    //! @{
public:
    QMutex * syncDomainGuard() const { return &syncDomainGuard_; }

private:
    mutable QMutex syncDomainGuard_;
    //! @}

    //! \name Thread Management
    //! @{
public:
    void startup(char const *serialDevicePathname);

    using QThread::isRunning;
    QString serialDevicePathname() const;

signals:
    void startupState(GatHost *host, GatHostStartupStateId startupState, QString const &description);

public slots:
    void shutdown(bool waitForTermination);

protected:
    virtual void run();
    //! @}

    //! \name Miscellaneous
    //! @{
protected slots:
    virtual void onRxDataReady();
    virtual void onTimer();
    virtual void onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId state);

private:
    QString portErrorDesc() const;

    QString serialDevicePathname_;
    QSerialPort serialPort_; // http://qt-project.org/wiki/QtSerialPort#7868ff75ba2ba6671f178bc8fb7da0fd
    GatLinkLayer gatLinkLayer_;
    QTimer timer_;
    //! @}

    //! \name Construction, Destruction, and Assignment
    //! @{
public:
    GatHost();
    virtual ~GatHost();

private:
    GatHost(GatHost const& ) = delete; //!< No cloning; leave unimplemented.
    GatHost& operator=(GatHost const& ) = delete; //!< No cloning; leave unimplemented.
    //! @}
};


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


inline GatHostPrivilegesForGatHostCmdInterface &
GatHostCmd::hostPrivileges()
{
    return static_cast<GatHostPrivilegesForGatHostCmdInterface &>(host_);
}


inline GatHostPrivilegesForGatHostCmdInterface const &
GatHostCmd::hostPrivileges() const
{
    return static_cast<GatHostPrivilegesForGatHostCmdInterface const &>(host_);
}


inline QMutex *
GatHostCmd::syncDomainGuard() const
{
    return host_.syncDomainGuard();
}


#endif // #ifndef GATHOST_HPP__BDE65372_4E6A_46CB_8155_B0A2A7D5BE33__INCLUDED


/*
    End of "GatHost.hpp"
*/
