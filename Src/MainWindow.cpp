/*!
    \file "MainWindow.cpp"

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


#include "Defs.hpp"
#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "AboutBox.hpp"
#include "GatCmdSpec.hpp"
#include "SelectSerialPortDlg.hpp"
#include "GatPkt_StatusQueryRslt_SR81.hpp"
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QScrollBar>
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QDomDocument>
#include <QDomNodeList>


int const specialFunctionsIndex = 0;        // Tab UI widget index.
int const lastOperationResultsIndex = 1;    // Tab UI widget index.
int const logIndex = 2;                     // Tab UI widget index.


QString
StatusQueryCmd::gatSpecialFunctionName() const
{
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    return "Status Query (SQ 0x01)";
}


void
StatusQueryCmd::onCmdStateChanged()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    GatHostCmd::onCmdStateChanged();

    switch (cmdState())
    {
        case CmdState::Started:
        {
            // Send request.
            sendRequest(GatRqst::SQ_01);
            break;
        }

        default:
        case CmdState::Canceled:
        case CmdState::Failed:
        case CmdState::Undefined:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Anything other than timeout.", "");
            break;
        }

        case CmdState::Failed_Timeout:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Timeout.", "");
            break;
        }

        case CmdState::Completed:
        {
            mainWindow_.setResult_StatusQuery(this, lastGatOpResult());
            break;
        }
    }
}


StatusQueryCmd::StatusQueryCmd(MainWindow &mainWindow, GatHost &host)
    : GatHostCmd(host)
    , mainWindow_(mainWindow)
{
    // Do nothing.
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


QString
LastAuthStatusQueryCmd::gatSpecialFunctionName() const
{
    //QMutexLocker syncDomainLock(SyncDomainGuard());

    return "Last Authentication Status Query (LASQ 0x02)";
}


void
LastAuthStatusQueryCmd::onCmdStateChanged()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    GatHostCmd::onCmdStateChanged();

    switch (cmdState())
    {
        case CmdState::Started:
        {
            // Send request.
            sendRequest(GatRqst::LASQ_02);
            break;
        }

        default:
        case CmdState::Canceled:
        case CmdState::Failed:
        case CmdState::Undefined:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Anything other than timeout.", "");
            break;
        }

        case CmdState::Failed_Timeout:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Timeout.", "");
            break;
        }

        case CmdState::Completed:
        {
            mainWindow_.setResult_LastAuthStatusQuery(this, lastGatOpResult());
            break;
        }
    }
}


LastAuthStatusQueryCmd::LastAuthStatusQueryCmd(MainWindow &mainWindow, GatHost &host)
    : GatHostCmd(host)
    , mainWindow_(mainWindow)
{
    // Do nothing.
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GetSpecialFunctionsCmd::onCmdStateChanged()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    GatHostGetSpecialFunctionsCmd::onCmdStateChanged();

    switch (cmdState())
    {
        case CmdState::Started:
            // Do nothing.  Ignore this state since it's handled by the base class.
            break;

        default:
        case CmdState::Canceled:
        case CmdState::Failed:
        case CmdState::Undefined:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Anything other than timeout.", "");
            break;
        }

        case CmdState::Failed_Timeout:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Timeout.", "");
            break;
        }

        case CmdState::Completed:
        {
            mainWindow_.setResult_GetSpecialFunctions(this, lastGatOpResult());
            break;
        }
    }
}


GetSpecialFunctionsCmd::GetSpecialFunctionsCmd(MainWindow &mainWindow, GatHost &host)
    : GatHostGetSpecialFunctionsCmd(host)
    , mainWindow_(mainWindow)
{
    // Do nothing.
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
getComponentCmd::onCmdStateChanged()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    GatHostGetComponentCmd::onCmdStateChanged();

    switch (cmdState())
    {
        case CmdState::Started:
            // Do nothing.  Ignore this state since it's handled by the base class.
            break;

        default:
        case CmdState::Canceled:
        case CmdState::Failed:
        case CmdState::Undefined:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Anything other than timeout.", "");
            break;
        }

        case CmdState::Failed_Timeout:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Timeout.", "");
            break;
        }

        case CmdState::Completed:
        {
            mainWindow_.setResult_GetComponent(this, lastGatOpResult());
            break;
        }
    }
}


getComponentCmd::getComponentCmd(MainWindow &mainWindow, GatHost &host,
                                 QString const &componentName, QStringList const *params)
    : GatHostGetComponentCmd(host, componentName, params)
    , mainWindow_(mainWindow)
{
    // Do nothing.
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
GetFileCmd::onCmdStateChanged()
{
    QMutexLocker syncDomainLock(syncDomainGuard());

    GatHostGetFileCmd::onCmdStateChanged();

    switch (cmdState())
    {
        case CmdState::Started:
            // Do nothing.  Ignore this state since it's handled by the base class.
            break;

        default:
        case CmdState::Canceled:
        case CmdState::Failed:
        case CmdState::Undefined:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Anything other than timeout.", "");
            break;
        }

        case CmdState::Failed_Timeout:
        {
            mainWindow_.operationFailed(this, gatSpecialFunctionName(), "Timeout.", "");
            break;
        }

        case CmdState::Completed:
        {
            mainWindow_.setResult_GetFile(this, lastGatOpResult());
            break;
        }
    }
}


GetFileCmd::GetFileCmd(MainWindow &mainWindow, GatHost &host, QString const &componentName, QStringList const *params)
    : GatHostGetFileCmd(host, componentName, params)
    , mainWindow_(mainWindow)
{
    // Do nothing.
}


// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


void
MainWindow::setResult_StatusQuery(GatHostCmd *command, QByteArray const &result)
{
    scheduleDpc(std::bind(&MainWindow::setResult_StatusQuery_Dpc, this, command, result));
}


void
MainWindow::setResult_StatusQuery_Dpc(GatHostCmd *command, QByteArray const &result)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    GatHost::gat_host_cmd_ptr_type gatCmdInProgress(activeGatCmd_);
    activeGatCmd_ = nullptr;

    using namespace std;
    ostringstream oss;

    try
    {
        GatPkt_StatusQueryRslt_SR81 statusQueryResult;
        Q_ASSERT(sizeof(uint8_t) == sizeof(char));
        if (statusQueryResult.parseResultPacket(reinterpret_cast<uint8_t const *>(result.data()), result.size()))
        {
            static char const *calculationStatusNames[] = {
                "Requested", // CGatPkt_StatusQueryRslt_SR81::ECalcStatus::Requested
                "Finished", // CGatPkt_StatusQueryRslt_SR81::ECalcStatus::Finished
                "Calculating", // CGatPkt_StatusQueryRslt_SR81::ECalcStatus::Calculating
                "Error, cannot complete or failed", // CGatPkt_StatusQueryRslt_SR81::ECalcStatus::Error
            };
            if (arycap(calculationStatusNames) >= static_cast<size_t>(statusQueryResult.calculationStatus()))
            {
                oss << "Version ID: "
                    << hex << static_cast<uint>((statusQueryResult.versionInBcd() >> 8) & 0xff)
                    << hex << setw(2) << setfill('0') << static_cast<uint>(statusQueryResult.versionInBcd() & 0xff) << endl
                    << (statusQueryResult.calculationInProgress() ? "Calculating" : "Idle") << endl
                    << "Last Authentication Results " << (statusQueryResult.authResultsReady() ? "A" : "Una") << "vailable" << endl
                    << "Current Calculation: " << calculationStatusNames[static_cast<size_t>(statusQueryResult.calculationStatus())] << endl
                    << "Data Formats: ";
                if (statusQueryResult.dataFormats().empty()) { oss << "<none>"; }
                for (GatPkt_StatusQueryRslt_SR81::data_formats_type::const_iterator iter = statusQueryResult.dataFormats().begin();
                     statusQueryResult.dataFormats().end() != iter;
                     ++iter)
                {
                    if (statusQueryResult.dataFormats().begin() != iter) { oss << ", "; }
                    switch (*iter)
                    {
                        case GatDataFormat::PlainText: oss << "Plain text (0x01)"; break;
                        case GatDataFormat::Xml: oss << "XML (0x02)"; break;
                        default: oss << "Unrecognized (0x" << hex << uppercase << setw(2) << setfill('0') << ")"; break;
                    }
                }
                oss << endl;
            }
        }
        else
        {
            oss << "Received invalid status response.";
        }
    }
    catch (...)
    {
        activeGatCmd_ = gatCmdInProgress;
        operationFailed_Dpc(command, "Status Query", "Internal error.", "");
        return;
    }

    populateOperationResults(oss.str().c_str());
}


void
MainWindow::setResult_LastAuthStatusQuery(GatHostCmd *command, QByteArray const &result)
{
    scheduleDpc(std::bind(&MainWindow::setResult_LastAuthStatusQuery_Dpc, this, command, result));
}


void
MainWindow::setResult_LastAuthStatusQuery_Dpc(GatHostCmd *command, QByteArray const &result)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    GatHost::gat_host_cmd_ptr_type gatCmdInProgress(activeGatCmd_);
    activeGatCmd_ = nullptr;

    using namespace std;
    ostringstream oss;

    try
    {
        if (9 == result.size() && 9 == result[1])
        {
            uint const authLevel = static_cast<unsigned char>(result[2]);
            uint_least32_t const secondsSinceLastCalculation = static_cast<uint_least32_t>(result[3]) << 24 |
                                                               static_cast<uint_least32_t>(result[4]) << 16 |
                                                               static_cast<uint_least32_t>(result[5]) <<  8 |
                                                               static_cast<uint_least32_t>(result[6]) <<  0;
            oss << "Authentication Level: " << hex << setw(2) << setfill('0') << authLevel << endl
                << "Time: " << dec << secondsSinceLastCalculation << endl;
        }
        else
        {
            oss << "Received invalid last authentication status response.";
        }
    }
    catch (...)
    {
        activeGatCmd_ = gatCmdInProgress;
        operationFailed_Dpc(command, "Status Query", "Internal error.", "");
        return;
    }

    populateOperationResults(oss.str().c_str());
}


void
MainWindow::setResult_GetSpecialFunctions(GatHostCmd *command, const QByteArray &result)
{
    scheduleDpc(std::bind(&MainWindow::setResult_GetSpecialFunctions_Dpc, this, command, result));
}


void
MainWindow::setResult_GetSpecialFunctions_Dpc(GatHostCmd *command, const QByteArray &result)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    GatHost::gat_host_cmd_ptr_type gatCmdInProgress(activeGatCmd_);
    activeGatCmd_ = nullptr;

    bool const isUtf16 = ui->rdoXmlAuto->isChecked()
                         ? (2 <= result.size() && (!isprint(result[0]) || !isprint(result[1])))
                         : ui->rdoXmlUtf16->isChecked();

    QString resultText(isUtf16 ? QString::fromUtf16(reinterpret_cast<ushort const *>(result.data()))
                               : QString::fromUtf8(reinterpret_cast<char const *>(result.data())));

    populateOperationResults(resultText);

    typedef QList<QStringList> TStringsColl;
    TStringsColl functionDefs;
    uint columnCount = 0;

    try
    {
        QDomDocument doc; // See: http://doc.qt.digia.com/qt/qdomdocument.html
        doc.setContent(resultText);
        QDomNodeList specialFunctionsNodes(doc.elementsByTagName("SpecialFunctions"));
        if (specialFunctionsNodes.isEmpty()) { throw int(-1); }
        QDomNode specialFunctionsNode(specialFunctionsNodes.at(0));
        for (QDomElement element(specialFunctionsNode.firstChildElement("Function"));
             !element.isNull();
             element = element.nextSiblingElement("Function"))
        {
            QDomElement feature(element.firstChildElement("Feature"));
            if (feature.isNull()) { throw int(-1); }

            QStringList stringList;
            stringList.push_back(feature.text());

            for (QDomElement parameter(element.firstChildElement("Parameter"));
                 !parameter.isNull();
                 parameter = parameter.nextSiblingElement("Parameter"))
            {
                stringList.push_back(parameter.text());
            }

            functionDefs.push_back(stringList);
            columnCount = (std::max)(columnCount, static_cast<uint>(stringList.size()));
        }

        initSpecialFunctionsView(functionDefs, columnCount);
        populateSpecialFunctionsView(functionDefs);
        if (logIndex != ui->tabWidget->currentIndex()) { ui->tabWidget->setCurrentIndex(specialFunctionsIndex); }
    }
    catch (...)
    {
        activeGatCmd_ = gatCmdInProgress;
        operationFailed_Dpc(command, "Get Special Functions", "Bad XML data received from GM.", resultText);
        return;
    }
}


void
MainWindow::setResult_GetComponent(GatHostCmd *command, const QByteArray &result)
{
    scheduleDpc(std::bind(&MainWindow::setResult_GetComponent_Dpc, this, command, result));
}


void
MainWindow::setResult_GetComponent_Dpc(GatHostCmd *command, const QByteArray &result)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    activeGatCmd_ = nullptr;

    bool const isUtf16 = ui->rdoXmlAuto->isChecked()
                         ? (2 <= result.size() && (!isprint(result[0]) || !isprint(result[1])))
                         : ui->rdoXmlUtf16->isChecked();

    QString resultText(isUtf16 ? QString::fromUtf16(reinterpret_cast<ushort const *>(result.data()))
                               : QString::fromUtf8(reinterpret_cast<char const *>(result.data())));

    populateOperationResults(resultText);
}


void
MainWindow::setResult_GetFile(GatHostCmd *command, QByteArray const& result)
{
    scheduleDpc(std::bind(&MainWindow::setResult_GetFile_Dpc, this, command, result));
}


void
MainWindow::setResult_GetFile_Dpc(GatHostCmd *command, const QByteArray &result)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    activeGatCmd_ = nullptr;

    bool const isUtf16 = ui->rdoXmlAuto->isChecked()
                         ? (2 <= result.size() && (!isprint(result[0]) || !isprint(result[1])))
                         : ui->rdoXmlUtf16->isChecked();

    QString resultText(isUtf16 ? QString::fromUtf16(reinterpret_cast<ushort const *>(result.data()))
                               : QString::fromUtf8(reinterpret_cast<char const *>(result.data())));

    populateOperationResults(resultText);
}


void
MainWindow::operationFailed(GatHostCmd *command, QString const &specialFuncionName,
                            QString const &description, const QString &operationResultDescription)
{
    scheduleDpc(std::bind(&MainWindow::operationFailed_Dpc, this, command,
                          specialFuncionName, description, operationResultDescription));
}


void
MainWindow::operationFailed_Dpc(GatHostCmd *command, QString const &specialFunctionName,
                                QString const &description, const QString &operationResultDescription)
{
    // Do nothing when this is not from the active command.
    if (activeGatCmd_.get() != command) { return; }
    activeGatCmd_ = nullptr;

    // Update status bar.
    QString textMessage("FAILED: ");
    textMessage += specialFunctionName;
    ui->statusBar->showMessage(textMessage);

    // Update last operation result view.
    int pageIdxToSwitchTo = /*lastOperationResultsIndex*/ specialFunctionsIndex;
    if (pageIdxToSwitchTo != ui->tabWidget->currentIndex() &&
        logIndex != ui->tabWidget->currentIndex())
    {
        ui->tabWidget->setCurrentIndex(pageIdxToSwitchTo);
    }
    if (operationResultDescription.isEmpty())
    {
        ui->lastOpRsltEdit->setText(textMessage);
    }
    else
    {
        QString lastOperationResultText(textMessage);
        lastOperationResultText += "\n";
        lastOperationResultText += description;
        lastOperationResultText += "\n\nOperation result:\n";
        lastOperationResultText += operationResultDescription;
        ui->lastOpRsltEdit->setText(lastOperationResultText);
    }

    // Display info box.
    textMessage += "\n";
    textMessage += description;
    QInfoMsgBox(textMessage, this);

    showOperationInProgressUiWidgets(false);
}


