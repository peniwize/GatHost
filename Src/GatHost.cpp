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


#include "GatHost.hpp"
#include <QApplication>
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/


void
GatHostCmd::begin()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    setCmdState(CmdState::Started);
}


void
GatHostCmd::cancel()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    setCmdState(CmdState::Canceled);
}


void
GatHostCmd::fail()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    setCmdState(CmdState::Failed);
}


void
GatHostCmd::processGmResponse(uint8_t const *data, size_t dataSize)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    if (nullptr != data && 0 < dataSize)
    {
        Q_ASSERT(sizeof(char) == sizeof(uint8_t));
        setLastGatOpResult(QByteArray(reinterpret_cast<char const *>(static_cast<uchar const *>(data)), dataSize));
    }
    else
    {
        setLastGatOpResult(QByteArray());
    }
    setCmdState(CmdState::Completed);
}


QByteArray
GatHostCmd::lastGatOpResult() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    return QByteArray(operationResult_.constData(), operationResult_.size()); // Clone return value!
}


QString
GatHostCmd::gatSpecialFunctionName() const
{
    // No need to enter synchronization domain for this particular method implementation.
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    return "GAT Command";
}


GatLinkLayer::RequestResult
GatHostCmd::sendRequest(GatRqst gatRequest, void const *data, uint dataSize)
{
    GatHostPrivilegesForGatHostCmdInterface *priv = static_cast<GatHostPrivilegesForGatHostCmdInterface *>(&host());
    GatLinkLayer::RequestResult const result = priv->gatLinkLayer().sendRequest(gatRequest, data, dataSize);
    bool const failedToTransmit = GatLinkLayer::RequestResult::Success != result &&
                                  GatLinkLayer::RequestResult::Pending != result;
    if (failedToTransmit) { scheduleFailureDpc(); }
    return result;
}


void
GatHostCmd::scheduleFailureDpc()
{
    // Post an event to this to invoke a DPC for operation command processing.
    QEvent *event = new QEvent(static_cast<QEvent::Type>(ELocalEventType::FailThisDpc));
    QApplication::postEvent(this, event);
}


void
GatHostCmd::customEvent(QEvent *event)
{
    QObject::customEvent(event);

    if (nullptr != event &&
        static_cast<uint>(ELocalEventType::FailThisDpc) == static_cast<uint>(event->type()))
    {
        setCmdState(CmdState::Failed);
    }
}


GatHostCmd::CmdState
GatHostCmd::setCmdState(CmdState value)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    if (cmdState() != value)
    {
        cmdState_ = value;
#ifdef DEBUG
        static char const *cmdStateNames[] = {
            "Started", // ECmdState::Started
            "Canceled", // ECmdState::Canceled
            "Timeout", // ECmdState::Failed_Timeout
            "Failed", // ECmdState::Failed
            "Completed", // ECmdState::Completed
            "Undefined ", // ECmdState::Undefined
        };
        Q_ASSERT(arycap(cmdStateNames) == cmdState_Count);
        size_t const valueAsInt = static_cast<size_t>(value);
        qDebug() << "CGatHostCmd::SetCmdState(ECmdState "
                 << (arycap(cmdStateNames) > valueAsInt ? cmdStateNames[valueAsInt] : QString().arg(valueAsInt))
                 << ")";
#endif // #ifdef DEBUG
        onCmdStateChanged();
    }

    return cmdState_;
}


void
GatHostCmd::onCmdStateChanged()
{
    emit gatHostCmdStateChanged(this, cmdState_);
}


