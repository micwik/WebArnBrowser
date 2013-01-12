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
#include <Wt/WSuggestionPopup>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WDialog>
#include "Wt/WHBoxLayout"
#include "Wt/WVBoxLayout"
#include "Wt/WServer"

#include "TermWindow.hpp"
#include "wqlib/WQApplication.hpp"
#include <ArnLib/ArnItem.hpp>
#include <ArnLib/Arn.hpp>
#include <QDebug>


using namespace Wt;


TermWindow::TermWindow( const QString& path, QObject* parent) :
    QObject( parent), WObject(0)
{
    _linesNeedUpdate = false;

    _wt = new WDialog("Terminal");
    _wt->setModal(false);
    _wt->show();
    _wt->setResizable(true);
    _wt->setClosable(true);
    _wt->setMinimumSize(150, 150);

    WText*  pathText = new WText("pipe path: ");
    pathText->setWordWrap(false);
    WLineEdit*  pathView = new WLineEdit;
    pathView->setReadOnly(true);
    pathView->setHeight(20);

    _linesView = new WTextArea();
    _linesView->setReadOnly(true);

    _inputLine = new WLineEdit();
    _inputLine->setEmptyText("Input to provider");
    _inputLine->setHeight(20);

    _inputLine2 = new WLineEdit();
    _inputLine2->setEmptyText("Input to requester");
    _inputLine2->setHeight(20);

    WSuggestionPopup::Options  lineEditOptions
    = { "<b>",       // highlightBeginTag
        "</b>",      // highlightEndTag
        '\0',        // listSeparator
        " ",         // whitespace
        "-.,= \";",  // wordSeparators
        ""           // appendReplacedText
    };

    _inputLineSug = new Wt::WSuggestionPopup( lineEditOptions);
    _inputLineSug->forEdit( _inputLine, WSuggestionPopup::DropDownIcon);

    _inputLine2Sug = new Wt::WSuggestionPopup( lineEditOptions);
    _inputLine2Sug->forEdit( _inputLine2, WSuggestionPopup::DropDownIcon);

    WHBoxLayout*  hlay = new WHBoxLayout;
    hlay->addWidget( pathText);
    hlay->addWidget( pathView, 1);
    hlay->setSpacing(10);
    WVBoxLayout*  vlay = new WVBoxLayout;
    vlay->addLayout( hlay);
    vlay->addWidget( _linesView, 1);
    vlay->addWidget( _inputLineSug);
    vlay->addWidget( _inputLine);
    vlay->addWidget( _inputLine2Sug);
    vlay->addWidget( _inputLine2);
    vlay->setSpacing(10);

    _wt->contents()->setPadding(0);
    _wt->resize(350, 450);
    _wt->contents()->setLayout( vlay);
    _inputLine->setFocus(true);  // MW: Doesn't work ...

    //// Logics
    QString  twinPath = Arn::twinPath( path);
    // Always pipe is client side and pipe2 is provider side
    if (!Arn::isProviderPath( path)) {  // path is client side (normal)
        _pipe.open( path);
        _pipe2.open( twinPath);
    }
    else {
        _pipe2.open( path);
        _pipe.open( twinPath);
    }
    pathView->setText( toWString( _pipe.path()));

    connect( &_pipe, SIGNAL(changed(QString)), this, SLOT(doPipeInput(QString)));
    connect( &_pipe2, SIGNAL(changed(QString)), this, SLOT(doPipe2Input(QString)));

    _inputLine->enterPressed().connect( this, &TermWindow::onLineEnter);
    _inputLine2->enterPressed().connect( this, &TermWindow::onLine2Enter);
    _wt->finished().connect( this, &TermWindow::doCloseWindow);
}


void TermWindow::setApplication( WApplication* app)
{
    _app = app;
}


void  TermWindow::updateLines()
{
    qDebug() << "+++ updateLines:";
    _linesView->setText( toWString( _lines));
    _linesNeedUpdate = false;

    // Little javascript trick to make sure we scroll along with new content
    WApplication::instance()->doJavaScript( _linesView->jsRef() + ".scrollTop += "
                       + _linesView->jsRef() + ".scrollHeight;");

    _app->triggerUpdate();
}


void  TermWindow::appendLine( QString line)
{
    _lines += "\n" + line;
}


void  TermWindow::onLineEnter()
{
    WString  lineW = _inputLine->text();
    QString  line  = toQString( lineW);
    _pipe = line;
    _inputLine->setText("");

    if (!_history.contains( line)) {
        _history += line;
        _inputLineSug->addSuggestion( lineW, lineW);
    }
}


void  TermWindow::onLine2Enter()
{
    WString  lineW = _inputLine2->text();
    QString  line  = toQString( lineW);
    _pipe2 = line;
    _inputLine2->setText("");

    if (!_history2.contains( line)) {
        _history2 += line;
        _inputLine2Sug->addSuggestion( lineW, lineW);
    }
}


void  TermWindow::doCloseWindow()
{
    delete _wt;
    _wt = 0;

    deleteLater();
}


void  TermWindow::doPipeInput( QString text)
{
    qDebug() << "+++ doPipeInput: val=" << text;
    appendLine( text);

    if (!_linesNeedUpdate) {
        _linesNeedUpdate = true;
        WServer::instance()->post( _app->sessionId(),
                                  boost::bind( &TermWindow::updateLines, this));
    }
}


void  TermWindow::doPipe2Input( QString text)
{
    qDebug() << "+++ doPipe2Input: val=" << text;
    appendLine( "==> " + text);

    if (!_linesNeedUpdate) {
        _linesNeedUpdate = true;
        WServer::instance()->post( _app->sessionId(),
                                  boost::bind( &TermWindow::updateLines, this));
    }
}