void
MainWindow::postEvent(QEvent *event)
{
    QApplication::postEvent(this, event);
}


void
MainWindow::customEvent(QEvent *event)
{
    QMainWindow::customEvent(event);

    if (nullptr != event)
    {
        switch (static_cast<LocalEventType>(event->type()))
        {
            case LocalEventType::Dpc:
            {
                Dpc *dpcEvent = reinterpret_cast<Dpc *>(event);
                (*dpcEvent)();
                break;
            }

            default:
            {
                qDebug() << "Unhandled local event type!" << endl;
                Q_ASSERT(false);
                break; // Prevent compiler warning.
            }
        }
    }
}


void
MainWindow::selectSerialDevice()
{
    if (!gatHost_.isRunning())
    {
        SelectSerialPortDlg dlg(this);
        dlg.setWindowModality(Qt::ApplicationModal);
        dlg.setSerialDevice(ui->serialDeviceEdit->text());
        if (QDialog::Accepted == dlg.exec())
        {
            ui->serialDeviceEdit->setText(dlg.serialDevice());
        }
    }

    syncUiWidgets();
}


void
MainWindow::openCloseGatDevice()
{
    if (gatHost_.isRunning())
    {
        ui->statusBar->showMessage("Closing \"" + gatHost_.serialDevicePathname() + "\"...");
        gatHost_.shutdown(true);
        ui->statusBar->showMessage("");
    }
    else
    {
        QString pathnameAsQString(ui->serialDeviceEdit->text());
        pathnameAsQString = pathnameAsQString.trimmed();
        std::string pathname(toStdStr(pathnameAsQString));
        gatHost_.startup(pathname.c_str());
        ui->statusBar->showMessage("Opening \"" + gatHost_.serialDevicePathname() + "\"...");
    }

    syncUiWidgets();
}