void
GatHostCmd::onLinkLayerStateChanged(GatLinkLayer *host, GatLinkLayer::StateId state)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    /*
        This logic is for a _single_ packet command with a _single_ packet reply, e.g. "IACQ(0x04)/IACR(0x84)".
        Special functions require the use of CGatHostSpecialFxnCmd, which should be inherited.
    */
    if (CmdState::Started == cmdState())
    {
        switch (state)
        {
            case GatLinkLayer::StateId::Ready:
            case GatLinkLayer::StateId::WaitForNextTxTime:
            case GatLinkLayer::StateId::Transmit:
            case GatLinkLayer::StateId::Receive:
                // Do nothing.  These link layer states can be ignored here.
                break;

            default:
            case GatLinkLayer::StateId::Undefined: // Unexpected link layer state.
            {
                setCmdState(CmdState::Failed);
                break;
            }

            case GatLinkLayer::StateId::Timeout:
            {
                setCmdState(CmdState::Failed_Timeout);
                break;
            }

            case GatLinkLayer::StateId::Reply:
            {
                GatLinkLayer::reply_type reply(host->reply());
                if ((nullptr == reply.first && 0 < reply.second) // Implementation error.
                    || GatLinkLayer::ResultType::Reply != host->resultType())
                {
                    setCmdState(CmdState::Failed);
                }
                else
                {
                    processGmResponse(reinterpret_cast<uint8_t const *>(reply.first), reply.second);
                }
                break;
            }
        }
    }
}


GatHostCmd::GatHostCmd(GatHost &gatHost)
    : cmdState_(CmdState::Undefined)
    , host_(gatHost)
{
    { // Subscribe to signals from 'CGatLinkLayer'.
        QMutexLocker syncDomainLock(host().syncDomainGuard());
        connect(&hostPrivileges().gatLinkLayer(),
                SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
                this,
                SLOT(onLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));
    }
}


GatHostCmd::~GatHostCmd()
{
    { // Unsubscribe from signals from 'CGatLinkLayer'.
        QMutexLocker syncDomainLock(host().syncDomainGuard());
        disconnect(&hostPrivileges().gatLinkLayer(),
                   SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
                   this,
                   SLOT(onLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));
    }
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GatHostSpecialFxnCmd::begin()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    // Get parameters for special function.
    QStringList params;
    gatSpecFxnParams(params);

    // Start execution of special command [state machine].
    bool const success = specialFxnExec_.sendRequest(params);
    if (success) { setCmdState(CmdState::Started); }
    else         { fail(); }
}


void
GatHostSpecialFxnCmd::onLinkLayerStateChanged(GatLinkLayer * /*host*/, GatLinkLayer::StateId /*state*/)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

//! \todo I don't like this design.  Revisit and refactor.

    // Do nothing.  Shunt base class functionality.
    // Perhaps a strategy is a better solution (changes behavior in/of derivatives).
}


void
GatHostSpecialFxnCmd::onSpecialFunctionExecStateChanged(GatSpecialFunctionExec * /*host*/,
                                                        GatSpecialFunctionExec::StateId newState)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    /*
        This logic is for a special function.  Single packet replies require the use of CGatHostCmd.
    */
    if (CmdState::Started == cmdState())
    {
        switch (newState)
        {
            case GatSpecialFunctionExec::StateId::Ready:
            case GatSpecialFunctionExec::StateId::Requesting:
            case GatSpecialFunctionExec::StateId::WaitingForReply:
            case GatSpecialFunctionExec::StateId::ReceivingReply:
                // Do nothing.  These link layer states can be ignored here.
                break;

            default:
            case GatSpecialFunctionExec::StateId::Undefined: // Unexpected link layer state.
            case GatSpecialFunctionExec::StateId::RequestFailed:
            case GatSpecialFunctionExec::StateId::ReplyFailed:
            {
                setCmdState(CmdState::Failed);
                break;
            }

            case GatSpecialFunctionExec::StateId::RequestFailed_Timeout:
            case GatSpecialFunctionExec::StateId::ReplyUnavailable_Timeout:
            {
                setCmdState(CmdState::Failed_Timeout);
                break;
            }

            case GatSpecialFunctionExec::StateId::ReplyReady:
            {
                GatSpecialFunctionExec::reply_type reply(specialFxnExec_.reply());
                if (nullptr == reply.first && 0 < reply.second) // Implementation error.
                {
                    setCmdState(CmdState::Failed);
                }
                else
                {
                    processGmResponse(reinterpret_cast<uint8_t const *>(reply.first), reply.second);
                }
                break;
            }
        }
    }
}


