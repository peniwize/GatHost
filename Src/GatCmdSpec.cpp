/*!
    \file "GatCmdSpec.hpp"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    GAT special function editor modal form.

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#include "Defs.hpp"
#include "GatCmdSpec.hpp"
#include "ui_GatCmdSpec.h"
#include "Defs.hpp"
#include <QDebug>
#include <QScrollBar>


GatCmdSpec::TypeId
GatCmdSpec::setType(TypeId const value)
{
    if (value != type_)
    {
        type_ = value;
        ui->rdoComponent->setChecked(TypeId::Component == value);
        ui->rdoGetFile->setChecked(TypeId::GetFile == value);
        ui->lblTypeName->setText((TypeId::Component == type_ ? ui->rdoComponent : ui->rdoGetFile)->text());
    }

    return type_;
}


bool
GatCmdSpec::setTypeIsReadOnly(bool const value)
{
    if (value != typeIsReadOnly_)
    {
        typeIsReadOnly_ = value;

        // Set the 'name' edit control background color.
        QColor color(palette().color(QPalette::ColorGroup::Active,
                                     typeIsReadOnly_ ? QPalette::ColorRole::Window : QPalette::ColorRole::Base));
        ui->rdoGetFile->setStyleSheet(QString("background-color: ") + color.name());
        ui->rdoGetFile->setEnabled(!typeIsReadOnly_);
        ui->rdoGetFile->setVisible(!typeIsReadOnly_);

        ui->rdoComponent->setStyleSheet(QString("background-color: ") + color.name());
        ui->rdoComponent->setEnabled(!typeIsReadOnly_);
        ui->rdoComponent->setVisible(!typeIsReadOnly_);

        ui->lblTypeName->setVisible(typeIsReadOnly_);
    }

    return typeIsReadOnly_;
}


QString const &
GatCmdSpec::setName(QString const &value)
{
    if (value != name_)
    {
        name_ = value;
        ui->edtName->setText(name_);
    }

    return name_;
}


bool
GatCmdSpec::setNameIsReadOnly(bool const value)
{
    if (value != nameIsReadOnly_)
    {
        nameIsReadOnly_ = value;

        // Set the 'name' edit control background color.
        QColor color(palette().color(QPalette::ColorGroup::Active,
                                     nameIsReadOnly_ ? QPalette::ColorRole::Window : QPalette::ColorRole::Base));
        ui->edtName->setStyleSheet(QString("background-color: ") + color.name());
        ui->edtName->setReadOnly(nameIsReadOnly_);
    }

    return nameIsReadOnly_;
}


GatCmdSpec::params_type const &
GatCmdSpec::setParameters(params_type const &value)
{
    int idx = 0;
    if (value.size() != parameters_.size())
    {
        for (idx = 0; parameters_.size() > idx; ++idx)
        {
            if (value[idx].first != parameters_[idx].first ||
                value[idx].second != parameters_[idx].second)
            {
                break;
            }
        }
    }
    if (parameters_.size() > idx || value.size() != parameters_.size())
    {
        parameters_ = value;

        initParametersView();
        populateParametersView();
    }

    return parameters_;
}


void
GatCmdSpec::accept()
{
    QDialog::accept();

    // NOTE: Use 'closeEvent(QCloseEvent *)' to implement validation, if needed.

    type_ = ui->rdoComponent->isChecked() ? TypeId::Component : TypeId::GetFile;
    name_ = ui->edtName->text();
    parameters_.clear();
    for (int rowIdx = 0; ui->tblParameters->rowCount() > rowIdx; ++rowIdx)
    {
        param_desc_type paramDesc;

        for (int colIdx = 0; ui->tblParameters->columnCount() > colIdx; ++colIdx)
        {
            QTableWidgetItem *twi = ui->tblParameters->item(rowIdx, colIdx);
            if (nullptr != twi)
            {
                if (0 == colIdx) { paramDesc.first = twi->text(); }
                if (1 == colIdx) { paramDesc.second = twi->text(); }
            }
        }

        parameters_.push_back(paramDesc);
    }
}


void
GatCmdSpec::initParametersView()
{
    if (parameters().empty())
    {
        ui->tblParameters->clear();
        ui->tblParameters->setRowCount(0);
        ui->tblParameters->setColumnCount(0);
        return;
    }

    Qt::ScrollBarPolicy const vertScrollBarPolicy = ui->tblParameters->verticalScrollBarPolicy();
    ui->tblParameters->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
    ui->tblParameters->updateGeometry();

    ui->tblParameters->setRowCount(0);
    uint const columnCount = parameters().empty() ? 0 : 2;
    ui->tblParameters->setColumnCount(columnCount);
    QStringList hdrs;
    for (size_t idx = 0; columnCount > idx; ++idx) { hdrs.push_back("x"); }
    ui->tblParameters->setHorizontalHeaderLabels(hdrs);
    ui->tblParameters->horizontalHeader()->setVisible(0 < columnCount);

    if (0 < columnCount)
    {
        ui->tblParameters->setRowCount(0);
        ui->tblParameters->setRowCount(1);
        ui->tblParameters->setItem(0, 0, new QTableWidgetItem("x"));
    }

    int colMargin = 10;
    int dynColIdx = 1; // Column index os column that will take up all extra space.

    // Calculate the text width of each column.
    std::vector<int> colWidths;
    int dynColWidth = 0;
    int totalWidth = 0;
    for (auto const &paramDef : parameters())
    {
        int totalParamDefWidth = 0;
        for (size_t idx = 0; 2 > idx; ++idx)
        {
            QFontMetrics fm(ui->tblParameters->item(0, 0)->font());

            std::fill_n(std::back_inserter(colWidths), colWidths.size() < idx ? idx - colWidths.size() : 0, 0);

            int const stringWidth =
                (std::max)(fm.size(Qt::TextSingleLine, 0 == idx ? paramDef.first : paramDef.second).width() + colMargin,
                           fm.size(Qt::TextSingleLine, 0 == idx ? "Parameter" : "Value").width() + colMargin);
            if (colWidths.size() > idx)
            {
                colWidths[idx] = (std::max)(colWidths[idx], stringWidth);
            }
            else
            {
                Q_ASSERT(colWidths.size() == idx);
                colWidths.push_back(stringWidth);
            }
            totalParamDefWidth += colWidths[idx];
            if (static_cast<size_t>(dynColIdx) == idx) { dynColWidth = (std::max)(dynColWidth, colWidths[idx]); }
        }

        totalWidth = (std::max)(totalWidth, totalParamDefWidth);
    }
    int const vsbWidth = ui->tblParameters->verticalScrollBar()->width();
    int const tableWidgetWidth = ui->tblParameters->size().width() - vsbWidth - 2;
    if (tableWidgetWidth > totalWidth &&
        colWidths.size() > static_cast<size_t>(dynColIdx))
    {
        dynColWidth += tableWidgetWidth - totalWidth;
        colWidths[dynColIdx] = dynColWidth;
    }
    Q_ASSERT(colWidths.size() == columnCount);

    // Populate column headers.
    for (size_t idx = 0; colWidths.size() > idx; ++idx)
    {
        ui->tblParameters->setColumnWidth(idx, colWidths[idx]);

        QTableWidgetItem *col = ui->tblParameters->horizontalHeaderItem(idx);
        Q_ASSERT(nullptr != col);
        col->setText(0 == idx ? "Parameter" : "Value");
        col->setTextAlignment(Qt::AlignLeft);
    }

    ui->tblParameters->setVerticalScrollBarPolicy(vertScrollBarPolicy);
}


void
GatCmdSpec::populateParametersView()
{
    ui->tblParameters->setSortingEnabled(false);

    ui->tblParameters->setRowCount(0);
    Q_ASSERT(0 == static_cast<size_t>(ui->tblParameters->rowCount()));

    for (size_t rowIdx = 0; static_cast<size_t>(parameters().size()) > rowIdx; ++rowIdx)
    {
        auto const &paramDesc(parameters()[rowIdx]);
        ui->tblParameters->setRowCount(ui->tblParameters->rowCount() + 1);
        for (size_t colIdx = 0; 2 > colIdx; ++colIdx)
        {
            ui->tblParameters->setColumnCount((std::max)(static_cast<size_t>(ui->tblParameters->columnCount()), colIdx + 1));

            QTableWidgetItem *twi = nullptr;
            Q_ASSERT(static_cast<size_t>(ui->tblParameters->rowCount()) > rowIdx);
            Q_ASSERT(static_cast<size_t>(ui->tblParameters->columnCount()) > colIdx);
            QString value(0 == colIdx ? paramDesc.first :
                           !paramDesc.second.isEmpty() ? paramDesc.second :
                           QString("(none)"));
            ui->tblParameters->setItem(rowIdx, colIdx, (twi = new QTableWidgetItem(value)));
            twi->setFlags(0 == colIdx ? twi->flags() & ~Qt::ItemIsEditable : twi->flags() | Qt::ItemIsEditable);
        }
    }
}


GatCmdSpec::GatCmdSpec(QWidget *parent)
    : QDialog(parent)
    , type_(TypeId::Undefined)
    , typeIsReadOnly_(true)
    , nameIsReadOnly_(true)
    , ui(new Ui::GatCmdSpec)
{
    ui->setupUi(this);
    setTypeIsReadOnly(false);
    setNameIsReadOnly(false);
    initParametersView();
    populateParametersView();
    setFixedSize(size().width(), size().height()); // Prevent window from being resized.
}


GatCmdSpec::~GatCmdSpec()
{
    delete ui;
}


/*
    End of "GatCmdSpec.hpp"
*/