void
MainWindow::onGatHostStartupState(GatHost * /*host*/, GatHostStartupStateId startupState, QString const& description)
{
    if (GatHostStartupStateId::Success != startupState)
    {
        ui->statusBar->showMessage("Error: Unable to open \"" + gatHost_.serialDevicePathname() +
                                   "\"!", statusMessageDuration);
        QErrMsgBox(description, this);
    }
    else
    {
        ui->statusBar->showMessage("Opened \"" + gatHost_.serialDevicePathname() + "\"", statusMessageDuration);
    }
}


void
MainWindow::onGatHostRunStateChange()
{
    syncUiWidgets();
}


void
MainWindow::onTxPacket(GatLinkLayer * /*host*/, QByteArray pktData)
{
    using namespace std;
    ostringstream oss;
    simpleHexDump(oss, pktData.constData(), pktData.size());
    writeToLog(QString("<font color=\"blue\">p&gt; %1</font>").arg(oss.str().c_str()));
}


void
MainWindow::onRxD(GatLinkLayer * /*host*/, QByteArray rawData)
{
    if (ui->chkIoLogShowRawRxD->isChecked())
    {
        using namespace std;
        ostringstream oss;
        simpleHexDump(oss, rawData.constData(), rawData.size());
        writeToLog(QString("<font color=\"dark gray\">r&lt; %1</font>").arg(oss.str().c_str()));
    }
}


