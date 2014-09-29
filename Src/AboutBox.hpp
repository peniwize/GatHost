/*!
    \file "AboutBox.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    Application about dialog box.

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef ABOUTBOX_HPP__5EA0467F_0AA2_4D88_B8C5_FE17EC842ED9__INCLUDED
#define ABOUTBOX_HPP__5EA0467F_0AA2_4D88_B8C5_FE17EC842ED9__INCLUDED


#include <QDialog>


namespace Ui {
    class AboutBox;
}


class AboutBox
    : public QDialog
{
    Q_OBJECT

public:
    explicit AboutBox(QWidget *parent = 0);
    ~AboutBox();

private:
    AboutBox(AboutBox const&) = delete; //!< No cloning; leave unimplemented.
    AboutBox& operator=(AboutBox const&) = delete; //!< No cloning; leave unimplemented.

    Ui::AboutBox *ui;
};


#endif // #ifndef ABOUTBOX_HPP__5EA0467F_0AA2_4D88_B8C5_FE17EC842ED9__INCLUDED


/*
    End of "AboutBox.hpp"
*/
