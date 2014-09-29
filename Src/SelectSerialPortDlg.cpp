/*!
    \file "SelectSerialPortDlg.cpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Serial port selection dialog box.

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#include "SelectSerialPortDlg.hpp"
#include "ui_SelectSerialPortDlg.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>


QString const&
SelectSerialPortDlg::setSerialDevice(QString const& pathname)
{
    if (pathname != serialDevicePathname_)
    {
        serialDevicePathname_ = pathname;

        // Search for and select serial device in list.
        QList<QListWidgetItem *> items = ui->serialDeviceListWidget->findItems(pathname, Qt::MatchExactly);
        if (!items.empty())
        {
            ui->serialDeviceListWidget->setCurrentItem(items.front(), QItemSelectionModel::SelectCurrent);
        }
    }

    return serialDevicePathname_;
}


void
SelectSerialPortDlg::syncUiWidgets()
{
    QPushButton *pBtn = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (nullptr != pBtn) { pBtn->setEnabled(ui->serialDeviceListWidget->currentIndex().isValid()); }
}


void
SelectSerialPortDlg::done(int resultCode)
{
    if (QDialog::Accepted == resultCode)
    {
        QListWidgetItem *pItem = ui->serialDeviceListWidget->currentItem();
        if (nullptr != pItem)
        {
            serialDevicePathname_ = pItem->text();
        }
    }

    QDialog::done(resultCode);
}


SelectSerialPortDlg::SelectSerialPortDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectSerialPortDlg)
{
    ui->setupUi(this);

    // Add all serial devices to list UI.
    QList<QSerialPortInfo> devices(QSerialPortInfo::availablePorts());
    for (auto iter = devices.begin(); devices.end() != iter; ++iter)
    {
        ui->serialDeviceListWidget->addItem(iter->systemLocation());
    }

    // Prevent window from being resized.
    setFixedSize(size().width(), size().height());

    syncUiWidgets();
}


SelectSerialPortDlg::~SelectSerialPortDlg()
{
    delete ui;
}


void
SelectSerialPortDlg::on_serialDeviceListWidget_itemSelectionChanged()
{
    syncUiWidgets();
}


void
SelectSerialPortDlg::on_serialDeviceListWidget_doubleClicked(const QModelIndex & /*index*/)
{
    accept();
}


/*
    End of "SelectSerialPortDlg.cpp"
*/