QString
GatHostSpecialFxnCmd::gatSpecialFunctionName() const
{
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    return "Special Function";
}


GatHostSpecialFxnCmd::GatHostSpecialFxnCmd(GatHost &host)
    : GatHostCmd(host)
{
    specialFxnExec_.setParent(this);

/*
    // Unsubscribe from signals subscribed to in 'CGatHostCmd::CGatHostCmd(CGatHost&)'.
    disconnect(&hostPrivileges().gatLinkLayer(),
               SIGNAL(StateChanged(GatLinkLayer*, GatLinkLayer::State)),
               this,
               SLOT(onLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::State)));
*/

    // Subscribe to signals from 'GatMultipktRply'.
    connect(&specialFxnExec_, SIGNAL(specialFunctionExecStateChanged(GatSpecialFunctionExec *, GatSpecialFunctionExec::StateId)),
            this, SLOT(onSpecialFunctionExecStateChanged(GatSpecialFunctionExec *, GatSpecialFunctionExec::StateId)));

    specialFxnExec_.setLinkLayer(&static_cast<GatHostPrivilegesForGatHostCmdInterface &>(host).gatLinkLayer());
}


GatHostSpecialFxnCmd::~GatHostSpecialFxnCmd()
{
    // Unsubscribe from signals from 'CGatMultipktRply'.
    disconnect(&specialFxnExec_, SIGNAL(specialFunctionExecStateChanged(GatSpecialFunctionExec *, GatSpecialFunctionExec::StateId)),
               this, SLOT(onSpecialFunctionExecStateChanged(GatSpecialFunctionExec *, GatSpecialFunctionExec::StateId)));
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GatHostGetSpecialFunctionsCmd::gatSpecFxnParams(QStringList &result)
{
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    result.append("Get Special Functions");
}


QString
GatHostGetSpecialFunctionsCmd::gatSpecialFunctionName() const
{
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    return "Get Special Functions";
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GatHostGetFileCmd::gatSpecFxnParams(QStringList &result)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    result.append("Get File");
    result.append(fileName());
    result.append(params());
}


QString
GatHostGetFileCmd::gatSpecialFunctionName() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    return QString("Get File ") + fileName_;
}


QStringList
GatHostGetFileCmd::params() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    QStringList result;
    for (auto iter = params_.begin(); params_.end() != iter; ++iter)
    {
        result.push_back(QString().append(*iter));
    }

    return result;
}


QString
GatHostGetFileCmd::fileName(FileName fileName)
{
    static char const *fileNames[] = {
        "AuthenticationResponse.xml", // EFileName_AuthResponse
        "ProgramID.xml",              // EFileName_ProgId
        "",                           // EFileName_None
    };
    Q_ASSERT(arycap(fileNames) == fileName_Count);
    size_t const fileNameIdx = static_cast<size_t>(fileName);
    return fileNames[static_cast<size_t>(arycap(fileNames) <= fileNameIdx ? FileName::None : fileName)];
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GatHostGetComponentCmd::gatSpecFxnParams(QStringList &result)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    result.append("Component");
    result.append(componentName());
    result.append(params());
}


QString
GatHostGetComponentCmd::gatSpecialFunctionName() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    return QString("Component ") + componentName_;
}


QStringList
GatHostGetComponentCmd::params() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    QStringList result;
    for (auto iter = params_.begin(); params_.end() != iter; ++iter)
    {
        result.push_back(QString().append(*iter));
    }

    return result;
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GatHost::schedule(gat_host_cmd_ptr_type operationCommand)
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    // Queue the command.
    operationCommand->moveToThread(this);
    cmds_.push_back(operationCommand);
    scheduleCmdQueueService();
}