void
MainWindow::onRxPacket(GatLinkLayer * /*host*/, QByteArray pktData, bool invalidPkt)
{
    using namespace std;
    ostringstream oss;
    simpleHexDump(oss, pktData.constData(), pktData.size());
    writeToLog(QString("<font color=\"%1\">p&lt; %2%3</font>")
               .arg(invalidPkt ? "red" : "green")
               .arg(invalidPkt ? "!INVALID! " : "")
               .arg(oss.str().c_str()));
}


void
MainWindow::syncUiWidgets()
{
    bool const isRunning = gatHost_.isRunning();

    uint rowSelectCount = 0;
    uint componentSelectCount = 0;
    uint getFilesSelectCount = 0;
    QList<QTableWidgetItem *> selectedItems(ui->tableWidget->selectedItems());
    for (int idx = 0; selectedItems.size() > idx; ++idx)
    {
        QTableWidgetItem *twi = selectedItems[idx];
        if (0 == twi->column())
        {
            ++rowSelectCount;
            if (twi->text() == "Component")
            {
                ++componentSelectCount;
            }
            else if (twi->text() == "Get File")
            {
                ++getFilesSelectCount;
            }
        }
    }

    ui->openCloseButton->setText(isRunning ? "Cl&ose" : "&Open");
    ui->openCloseButton->setIcon(QIcon(QString(isRunning ? ":/Icons/open" : ":/Icons/close")));
    QColor color(palette().color(QPalette::ColorGroup::Active, isRunning
                                 ? QPalette::ColorRole::Window
                                 : QPalette::ColorRole::Base));
    ui->serialDeviceEdit->setStyleSheet(QString("background-color: ") + color.name());
    ui->serialDeviceEdit->setReadOnly(isRunning);
    ui->showSerialDevicesButton->setEnabled(!isRunning);
    ui->actionShowSerialDevices->setEnabled(!isRunning);
    ui->actionOpenSerialDevice->setText(isRunning ? "Close serial device" : "Open serial device");
    ui->actionGetSpecialFunctions->setEnabled(isRunning);
    ui->actionGetFile->setEnabled(isRunning && 1 == rowSelectCount && 1 == getFilesSelectCount);
    ui->actionGetComponent->setEnabled(isRunning && 1 == rowSelectCount && 1 == componentSelectCount);
    ui->actionStatusQuery->setEnabled(isRunning);
    ui->actionLastAuthenticationStatusQuery->setEnabled(isRunning);
}


