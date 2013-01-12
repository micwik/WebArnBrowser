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

#ifndef VCSWINDOW_HPP
#define VCSWINDOW_HPP

#include <Wt/WObject>
#include <ArnLib/MQFlags.hpp>
#include <ArnLib/ArnItem.hpp>
#include <ArnLib/ArnPersistSapi.hpp>
#include <QStringList>
#include <QObject>

using namespace Wt;

namespace Wt {
    class WLineEdit;
    class WText;
    class WTextEdit;
    class WTextArea;
    class WComboBox;
    class WSelectionBox;
    class WButtonGroup;
    class WGroupBox;
    class WDialog;
    class WContainerWidget;
}


class VcsWindow : public QObject, public WObject
{
public:
Q_OBJECT
public:
    struct Sel {
        enum E {
            WorkTree = 0x01,
            RepoTree = 0x02,
            RefId    = 0x04,
            RefTag   = 0x08
        };
        MQ_DECLARE_FLAGS( Sel)
    };

    explicit VcsWindow( QObject *parent = 0);

    void  onReLoadButtonClicked();
    void  onCommitButtonClicked();
    void  onCheckoutButtonClicked();
    void  onTagButtonClicked();
    void  onDiffButtonClicked();
    void  onStatusButtonClicked();
    void  onTreeChanged();
    void  onRefTypeChanged();
    void  onRefChanged( WString refTxt);

    void  doCloseWindow();
    void  doCloseTagWindow( WDialog* tagDialog, WLineEdit* tagEdit);
    void  doCloseCommitWindow( WDialog* commitDialog, WTextArea* msgEdit);

signals:

private slots:
    void  sapiVcsNotify( QString msg);
    void  sapiVcsProgress( int percent, QString msg=QString());
    void  sapiVcsUserSettingsR( QString name, QString eMail);
    void  sapiVcsFilesR( QStringList files);
    void  sapiVcsTagR();
    void  sapiVcsCommitR();
    void  sapiVcsCheckoutR();
    void  sapiVcsBranchesR( QStringList branches);
    void  sapiVcsTagsR( QStringList tags);
    void  sapiVcsStatusR( QString status);
    void  sapiVcsDiffR( QString txt);
    void  sapiVcsLogRecordR( QString txt);
    void  sapiVcsLogR( QStringList refs, QStringList msgs);
    void  sapiLsR( QStringList files);

private:
    void  appendText( WString text);
    void  doUpdate();
    QString  getRef();
    QStringList  getSelFiles();

    WApplication*  _app;
    WDialog*  _wt;
    WPushButton*  _reLoadButton;
    WPushButton*  _commitButton;
    WPushButton*  _checkoutButton;
    WPushButton*  _tagButton;
    WPushButton*  _diffButton;
    WPushButton*  _statusButton;
    WButtonGroup*  _treeSel;
    WButtonGroup*  _refTypeSel;
    WGroupBox*  _refTypeBox;
    WComboBox*  _refSel;
    WSelectionBox*  _filesView;
    WTextArea*  _textView;

    ArnPersistSapi  _sapiVcs;
    QStringList  _refIdList;
    QStringList  _refIdMsgList;
    QStringList  _refTagList;
    QStringList  _fileList;
    bool  _filesUpdated;
    bool  _refUpdated;
};

MQ_DECLARE_OPERATORS_FOR_FLAGS( VcsWindow::Sel)

#endif // VCSWINDOW_HPP