void
GatHost::failPendingGatCmds(uint commandCountToLeaveInQueue)
{
    // Purge and fail all but the last command in the queue (only one can be in progress at a time).
    while (cmds_.size() > commandCountToLeaveInQueue)
    {
        gat_host_cmd_ptr_type cmd = cmds_.front();
        cmds_.pop_front();
        try { cmd->fail(); } catch (...) { qWarning("Unexpected exception caught and discarded in CGatHost::customEvent(QEvent *). " STRINGIZE(__LINE__)); }
    }
}


void
GatHost::cleanupAllGatCommands() throw()
{
    try { cancelGatCmdInPgrs(); } catch (...) { }
    try { failPendingGatCmds(); } catch (...) { }
}


void
GatHost::scheduleCmdQueueService()
{
    if (!cmds_.empty())
    {
        // Post an event to this to invoke a DPC for operation command processing.
        QEvent *event = new QEvent(static_cast<QEvent::Type>(LocalEventType::CmdQueueChanged));
        QApplication::postEvent(this, event);
    }
}


void
GatHost::onGatHostCmdStateChanged(GatHostCmd *cmd, GatHostCmd::CmdState cmdState)
{
    switch (cmdState)
    {
        case GatHostCmd::CmdState::Canceled:
        case GatHostCmd::CmdState::Failed_Timeout:
        case GatHostCmd::CmdState::Failed:
        case GatHostCmd::CmdState::Completed:
        {
            if (cmdInProgress_.get() == cmd)
            {
                // Schedule DPC.
                QEvent *event = new QEvent(static_cast<QEvent::Type>(LocalEventType::ReleaseCmdInPgrs));
                QApplication::postEvent(this, event);
            }
            else
            {
                cmds_type::iterator iter = std::find_if(cmds_.begin(), cmds_.end(),
                    [cmd](gat_host_cmd_ptr_type& a) -> bool { return a.get() == cmd; });
                if (cmds_.end() != iter) { cmds_.erase(iter); }
            }
            break;
        }
        default: break; // Prevent compiler warning.
    }
}


void
GatHost::customEvent(QEvent *event)
{
    QThread::customEvent(event);

    if (nullptr != event)
    {
        if (static_cast<uint>(LocalEventType::CmdQueueChanged) == static_cast<uint>(event->type()))
        {
            if (!cmds_.empty())
            {
                // If a command is in progress, cancel it.
                cancelGatCmdInPgrs();

                // Purge and fail all but the last command in the queue (only one can be in progress at a time).
                failPendingGatCmds(1);

                // Remove the last command from the queue, make it current, and start it.
                Q_ASSERT(!cmds_.empty());
                if (!cmds_.empty())
                {
                    cmdInProgress_ = cmds_.front();
                    cmds_.pop_front();
                    connect(cmdInProgress_.get(), SIGNAL(gatHostCmdStateChanged(GatHostCmd *, GatHostCmd::CmdState)),
                            this, SLOT(onGatHostCmdStateChanged(GatHostCmd *, GatHostCmd::CmdState)));
                    try { cmdInProgress_->begin(); } catch (...) { qWarning("Unexpected exception caught and discarded in CGatHost::customEvent(QEvent *). " STRINGIZE(__LINE__)); }
                }

                // Failsafe (to ensure queue never stalls).
                if (!cmds_.empty()) { scheduleCmdQueueService(); }
            }
        }
        else if (static_cast<uint>(LocalEventType::ReleaseCmdInPgrs) == static_cast<uint>(event->type()))
        {
            cmdInProgress_.reset();
        }
    }
}


void
GatHost::cancelGatCmdInPgrs()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    if (nullptr != cmdInProgress_.get())
    {
        disconnect(cmdInProgress_.get(), SIGNAL(gatHostCmdStateChanged(GatHostCmd *, GatHostCmd::CmdState)),
                   this, SLOT(onGatHostCmdStateChanged(GatHostCmd *, GatHostCmd::CmdState)));
        try { cmdInProgress_->cancel(); } catch (...) { qWarning("Unexpected exception caught and discarded in CGatHost::customEvent(QEvent *). " STRINGIZE(__LINE__)); }
        try { cmdInProgress_.reset(); } catch (...) { qWarning("Unexpected exception caught and discarded in CGatHost::customEvent(QEvent *). " STRINGIZE(__LINE__)); }
    }
}