void
MainWindow::populateOperationResults(QString const &value)
{
    ui->lastOpRsltEdit->setText(value);
    ui->statusBar->showMessage("");
}


void
MainWindow::showOperationInProgressUiWidgets(bool /*show*/)
{

//! \todo >>> Under Construction <<<

}


void
MainWindow::initSpecialFunctionsView(QList<QStringList> const &functionDefinitions, uint columns)
{
    if (1 > columns) { return; }

    Qt::ScrollBarPolicy const vertScrollBarPolicy = ui->tableWidget->verticalScrollBarPolicy();
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    ui->tableWidget->updateGeometry();

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(columns);
    QStringList headers;
    for (size_t iIdx = 0; columns > iIdx; ++iIdx) { headers.push_back("x"); }
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setVisible(0 < columns);

    if (0 < columns)
    {
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("x"));
    }

    int columnMargin = 10;
    int dynamicColumnIndex = 1; // Column index os column that will take up all extra space.

    // Calculate the text width of each column.
    std::vector<int> columnWidths;
    int dynamicColumnWidth = 0;
    int totalWidth = 0;
    for (QStringList const &functionDefinition : functionDefinitions)
    {
        int totalFunctionDefinitionWidth = 0;
        for (size_t idx = 0; static_cast<size_t>(functionDefinition.size()) > idx; ++idx)
        {
#if 0
            QTableWidgetItem *col = ui->tableWidget->horizontalHeaderItem(iIdx);
            Q_ASSERT(nullptr != col);
            QFontMetrics fm(col->font());
#else // #if 0
            QFontMetrics fm(ui->tableWidget->item(0, 0)->font());
#endif // #if 0

            std::fill_n(std::back_inserter(columnWidths), columnWidths.size() < idx ? idx - columnWidths.size() : 0, 0);

            int const stringWidth = fm.size(Qt::TextSingleLine, functionDefinition[idx]).width() + columnMargin;
            if (columnWidths.size() > idx)
            {
                columnWidths[idx] = (std::max)(columnWidths[idx], stringWidth);
            }
            else
            {
                Q_ASSERT(columnWidths.size() == idx);
                columnWidths.push_back(stringWidth);
            }
            totalFunctionDefinitionWidth += columnWidths[idx];
            if (static_cast<size_t>(dynamicColumnIndex) == idx)
            {
                dynamicColumnWidth = (std::max)(dynamicColumnWidth, columnWidths[idx]);
            }
        }

        totalWidth = (std::max)(totalWidth, totalFunctionDefinitionWidth);
    }
    int const vertSbWidth = ui->tableWidget->verticalScrollBar()->width();
    int const tableWidgetWidth = ui->tableWidget->size().width() - vertSbWidth - 2;
    if (tableWidgetWidth > totalWidth &&
        columnWidths.size() > static_cast<size_t>(dynamicColumnIndex))
    {
        dynamicColumnWidth += tableWidgetWidth - totalWidth;
        columnWidths[dynamicColumnIndex] = dynamicColumnWidth;
    }
    Q_ASSERT(columnWidths.size() == columns);

    // Populate column headers.
    for (size_t idx = 0; columnWidths.size() > idx; ++idx)
    {
        ui->tableWidget->setColumnWidth(idx, columnWidths[idx]);

        QTableWidgetItem *col = ui->tableWidget->horizontalHeaderItem(idx);
        Q_ASSERT(nullptr != col);
        col->setText(0 == idx ? "Feature" : QString("Param %1").arg(idx));
        col->setTextAlignment(Qt::AlignLeft);
    }

    ui->tableWidget->setVerticalScrollBarPolicy(vertScrollBarPolicy);
}


