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

#include <iostream>

#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WGroupBox>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WLabel>
#include <Wt/WTreeView>
#include <Wt/WPanel>
#include <Wt/WDialog>
#include <Wt/WMessageBox>
#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"
#include "Wt/WHBoxLayout"
#include "Wt/WVBoxLayout"
#include "Wt/WScrollArea"
#include <Wt/WEnvironment>
#include <Wt/Auth/AuthWidget>

#include "MultiDelegate.hpp"
#include "MainWindow.hpp"
#include "TermWindow.hpp"
#include "CodeWindow.hpp"
#include "ManageWindow.hpp"
#include "VcsWindow.hpp"
#include <ArnLib/Arn.hpp>
#include <QThread>
#include <QMutex>
#include <QDebug>


using namespace Wt;

extern QByteArray  arnBrowseName;
extern QMutex  mainMutex;


MainWindowQ::MainWindowQ( MainWindowW* wt, QObject* parent)
    : QObject( parent)
{
    _wt = wt;
}


MainWindowW::MainWindowW( const WEnvironment& env)
    : WQApplication( env, true)
{
    // Note: do not create any Qt objects from here. Initialize your
    // application from within the virtual create() method.
}


void MainWindowW::create()
{
    enableUpdates();

    QByteArray  title = "Arn Browser";
    mainMutex.lock();
    if (!arnBrowseName.isEmpty())
        title += " - " + arnBrowseName;
    mainMutex.unlock();
    setTitle( title.constData());

    setCssTheme("polished");
    useStyleSheet("styles.css");

    //// Login
    _session = new Session;
    _session->login().changed().connect(this, &MainWindowW::onAuthEvent);

    Auth::AuthModel *authModel = new Auth::AuthModel(Session::auth(),
                                                     _session->users(), this);
    authModel->addPasswordAuth(&Session::passwordAuth());
    authModel->addOAuth(Session::oAuth());

    _authWidget = new Auth::AuthWidget(_session->login());
    _authWidget->setModel(authModel);
    // authWidget->setRegistrationEnabled(true);
    ////

    root()->addWidget( _headTxt = new WLabel(("<b>" + title + "</b>").constData()));
    _headTxt->setMargin(10, Left);

    root()->addWidget(_authWidget);

    _mainWidget = new WContainerWidget;
    //_mainWidget->setMaximumSize(900, WLength::Auto);
    setupArnView();

    _mainWidget->hide();

    _mainWindowQ = new MainWindowQ(this);

    _authWidget->processEnvironment();

    _curItemPath = "//RSNet/pipe";  // MW: Test
}


void MainWindowW::destroy()
{
    // Note: Delete any Qt object from here.

    delete _mainWindowQ;
}


void MainWindowW::setupArnView()
{
    _arnView = new WTreeView();

    WPanel *panel = new WPanel();
    //panel->resize(700, WLength::Auto);
    panel->setCentralWidget( _arnView);
    panel->setMinimumSize(725, 0);
    panel->setMaximumSize( panel->minimumWidth(), WLength::Auto);

    _model = new ArnModelW;
    _model->setApplication( this);

    _arnView->setRowHeight(20);
    _arnView->setAlternatingRowColors(true);
    _arnView->setSortingEnabled(false);
    _arnView->setSelectionMode(SingleSelection);
    _arnView->setEditTriggers( WAbstractItemView::DoubleClicked | WAbstractItemView::SelectedClicked);
    _arnView->setModel(_model);
    _arnView->setItemDelegate( new MultiDelegate);
    _arnView->setColumnWidth(0, WLength(190));
    _arnView->setColumnWidth(1, WLength(490));

    WContainerWidget*  wlc = new WContainerWidget;
    WPanel*  wplc = new WPanel;
    wplc->setMinimumSize(130, 0);
    wplc->setMaximumSize( wplc->minimumWidth(), WLength::Auto);
    wplc->setCentralWidget( wlc);
    wplc->setMargin(0);
    wlc->setPadding(0);
    wlc->setMargin(0);
    //wlc->setPadding(10, Right);
    _terminalButton = new WPushButton("Terminal");
    _terminalButton->setHeight(30);
    _terminalButton->setEnabled(false);
    _editButton = new WPushButton("Edit");
    _editButton->setHeight(30);
    _editButton->setEnabled(false);
    _manageButton = new WPushButton("Manage");
    _manageButton->setHeight(30);
    _manageButton->setMargin(10, Top);
    _manageButton->setEnabled(true);
    _vcsButton = new WPushButton("VCS");
    _vcsButton->setHeight(30);;
    _vcsButton->setEnabled(true);
    _helpButton = new WPushButton("Help");
    _helpButton->setHeight(30);
    _helpButton->setMargin(20, Top);
    _helpButton->setEnabled(false);

    WVBoxLayout*  butlay = new WVBoxLayout;
    butlay->addWidget( _terminalButton);
    butlay->addWidget( _editButton);
    butlay->addWidget( _manageButton);
    butlay->addWidget( _vcsButton);
    butlay->addWidget( _helpButton);
    butlay->addStretch(1);
    butlay->setSpacing(20);
    wlc->setLayout( butlay);

    WHBoxLayout*  hlay = new WHBoxLayout;
    //hlay->setSpacing(1);
    hlay->addWidget( wplc, 0);
    hlay->addSpacing(10);
    hlay->addWidget( panel, 1);
    hlay->addStretch();
    // hlay->setResizable(2, true);
    _mainWidget->setLayout( hlay);
    _mainWidget->setMaximumSize(900, WLength::Auto);

    _terminalButton->clicked().connect( this, &MainWindowW::onTerminalButtonClicked);
    _editButton->clicked().connect( this, &MainWindowW::onEditButtonClicked);
    _manageButton->clicked().connect( this, &MainWindowW::onManageButtonClicked);
    _vcsButton->clicked().connect( this, &MainWindowW::onVcsButtonClicked);
    _helpButton->clicked().connect( this, &MainWindowW::onHelpButtonClicked);
    _arnView->clicked().connect( this, &MainWindowW::onArnViewClicked);

    _model->hasIndex(0, 0, WModelIndex());  // Start the view by asking about root
}


