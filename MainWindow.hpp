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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "Session.hpp"
#include "wqlib/WQApplication.hpp"
#include "ArnModel.hpp"

// #define STDITEMTEST

class MainWindowW;


using namespace Wt;

namespace Wt {
    class WLineEdit;
    class WText;
    class WLabel;
    class WPushButton;
    class WTreeView;
    class WContainerWidget;

    namespace Auth {
        class AuthWidget;
    }
}



class MainWindowQ : public QObject
{
Q_OBJECT;
public:
    MainWindowQ( MainWindowW* wt, QObject* parent = 0);

signals:

public slots:

private:
    MainWindowW*  _wt;
};


class MainWindowW : public WQApplication
{
    friend class MainWindowQ;
public:
    MainWindowW( const WEnvironment& env);

    virtual void  create();
    virtual void  destroy();

private:
    void setupArnView();
    void logout();
    void onAuthEvent();
    void onTerminalButtonClicked();
    void onEditButtonClicked();
    void onVcsButtonClicked();
    void onManageButtonClicked();
    void onHelpButtonClicked();
    void onArnViewClicked( WModelIndex index);

    QString  _curItemPath;
    WString  _curHelp;

    WPushButton*  _terminalButton;
    WPushButton*  _editButton;
    WPushButton*  _manageButton;
    WPushButton*  _vcsButton;
    WPushButton*  _helpButton;
    WLabel*  _headTxt;
    Auth::AuthWidget*  _authWidget;
    WContainerWidget*  _mainWidget;
    WTreeView*  _arnView;

#ifdef STDITEMTEST
    WStandardItemModel*  _model;
#else
    ArnModelW*  _model;
#endif

    MainWindowQ*  _mainWindowQ;

    Session* _session;
};


#endif // MAINWINDOW_HPP
