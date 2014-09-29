/*!
    \file "SelectSerialPortDlg.hpp"

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


#ifndef SELECTSERIALPORTDLG_HPP__EE073E0C_85A5_45D2_AF9C_437365C3C11D__INCLUDED
#define SELECTSERIALPORTDLG_HPP__EE073E0C_85A5_45D2_AF9C_437365C3C11D__INCLUDED


#include "Defs.hpp"
#include <QDialog>
#include <QModelIndex>


namespace Ui {
    class SelectSerialPortDlg;
}


class SelectSerialPortDlg
    : public QDialog
{
    Q_OBJECT

public:
    QString const& setSerialDevice(QString const& pathname);
    QString const& serialDevice() const { return serialDevicePathname_; }

    explicit SelectSerialPortDlg(QWidget *parent = 0);
    ~SelectSerialPortDlg();

protected:
    void syncUiWidgets();
    virtual void done(int resultCode);

private slots:
    void on_serialDeviceListWidget_itemSelectionChanged();
    void on_serialDeviceListWidget_doubleClicked(const QModelIndex &index);

private:
    SelectSerialPortDlg(SelectSerialPortDlg const&) = delete; //!< No cloning; leave unimplemented.
    SelectSerialPortDlg& operator=(SelectSerialPortDlg const&) = delete; //!< No cloning; leave unimplemented.

    QString serialDevicePathname_;
    Ui::SelectSerialPortDlg *ui;
};


#endif // #ifndef SELECTSERIALPORTDLG_HPP__EE073E0C_85A5_45D2_AF9C_437365C3C11D__INCLUDED


/*
    End of "SelectSerialPortDlg.hpp"
*/