void
MainWindow::populateSpecialFunctionsView(QList<QStringList> const &functionDefinitions)
{
    ui->tableWidget->setSortingEnabled(false);

    ui->tableWidget->setRowCount(0);
    Q_ASSERT(0 == static_cast<size_t>(ui->tableWidget->rowCount()));

    for (size_t rowIdx = 0; static_cast<size_t>(functionDefinitions.size()) > rowIdx; ++rowIdx)
    {
        QStringList const &functionProps(functionDefinitions[rowIdx]);
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
        for (size_t colIdx = 0; static_cast<size_t>(functionProps.size()) > colIdx; ++colIdx)
        {
            ui->tableWidget->setColumnCount((std::max)(static_cast<size_t>(ui->tableWidget->columnCount()), colIdx + 1));

            QTableWidgetItem *twi = nullptr;
            Q_ASSERT(static_cast<size_t>(ui->tableWidget->rowCount()) > rowIdx);
            Q_ASSERT(static_cast<size_t>(ui->tableWidget->columnCount()) > colIdx);
            ui->tableWidget->setItem(rowIdx, colIdx, (twi = new QTableWidgetItem(functionProps[colIdx])));
            twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
        }
    }

    ui->tableWidget->sortByColumn(functionDefinitions.size() > 1 ? 1 : 0, Qt::AscendingOrder);
    ui->tableWidget->setSortingEnabled(true);
}


/*!
   \brief Cancels command in progress (if any) and schedules a new one.
   \param apNewCommand Command to schedule.
 */
bool
MainWindow::scheduleGatCommand(GatHost::gat_host_cmd_ptr_type newCommand)
{
    if (nullptr == newCommand) { return false; }

    // Ignore redundant calls.
    if (activeGatCmd_ == newCommand) { return true; }

    try
    {
        // Schedule new command.
        activeGatCmd_ = newCommand;
        if (nullptr != activeGatCmd_)
        {
            ui->lastOpRsltEdit->clear();
            QString messageText("Executing: " + activeGatCmd_->gatSpecialFunctionName());
            ui->statusBar->showMessage(messageText);
            if (ui->chkIoLogShowOnlyOneOp->isChecked()) { ui->edtIoLog->clear(); }
            writeToLog(QString("---------- ---------- ---------- ---------- ----------"), !ui->chkIoLogShowOnlyOneOp->isChecked());
            writeToLog(QString("%1").arg(messageText));
            ui->lastOpRsltEdit->setText(messageText + "...  please wait.");
            if (logIndex != ui->tabWidget->currentIndex()) { ui->tabWidget->setCurrentIndex(lastOperationResultsIndex); }
            try { gatHost_.schedule(newCommand); }
            catch (...) { ui->statusBar->showMessage(""); throw; }
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}


void
MainWindow::writeToLog(QString value, bool writeToIoLog)
{
    QString serialNumber(QString("%1").arg(ioLogMsgSerialNumber_ + 1, 3, 10, QLatin1Char('0')));
    ioLogMsgSerialNumber_ = (ioLogMsgSerialNumber_ + 1) % 1000;
    QString timeStamp(QString("%1").arg(static_cast<uint>(monotonicClock32() % 100000ul), 5));
    QString logLine(QString("<p>%1 %2 %3</p>").arg(serialNumber).arg(timeStamp).arg(value));

    ui->edtLogText->appendHtml(logLine);
    ui->edtLogText->horizontalScrollBar()->setSliderPosition(ui->edtLogText->horizontalScrollBar()->minimum());
    ui->edtLogText->verticalScrollBar()->setSliderPosition((std::max)(0, ui->edtLogText->blockCount() - edtLogTextLineHeight_));

    if (writeToIoLog)
    {
        ui->edtIoLog->appendHtml(logLine);
        ui->edtIoLog->horizontalScrollBar()->setSliderPosition(ui->edtIoLog->horizontalScrollBar()->minimum());
        ui->edtIoLog->verticalScrollBar()->setSliderPosition((std::max)(0, ui->edtIoLog->blockCount() - edtIoLogLineHeight_));
    }
}


void
MainWindow::on_actionExitApplication_triggered()
{
    close();
}


void
MainWindow::on_openCloseButton_clicked()
{
    openCloseGatDevice();
}


void
MainWindow::on_getSpecialFunctionsButton_clicked()
{
    on_actionGetSpecialFunctions_triggered();
}


void
MainWindow::on_showSerialDevicesButton_clicked()
{
    selectSerialDevice();
}


void
MainWindow::on_actionShowSerialDevices_triggered()
{
    selectSerialDevice();
}


void
MainWindow::on_actionGetSpecialFunctions_triggered()
{
    GatHost::gat_host_cmd_ptr_type newCmd(new GetSpecialFunctionsCmd(*this, gatHost_));
    if (!scheduleGatCommand(newCmd))
    {
        QErrMsgBox("Failed to schedule " + newCmd->gatSpecialFunctionName() + ".", this);
    }

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);

    syncUiWidgets();
}


void
MainWindow::on_actionStatusQuery_triggered()
{
    GatHost::gat_host_cmd_ptr_type newCmd(new StatusQueryCmd(*this, gatHost_));
    if (!scheduleGatCommand(newCmd))
    {
        QErrMsgBox("Failed to schedule " + newCmd->gatSpecialFunctionName() + ".", this);
    }

    syncUiWidgets();
}


void
MainWindow::on_actionLastAuthenticationStatusQuery_triggered()
{
    GatHost::gat_host_cmd_ptr_type newCmd(new LastAuthStatusQueryCmd(*this, gatHost_));
    if (!scheduleGatCommand(newCmd))
    {
        QErrMsgBox("Failed to schedule " + newCmd->gatSpecialFunctionName() + ".", this);
    }

    syncUiWidgets();
}


void
MainWindow::on_actionGetComponent_triggered()
{
    onGetComponentOrFile();
    syncUiWidgets();
}


void
MainWindow::on_actionGetFile_triggered()
{
    onGetComponentOrFile();
    syncUiWidgets();
}


void
MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);

    if (Qt::Key_Return == event->key())
    {
        onGetComponentOrFile();
    }
}