uint
GatHost::write(GatLinkLayer & /*host*/, void const *data, uint dataSizeInBytes)
{
    return serialPort_.write(reinterpret_cast<char const *>(data), dataSizeInBytes);
}


void
GatHost::onLinkToHost(GatLinkLayer * /*host*/)
{
    // Do nothing.
}


void
GatHost::startup(char const *serialDevicePathname)
{
    if (isRunning() || nullptr == serialDevicePathname) { return; }

    QMutexLocker syncDomainLock(syncDomainGuard());

    serialDevicePathname_ = serialDevicePathname;
    start(QThread::HighestPriority); // Priority is more important on Windows than on Linux.
    // Inform Qt that 'this' object is owned by the OS thread it encapsulates.
    // This ensures that signals/slots and QEvents are passed between threads and
    // executed (asynchronously) in the appropriate context.
    // See: http://huntharo.com/2009/08/qthread-signalsslots-why-your-calls-stay-in-the-main-thread/
    //
    // 'moveToThread(QThread *)' can only /push/ an object from the current OS thread to the
    // OS thread implemented by its argument.  It cannot /pull/ a QObject from one OS thread to another,
    // which is why this method is called here and *not* in 'run()'.
    //
    // Note that 'run()' immediately enters the synchronization domain, which causes it to
    // block until this function leaves the synchronization domain.  This ensures that the
    // thread does no substantial work until this function returns.
    moveToThread(this);
}


void
GatHost::shutdown(bool waitForTermination)
{
    // No need to enter synchronization domain for this particular method implementation.
    // All called methods are thread safe and/or synchronized by the event queue.
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    if (isRunning())
    {
        quit(); // This could also be called directly by either this thread or another, since quit() is a slot.

        if (waitForTermination && this != currentThread())
        {
            wait();
        }
    }

    cleanupAllGatCommands();
}


QString
GatHost::serialDevicePathname() const
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    return QString().append(serialDevicePathname_);
}


void
GatHost::run()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer_.setInterval(1000);
    timer_.start();

    // Initialize GAT state machine (variables).
//! \todo >>> Under Construction <<<

    // Initiailze and open serial port.
    if (serialPort_.isOpen()) { serialPort_.close(); }
    serialPort_.setPortName(serialDevicePathname_); // Use SerialPortInfo class to enumerate devices.
/*

There appears to be a bug in QtAddOn::SerialPort::SerialPort::open().  Opening a port about 20 times in a row
will eventually cause QtAddOn::SerialPort::SerialPort::open() to return false, even though the 'open()' system
call works.  I'm not sure why.  I'll investigate in the future when there's more time.
Eventually the system settles and it works again (usually a couple minutes).  Time seems to be a factor.
This only seems to occur on VMware virtual machines so perhaps it's a bug in the VMware driver.

int handle = open(toStdStr(serialDevicePathname_).c_str(), O_RDWR | O_ASYNC);
if (-1 != handle)
{
    close(handle);
}
else
{
    qWarning() << "Failed to open \"" << serialDevicePathname_ << "\"!" << endl << errnoToStr(errno);
}
*/
    bool const portOpened = serialPort_.open(QIODevice::ReadWrite); // Port is always exclusive to this thread; no other process or thread can access.
    serialPort_.setBaudRate(QSerialPort::Baud9600);
    serialPort_.setDataBits(QSerialPort::Data8);
    serialPort_.setParity(QSerialPort::NoParity);
    serialPort_.setStopBits(QSerialPort::OneStop);
    serialPort_.setFlowControl(QSerialPort::NoFlowControl);
    serialPort_.setReadBufferSize(1024); // Limit read buffer (prevent heap abuse).
    int const errorNumber = errno;
    if (!portOpened)
    {
        QString errorDescription(errnoToStr(errorNumber));
        QString description("Unable to open serial port \"%1\".\n%2\n%3");
        description = description.arg(toStdStr(serialDevicePathname_).c_str(), portErrorDesc(), errorDescription);
        qWarning() << description;
        emit startupState(this, GatHostStartupStateId::Fail, description);
    }
    else
    {
        try
        {
            emit startupState(this, GatHostStartupStateId::Success, "Success.");

            // Start [thread] Qt event loop.
            syncDomainLock.unlock();
            exec(); // Does not return until either exit() or quit() is called.
            syncDomainLock.relock();
        }
        catch (...)
        {
            syncDomainLock.relock();
        }

        // Clean up.
        cleanupAllGatCommands();
        try { serialPort_.close(); } catch (...) { }
    }

    Q_ASSERT(!serialPort_.isOpen());

    disconnect(&timer_, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer_.stop();
}


