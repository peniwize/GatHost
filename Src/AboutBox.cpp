/*!
    \file "AboutBox.cpp"

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


#include "Defs.hpp"
#include "AboutBox.hpp"
#include "ui_AboutBox.h"
#include "Defs.hpp"
#include <QDebug>


AboutBox::AboutBox(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutBox)
{
    ui->setupUi(this);

    this->setWindowTitle(applicationName);

    // Set edit control background to the same color as the window.
    QColor color(palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Window));
    ui->textEdit->setStyleSheet(QString("background-color: ") + color.name());

    // Set the edit control text.
    QString text(ui->textEdit->toHtml());
    char const *versionTag = "${version}";
    int tagIdx = text.indexOf(versionTag);
    if (-1 != tagIdx)
    {
        uint majorVersion = productMavorVersion;
        uint minorVersion = productMinorVersion;
        text.replace(tagIdx, strlen(versionTag),
                     QString::number(majorVersion) + "." + QString::number(minorVersion) + "." + productBuildString);
        ui->textEdit->setHtml(text);
    }

    // Prevent window from being resized.
    setFixedSize(size().width(), size().height());
}


AboutBox::~AboutBox()
{
    delete ui;
}


/*
    End of "AboutBox.cpp"
*/