void
MainWindow::onGetComponentOrFile()
{
    bool const isRunning = gatHost_.isRunning();
    if (!isRunning) { return; }

    // Get the component name and arguments from the selected item.
    QString name;
    QStringList params;
    enum class GatType : uint { Component, File, Undefined } eType = GatType::Undefined;
    if (0 <= ui->tableWidget->currentRow())
    {
        QList<QTableWidgetItem *> selectedItems(ui->tableWidget->selectedItems());
        for (int idx = 0; selectedItems.size() > idx; ++idx)
        {
            QTableWidgetItem *twi = selectedItems[idx];
            if (0 == twi->column())
            {
                if (twi->text() == "Component")
                {
                    eType = GatType::Component;
                }
                else if (twi->text() == "Get File")
                {
                    eType = GatType::File;
                }
                else
                {
                    name.clear();
                    params.clear();
                    break;
                }
            }
            else if (1 == twi->column())
            {
                name = twi->text();
            }
            else
            {
                params.push_back(twi->text());
            }
        }

        if (!name.isEmpty() &&
            GatType::Undefined != eType)
        {
            bool scheduleCommand = true;

            if (!params.empty())
            {
                GatCmdSpec dlg(this);
                dlg.setWindowModality(Qt::ApplicationModal);
                Q_ASSERT(GatType::Component == eType || GatType::File == eType);
                dlg.setType(GatType::Component == eType ? GatCmdSpec::TypeId::Component :
                            /*GatType::File == eType ? */GatCmdSpec::TypeId::GetFile);
                dlg.setTypeIsReadOnly(true);
                dlg.setName(name);
                dlg.setNameIsReadOnly(true);
                GatCmdSpec::params_type dlgParams;
                for (auto const &value : params) { dlgParams.push_back(std::make_pair(value, QString())); }
                dlg.setParameters(dlgParams);
                scheduleCommand = false;
                if (QDialog::Accepted == dlg.exec())
                {
                    params.clear();
                    for (auto const &dlgParam : dlg.parameters()) { params.push_back(dlgParam.second); }
                    scheduleCommand = true;
                }
            }

            if (scheduleCommand)
            {
                GatHost::gat_host_cmd_ptr_type newCmd(
                    GatType::Component == eType
                    ? static_cast<GatHostCmd *>(new getComponentCmd(*this, gatHost_, name, &params))
                    : GatType::File == eType
                      ? static_cast<GatHostCmd *>(new GetFileCmd(*this, gatHost_, name, &params))
                      : nullptr);
                if (!scheduleGatCommand(newCmd))
                {
                    QErrMsgBox("Failed to schedule " + (nullptr == newCmd.get()
                                                        ? QString("undefined GAT command")
                                                        : newCmd->gatSpecialFunctionName()) + ".",
                               this);
                }
            }
        }
    }
}


void
MainWindow::on_tableWidget_itemSelectionChanged()
{
    syncUiWidgets();
}


void
MainWindow::on_tableWidget_cellDoubleClicked(int /*row*/, int /*column*/)
{
    on_actionGetComponent_triggered();
}


void
MainWindow::on_btnIoLogClear_clicked()
{
    ui->edtIoLog->clear();
}


void
MainWindow::on_tabWidget_currentChanged(int /*tabIndex*/)
{
    if (ui->tabWidget->currentIndex() == logIndex)
    {
        // Scroll log edit widget to far (absolute) left.
        ui->edtLogText->horizontalScrollBar()->setSliderPosition(ui->edtLogText->horizontalScrollBar()->minimum());
    }
}


void
MainWindow::on_actionOpenSerialDevice_triggered()
{
    openCloseGatDevice();
}


void
MainWindow::onGatHostShutdown()
{
    scheduleGatCommand(nullptr);
    syncUiWidgets();
}