QString
GatHost::portErrorDesc() const
{
    static char const *errorDescriptions[] = {
        "NoError", // SPN::SerialPort::NoError
        "NoSuchDeviceError", // SPN::SerialPort::NoSuchDeviceError
        "PermissionDeniedError", // SPN::SerialPort::PermissionDeniedError
        "DeviceAlreadyOpenedError", // SPN::SerialPort::DeviceAlreadyOpenedError
        "DeviceIsNotOpenedError", // SPN::SerialPort::DeviceIsNotOpenedError
        "ParityError", // SPN::SerialPort::ParityError
        "FramingError", // SPN::SerialPort::FramingError
        "BreakConditionError", // SPN::SerialPort::BreakConditionError
        "IoError", // SPN::SerialPort::IoError
        "UnsupportedPortOperationError", // SPN::SerialPort::UnsupportedPortOperationError
        "UnknownPortError", // SPN::SerialPort::UnknownPortError
    };
    auto const portError = serialPort_.error();
    auto idx = static_cast<size_t>(portError);
    return errorDescriptions[arycap(errorDescriptions) >= idx ? (arycap(errorDescriptions) - 1) : idx];
}


/*!
    The readyRead() signal (bound to this slot) is emitted every time new serial data arrives.
*/
void
GatHost::onRxDataReady()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    qint64 bytesAvailable = serialPort_.bytesAvailable();
    char buffer[1024];
    Q_ASSERT(arycap(buffer) <= std::numeric_limits<size_t>::max());
    while (0 < bytesAvailable)
    {
        qint64 byteCountToRead = (std::min)(static_cast<qint64>(arycap(buffer)), bytesAvailable);
        qint64 byteCountRead = serialPort_.read(buffer, byteCountToRead);
        gatLinkLayer_.receiveData(buffer, static_cast<size_t>(byteCountRead));
        bytesAvailable -= byteCountRead;
    }
}


void
GatHost::onTimer()
{
#if 0
    static uint idx = 0;
    qDebug("[%u] GatHost::onTimer()", ++idx);
#endif // #if 0
}


void
GatHost::onLinkLayerStateChanged(GatLinkLayer * /*host*/, GatLinkLayer::StateId /*state*/)
{
    // Do nothing.
}


GatHost::GatHost()
    : syncDomainGuard_(QMutex::Recursive)
{
    serialPort_.setParent(this);
    gatLinkLayer_.setParent(this);
    timer_.setParent(this);

    gatLinkLayer_.setStrategy(this);

    // Subscribe to signals.
    connect(&serialPort_, SIGNAL(readyRead()), this, SLOT(onRxDataReady()));
    connect(&gatLinkLayer_, SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
            this, SLOT(onLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));
}


GatHost::~GatHost()
{
    Q_ASSERT(!isRunning());
    shutdown(true);

    // Unsubscribe from signals.
    disconnect(&gatLinkLayer_, SIGNAL(stateChanged(GatLinkLayer*, GatLinkLayer::StateId)),
               this, SLOT(onLinkLayerStateChanged(GatLinkLayer*, GatLinkLayer::StateId)));

    gatLinkLayer_.setStrategy(nullptr);
}


/*
    End of "GatHost.hpp"
*/
