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


#ifndef GATCMDSPEC_HPP__A05C0DF0_B96A_4376_8FA0_B4CA45ED20F6__INCLUDED
#define GATCMDSPEC_HPP__A05C0DF0_B96A_4376_8FA0_B4CA45ED20F6__INCLUDED


#include <QDialog>


namespace Ui {
    class GatCmdSpec;
}


class GatCmdSpec : public QDialog
{
    Q_OBJECT

public:
    enum class TypeId : uint { GetFile, Component, Undefined };

    TypeId setType(TypeId const value);
    TypeId type() const { return type_; }
    bool setTypeIsReadOnly(bool const value);
    bool isTypeReadOnly() const { return typeIsReadOnly_; }

    QString const & setName(QString const &value);
    QString const & Name() const { return name_; }
    bool setNameIsReadOnly(bool const value);
    bool isNameReadOnly() const { return nameIsReadOnly_; }

    typedef std::pair<QString, QString> param_desc_type; // Name : Value
    typedef QList<param_desc_type> params_type;

    params_type const & setParameters(params_type const &value);
    params_type const & parameters() const { return parameters_; }

    explicit GatCmdSpec(QWidget *parent = 0);
    virtual ~GatCmdSpec();

protected:
    virtual void accept();

private:
    void initParametersView();
    void populateParametersView();

    GatCmdSpec(GatCmdSpec const&) = delete; //!< No cloning; leave unimplemented.
    GatCmdSpec& operator=(GatCmdSpec const&) = delete; //!< No cloning; leave unimplemented.

private:
    TypeId type_;
    bool typeIsReadOnly_;
    QString name_;
    bool nameIsReadOnly_;
    params_type parameters_;
    Ui::GatCmdSpec *ui;
};


#endif // #ifndef GATCMDSPEC_HPP__A05C0DF0_B96A_4376_8FA0_B4CA45ED20F6__INCLUDED


/*
    End of "GatCmdSpec.hpp"
*/