void
MainWindow::on_actionAboutApplication_triggered()
{
    AboutBox aboutBox(this);
    aboutBox.setWindowModality(Qt::ApplicationModal);
    aboutBox.exec();

    syncUiWidgets();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ioLogMsgSerialNumber_(0)
{
    ui->setupUi(this);

    // Subscribe to signals.
    connect(&gatHost_, SIGNAL(finished()), this, SLOT(onGatHostShutdown()));
    connect(&gatHost_, SIGNAL(started()), this, SLOT(onGatHostRunStateChange()));
    connect(&gatHost_, SIGNAL(finished()), this, SLOT(onGatHostRunStateChange()));
    //connect(&mGatHost, SIGNAL(terminated()), this, SLOT(OnGatHostRunStateChange()));
    connect(&gatHost_, SIGNAL(startupState(GatHost *, GatHostStartupStateId, QString const&)),
            this, SLOT(onGatHostStartupState(GatHost *, GatHostStartupStateId, QString const&)));
    connect(&gatHost_.gatLinkLayer(), SIGNAL(onTxPacket(GatLinkLayer *, QByteArray)),
            this, SLOT(onTxPacket(GatLinkLayer *, QByteArray)));
    connect(&gatHost_.gatLinkLayer(), SIGNAL(onReceiveData(GatLinkLayer *, QByteArray)),
            this, SLOT(onRxD(GatLinkLayer *, QByteArray)));
    connect(&gatHost_.gatLinkLayer(), SIGNAL(onReceivePacket(GatLinkLayer *, QByteArray, bool)),
            this, SLOT(onRxPacket(GatLinkLayer *, QByteArray, bool)));

    // Select a default serial port (if one is available and not already selected).
    if (ui->serialDeviceEdit->text().trimmed().isEmpty())
    {
        QList<QSerialPortInfo> serialDevices(QSerialPortInfo::availablePorts());
        if (!serialDevices.isEmpty())
        {
            ui->serialDeviceEdit->setText(serialDevices.front().systemLocation());
        }
    }

    // Prevent window from being resized.
    setFixedSize(size().width(), size().height());

    {// Get line height of I/O log widget.
        QFontMetrics fm(ui->edtIoLog->font());
        edtIoLogLineHeight_ = (std::max)(ui->edtIoLog->height() / fm.height() - 1/*margin*/, 0);
    }

    {// Get line height of log text widget.
        QFontMetrics fm(ui->edtLogText->font());
        edtLogTextLineHeight_ = (std::max)(ui->edtLogText->height() / fm.height() - 1/*margin*/, 0);
    }

    ui->openCloseButton->setFocus();

#if 0
    //ui->tabWidget->setCurrentIndex(0);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList("x") << "x" << "x");

    int vertScrollBarWidth = 20; //! \todo Need to find the Qt API for this.

    int colIdx = 1;
    char const *colName = "Param 1";
    QTableWidgetItem *col = ui->tableWidget->horizontalHeaderItem(colIdx);
    Q_ASSERT(nullptr != col);
    QFontMetrics fm(col->font());
    QSize size(fm.size(Qt::TextSingleLine, colName));
    //int margin = 8;
    //ui->tableWidget->setColumnWidth(colIdx, size.width() + margin);
    ui->tableWidget->setColumnWidth(colIdx, (ui->tableWidget->size().width() - vertScrollBarWidth) * 6 / 10);
    col->setText(colName);
    col->setTextAlignment(Qt::AlignLeft);

    colIdx = 2;
    colName = "Param 2";
    col = ui->tableWidget->horizontalHeaderItem(colIdx);
    Q_ASSERT(nullptr != col);
    fm = col->font();
    size = fm.size(Qt::TextSingleLine, colName);
    //margin = 8;
    //ui->tableWidget->setColumnWidth(colIdx, size.width() + margin);
    ui->tableWidget->setColumnWidth(colIdx, (ui->tableWidget->size().width() - vertScrollBarWidth) * 2 / 10);
    col->setText(colName);
    col->setTextAlignment(Qt::AlignLeft);

    colIdx = 0;
    colName = "Feature";
    col = ui->tableWidget->horizontalHeaderItem(colIdx);
    //ui->tableWidget->setColumnWidth(colIdx, <remaining_size>);
    ui->tableWidget->setColumnWidth(colIdx, (ui->tableWidget->size().width() - vertScrollBarWidth) * 2 / 10 - 2);
    col->setText(colName);
    col->setTextAlignment(Qt::AlignLeft);

// Test code.
QTableWidgetItem *twi = nullptr;
ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
ui->tableWidget->setItem(0, 0, (twi = new QTableWidgetItem("Get File")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
ui->tableWidget->setItem(0, 1, (twi = new QTableWidgetItem("AuthenticationResponse.xml")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
ui->tableWidget->setItem(0, 2, (twi = new QTableWidgetItem("%%SHA1_HMAC%%")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
ui->tableWidget->setItem(1, 0, (twi = new QTableWidgetItem("Component")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
ui->tableWidget->setItem(1, 1, (twi = new QTableWidgetItem("SHA1-Example")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
ui->tableWidget->setItem(1, 2, (twi = new QTableWidgetItem("")));
twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);

ui->tableWidget->sortByColumn(0, Qt::AscendingOrder);
ui->tableWidget->setSortingEnabled(true);
#endif // #if 0

    syncUiWidgets();
}


MainWindow::~MainWindow()
{
    scheduleGatCommand(nullptr);
    gatHost_.shutdown(true); // Failsafe.
    delete ui;
}


/*
    End of "MainWindow.cpp"
*/