void  MainWindowW::onTerminalButtonClicked()
{
    TermWindow*  termWindow = new TermWindow( _curItemPath, 0);
    termWindow->setApplication( this);
}


void  MainWindowW::onEditButtonClicked()
{
    CodeWindow*  codeWindow = new CodeWindow( _curItemPath, 0);
    codeWindow->setApplication( this);
}


void  MainWindowW::onManageButtonClicked()
{
    new ManageWindow( _curItemPath, 0);
}


void  MainWindowW::onVcsButtonClicked()
{
    new VcsWindow(0);
}


void  MainWindowW::onHelpButtonClicked()
{
    WDialog*  helpBox = new WDialog("Help");
    helpBox->setModal(false);
    helpBox->show();
    helpBox->setResizable(true);
    helpBox->setClosable(true);
    helpBox->setMinimumSize(50, 50);

    WText*  wHelpText = new WText( _curHelp);
    WScrollArea*  sca = new WScrollArea;
    sca->setScrollBarPolicy( WScrollArea::ScrollBarAsNeeded);
    sca->setWidget( wHelpText);

    WVBoxLayout*  vlay = new WVBoxLayout;
    vlay->addWidget( sca, 1);

    helpBox->contents()->setPadding(0);
    helpBox->resize(400, 400);
    helpBox->contents()->setLayout( vlay);
}


void  MainWindowW::onArnViewClicked( WModelIndex index)
{
    boost::any  path = _model->data( index, ItemDataRoleUser::Path);
    _curItemPath = boost::any_cast<QString>(path);

    // Set state for Help button and get current help text
    boost::any  help = _model->data( index, ItemDataRoleUser::Help);
    if (help.empty()) {
        _curHelp = "";
        _helpButton->setEnabled(false);
    }
    else {
        _curHelp = boost::any_cast<WString>(help);
        _helpButton->setEnabled(true);
    }

    ArnItem  arnItem( _curItemPath);
    ArnLink::Type  type = arnItem.type();

    // Set state for Terminal button
    _terminalButton->setEnabled( arnItem.isPipeMode());

    // Set state for Edit button
    bool editEn = !arnItem.isFolder() && ((type.e == type.Null)
                                      || (type.e == type.ByteArray)
                                      || (type.e == type.String));
    _editButton->setEnabled( editEn);
}


void  MainWindowW::onAuthEvent()
{
    if (_session->login().loggedIn()) {
        _mainWidget->show();
        WHBoxLayout*  hlay = new WHBoxLayout;
        hlay->addWidget( _headTxt);
        hlay->addWidget( _authWidget);
        WVBoxLayout*  vlay = new WVBoxLayout;
        vlay->addLayout( hlay);
        vlay->addWidget( _mainWidget, 1);
        vlay->setSpacing(15);
        root()->setLayout( vlay);
    }
    else {
        logout();
    }
}


void MainWindowW::logout()
{
    redirect("/logout");
    quit();
}
