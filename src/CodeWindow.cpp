// Copyright (C) 2010-2013 Michael Wiklund.
// All rights reserved.
// Contact: arnlib@wiklunden.se
//
// This file is part of the WebArnBrowser - Active Registry Network Browser.
// Parts of WebArnBrowser depend on Qt 4 and/or other libraries that have their own
// licenses. WebArnBrowser is independent of these licenses; however, use of these other
// libraries is subject to their respective license agreements.
//
// GNU Lesser General Public License Usage
// The dependent library Wt is not using LGPL.
// This file may be used under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation and
// appearing in the file LICENSE.LGPL included in the packaging of this file.
// In addition, as a special exception, you may use the rights described
// in the Nokia Qt LGPL Exception version 1.1, included in the file
// LGPL_EXCEPTION.txt in this package.
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public License version 3.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL included in the packaging of this file.
//
// Other Usage
// Alternatively, this file may be used in accordance with the terms and
// conditions contained in a signed written agreement between you and Michael Wiklund.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//

#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WTextEdit>
#include <Wt/WDialog>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WServer>
#include <wt/WCodeEdit.h>
#include <Wt/WBorder>

#include "CodeWindow.hpp"
#include "wqlib/WQApplication.hpp"
#include <ArnInc/ArnItem.hpp>
#include <ArnInc/Arn.hpp>
#include <QDebug>


using namespace Wt;


CodeWindow::CodeWindow( const QString& path, QObject* parent) :
    QObject( parent), WObject(0)
{
    QString  itemName = ArnM::itemName( path);

    _wt = new WDialog("Edit " + toWString( path));
    _wt->setModal(false);
    _wt->show();
    _wt->setResizable(true);
    _wt->setClosable(true);
    _wt->setMinimumSize(600, 400);

    _reLoadButton = new WPushButton("ReLoad");
    _reLoadButton->resize(80, 25);
    _saveButton = new WPushButton("Save");
    _saveButton->resize(80, 25);

    if (itemName.endsWith(".html")) {
        WTextEdit*  linRt = new WTextEdit;
        _lines = linRt;
        linRt->setExtraPlugins("searchreplace, table, nonbreaking, print");
        linRt->setToolBar(0, "fontselect, |, bold, italic, underline, strikethrough, |, fontsizeselect, |, "
                             "forecolor, backcolor, |, justifyleft, justifycenter, justifyright, justifyfull, |, "
                             "numlist, bullist, |, hr, charmap, nonbreaking, |, sub, sup");
        linRt->setToolBar(1, "cut, copy, paste, |, search, replace, |, outdent, indent, |, tablecontrols, |, "
                             "undo, redo, |, code, print");
    }
    else if (itemName.endsWith(".js")) {
        WCodeEdit*  ed = new WCodeEdit;
        _lines = ed;
        ed->setOptionSetting("indentUnit", 4);
        ed->setOptionSetting("autoClearEmptyLines", true);
        ed->setOptionSetting("lineNumbers", true);
        ed->setOptionSetting("fixedGutter", true);
        ed->setOptionSetting("matchBrackets", true);
        // ed->decorationStyle().setBorder( WBorder( WBorder::Solid, WBorder::Medium, Wt::black));
    }
    else {
        _lines = new WTextArea();
    }

    WHBoxLayout*  hlay1 = new WHBoxLayout;
    hlay1->addWidget( _reLoadButton);
    hlay1->addWidget( _saveButton);
    hlay1->addStretch(1);
    hlay1->setSpacing(10);
    hlay1->insertSpacing(2, 50);
    WHBoxLayout*  hlay2 = new WHBoxLayout;
    hlay2->addWidget( _lines);
    WVBoxLayout*  vlay = new WVBoxLayout;
    vlay->addLayout( hlay1);
    vlay->addLayout( hlay2, 1);
    // vlay->addWidget( _lines, 1);
    vlay->setSpacing(10);

    _wt->contents()->setPadding(0);
    _wt->resize(800, 600);
    _wt->contents()->setLayout( vlay);
    //_wt->refresh();
    _lines->setFocus(true);

    //// Logics
    _arnItem.open( path);
    _lines->setText( toWString( _arnItem.toString()));

    _reLoadButton->clicked().connect( this, &CodeWindow::onReLoadButtonClicked);
    _saveButton->clicked().connect( this, &CodeWindow::onSaveButtonClicked);
    _wt->finished().connect( this, &CodeWindow::doCloseWindow);
}


void CodeWindow::setApplication( WApplication* app)
{
    _app = app;
}


void  CodeWindow::onReLoadButtonClicked()
{
    _lines->setText( toWString( _arnItem.toString()));
}


void  CodeWindow::onSaveButtonClicked()
{
    _arnItem = toQString( _lines->text());
}


void  CodeWindow::doCloseWindow()
{
    delete _wt;
    _wt = 0;

    deleteLater();
}
