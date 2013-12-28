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
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WGroupBox>
#include <Wt/WRadioButton>
#include <Wt/WButtonGroup>
#include <Wt/WComboBox>
#include <Wt/WTreeView>
#include <Wt/WSelectionBox>
#include <Wt/WTextArea>
#include <Wt/WDialog>
#include "Wt/WHBoxLayout"
#include "Wt/WVBoxLayout"
#include "Wt/WServer"
#include "Wt/WRegExpValidator"

#include "VcsWindow.hpp"
#include "wqlib/WQApplication.hpp"
#include <ArnInc/Arn.hpp>
#include <QDebug>


using namespace Wt;


VcsWindow::VcsWindow( QObject* parent) :
    QObject( parent), WObject(0)
{
    _app          = WApplication::instance();
    _filesUpdated = false;
    _refUpdated   = false;

    _wt = new WDialog("Version Control System");
    _wt->setModal(false);
    _wt->show();
    _wt->setResizable(true);
    _wt->setClosable(true);
    _wt->setMinimumSize(300, 200);

    _reLoadButton = new WPushButton("Refresh Files");
    _reLoadButton->resize(100, 25);
    _commitButton = new WPushButton("Commit");
    _commitButton->resize(100, 25);
    _checkoutButton = new WPushButton("Checkout");
    _checkoutButton->resize(100, 25);
    _tagButton = new WPushButton("Tag");
    _tagButton->resize(100, 25);
    _diffButton = new WPushButton("Diff");
    _diffButton->resize(100, 25);
    _statusButton = new WPushButton("Status");
    _statusButton->resize(100, 25);

    Wt::WRadioButton *button;
    _treeSel = new WButtonGroup( this);
    WGroupBox*  treeBox = new WGroupBox("Tree select");
    treeBox->setMinimumSize(120, WLength::Auto);
    button = new Wt::WRadioButton("Work", treeBox);
    _treeSel->addButton( button, Sel::WorkTree);
    new Wt::WBreak( treeBox);
    button = new Wt::WRadioButton("Repo", treeBox);
    _treeSel->addButton( button, Sel::RepoTree);
    _treeSel->setCheckedButton( _treeSel->button( Sel::WorkTree));

    _refTypeSel = new WButtonGroup( this);
    _refTypeBox = new WGroupBox("Ref. select");
    button = new Wt::WRadioButton("Id", _refTypeBox);
    _refTypeSel->addButton( button, Sel::RefId);
    button = new Wt::WRadioButton("Tag", _refTypeBox);
    button->setMargin(10);
    _refTypeSel->addButton( button, Sel::RefTag);
    new Wt::WBreak( _refTypeBox);
    _refSel = new WComboBox( _refTypeBox);

    _filesView = new WSelectionBox;
    _filesView->setSelectionMode( Wt::ExtendedSelection);
    _textView = new WTextArea;
    _textView->setReadOnly(true);

    WVBoxLayout*  butlay = new WVBoxLayout;
    butlay->addWidget( _reLoadButton);
    butlay->addWidget( _commitButton);
    butlay->addWidget( _checkoutButton);
    butlay->addWidget( _tagButton);
    butlay->addWidget( _diffButton);
    butlay->addWidget( _statusButton);
    butlay->addStretch(1);
    butlay->setSpacing(20);
    WHBoxLayout*  hlay1 = new WHBoxLayout;
    hlay1->addWidget( treeBox);
    hlay1->addWidget( _refTypeBox, 1);
    hlay1->setSpacing(8);
    WHBoxLayout*  hlay2 = new WHBoxLayout;
    hlay2->addWidget( _filesView, 4);
    hlay2->addWidget( _textView, 6);
    hlay2->setResizable(0, true);
    WVBoxLayout*  vlay1 = new WVBoxLayout;
    vlay1->addLayout( hlay1);
    vlay1->addLayout( hlay2, 1);
    vlay1->setSpacing(12);
    WHBoxLayout*  toplay = new WHBoxLayout;
    toplay->addLayout( butlay);
    toplay->addLayout( vlay1, 1);
    toplay->setSpacing(20);

    _wt->contents()->setPadding(8);
    _wt->resize(900, 600);
    _wt->contents()->setLayout( toplay);

    //// Logics
    _sapiVcs.open("//.sys/Persist/Pipes/CommonPipe");
    _sapiVcs.batchConnect( QRegExp("^rq_(.+)"), this, "sapi\\1");

    _reLoadButton->clicked().connect( this, &VcsWindow::onReLoadButtonClicked);
    _commitButton->clicked().connect( this, &VcsWindow::onCommitButtonClicked);
    _checkoutButton->clicked().connect( this, &VcsWindow::onCheckoutButtonClicked);
    _tagButton->clicked().connect( this, &VcsWindow::onTagButtonClicked);
    _diffButton->clicked().connect( this, &VcsWindow::onDiffButtonClicked);
    _statusButton->clicked().connect( this, &VcsWindow::onStatusButtonClicked);
    _treeSel->checkedChanged().connect( this, &VcsWindow::onTreeChanged);
    _refTypeSel->checkedChanged().connect( this, &VcsWindow::onRefTypeChanged);
    _refSel->sactivated().connect( this, &VcsWindow::onRefChanged);
    _wt->finished().connect( this, &VcsWindow::doCloseWindow);

    doUpdate();
}


void  VcsWindow::appendText( WString text)
{
    _textView->setText( _textView->text() + "\n" + text);

    _app->triggerUpdate();
}


void  VcsWindow::doUpdate()
{
    _commitButton->setDisabled( _treeSel->checkedId() != Sel::WorkTree);
    _checkoutButton->setDisabled( _treeSel->checkedId() != Sel::RepoTree);
    _diffButton->setDisabled( _treeSel->checkedId() != Sel::RepoTree);
    _refTypeBox->setDisabled( _treeSel->checkedId() != Sel::RepoTree);
    _statusButton->setDisabled( _treeSel->checkedId() != Sel::WorkTree);

    if (_refUpdated) {
        _refUpdated  = false;
        _refSel->clear();
        if (_refTypeSel->checkedId() == Sel::RefId) {
            foreach (QString msg, _refIdMsgList) {
                _refSel->addItem( toWString( msg));
            }
        }
        else if (_refTypeSel->checkedId() == Sel::RefTag) {
            foreach (QString tag, _refTagList) {
                _refSel->addItem( toWString( tag));
            }
        }
        _app->triggerUpdate();
    }

    if (_filesUpdated) {
        _filesUpdated  = false;
        _filesView->clear();
        foreach (QString file, _fileList) {
            _filesView->addItem( toWString( file));
        }
        _app->triggerUpdate();
    }
}


QString  VcsWindow::getRef()
{
    if (_treeSel->checkedId() != Sel::RepoTree)  return QString();

    int  index = _refSel->currentIndex();
    if (index < 0)  return QString();

    QString  ref;
    if (_refTypeSel->checkedId() == Sel::RefId) {
        ref = _refIdList.at( index);
    }
    else if (_refTypeSel->checkedId() == Sel::RefTag) {
        ref = _refTagList.at( index);
    }
    return ref;
}


QStringList  VcsWindow::getSelFiles()
{
    const std::set<int>&  list = _filesView->selectedIndexes();
    QStringList  selFiles;
    foreach (int index, list) {
        selFiles += _fileList.at( index);
    }
    return selFiles;
}


void  VcsWindow::onReLoadButtonClicked()
{
    _textView->setText("");
    _filesView->clearSelection();
    _filesView->clear();

    if (_treeSel->checkedId() == Sel::WorkTree) {
        emit _sapiVcs.pv_ls();
    }
    else if (_treeSel->checkedId() == Sel::RepoTree) {
        emit _sapiVcs.pv_vcsFiles( getRef());
    }
}


void  VcsWindow::onCommitButtonClicked()
{
    WDialog* dia;
    dia = new WDialog("Commiting files");
    dia->setModal(true);
    dia->show();
    dia->setResizable(false);
    dia->setClosable(true);
    dia->rejectWhenEscapePressed();

    WLabel*  msgLabel = new WLabel("Commit message");
    WTextArea*  msgEdit = new WTextArea;
    msgLabel->setBuddy( msgEdit);
    WPushButton*  cancelButton = new WPushButton("Cancel");
    cancelButton->resize(80, 25);
    WPushButton*  okButton = new WPushButton("Ok");
    okButton->resize(80, 25);

    WHBoxLayout*  butlay = new WHBoxLayout;
    butlay->addWidget( cancelButton);
    butlay->addWidget( okButton);
    butlay->addStretch(1);
    butlay->setSpacing(20);
    WVBoxLayout*  toplay = new WVBoxLayout;
    toplay->addWidget( msgLabel);
    toplay->addWidget( msgEdit, 1);
    toplay->addSpacing(5);
    toplay->addLayout( butlay);
    toplay->setSpacing(10);

    dia->contents()->setPadding(8);
    dia->resize(600, 400);
    dia->contents()->setLayout( toplay);
    msgEdit->setFocus(true);

    cancelButton->clicked().connect( dia, &WDialog::reject);
    okButton->clicked().connect( dia, &WDialog::accept);
    dia->finished().connect( boost::bind( &VcsWindow::doCloseCommitWindow, this, dia, msgEdit));
}


void  VcsWindow::onCheckoutButtonClicked()
{
    _textView->setText("");

    _sapiVcs.pv_vcsCheckout( getRef(), getSelFiles());
}


void  VcsWindow::onTagButtonClicked()
{
    WDialog* dia;
    dia = new WDialog("Set tag name");
    dia->setModal(true);
    dia->show();
    dia->setResizable(false);
    dia->setClosable(true);
    dia->rejectWhenEscapePressed();

    WLabel*  tagLabel = new WLabel("Tag");
    WLineEdit*  tagEdit = new WLineEdit();
    tagEdit->setValidator( new WRegExpValidator("[^ \\/\\\\]+"));
    tagLabel->setBuddy( tagEdit);
    WPushButton*  cancelButton = new WPushButton("Cancel");
    cancelButton->resize(80, 25);
    WPushButton*  okButton = new WPushButton("Ok");
    okButton->resize(80, 25);

    WHBoxLayout*  hlay1 = new WHBoxLayout;
    hlay1->addWidget( tagLabel);
    hlay1->addWidget( tagEdit, 1);
    hlay1->setSpacing(8);
    WHBoxLayout*  butlay = new WHBoxLayout;
    butlay->addWidget( cancelButton);
    butlay->addWidget( okButton);
    butlay->addStretch(1);
    butlay->setSpacing(20);
    WVBoxLayout*  toplay = new WVBoxLayout;
    toplay->addLayout( hlay1);
    toplay->addLayout( butlay);
    toplay->setSpacing(20);

    dia->contents()->setPadding(8);
    dia->resize(300, 120);
    dia->contents()->setLayout( toplay);
    tagEdit->setFocus(true);

    cancelButton->clicked().connect( dia, &WDialog::reject);
    okButton->clicked().connect( dia, &WDialog::accept);
    dia->finished().connect( boost::bind( &VcsWindow::doCloseTagWindow, this, dia, tagEdit));
}


void  VcsWindow::onDiffButtonClicked()
{
    _textView->setText("");

    emit _sapiVcs.pv_vcsDiff( getRef(), getSelFiles());
}


void  VcsWindow::onStatusButtonClicked()
{
    _textView->setText("");

    emit _sapiVcs.pv_vcsStatus();
}


void  VcsWindow::onTreeChanged()
{
    _filesView->clear();
    doUpdate();
}


void  VcsWindow::onRefTypeChanged()
{
    _textView->setText("");
    _refSel->clear();
    if (_refTypeSel->checkedId() == Sel::RefId) {
        emit _sapiVcs.pv_vcsLog(100);
    }
    else if (_refTypeSel->checkedId() == Sel::RefTag) {
        emit _sapiVcs.pv_vcsTags();
    }
}


void  VcsWindow::onRefChanged( WString refTxt)
{
    Q_UNUSED( refTxt);

    _textView->setText("");

    emit _sapiVcs.pv_vcsLogRecord( getRef());
}


void  VcsWindow::doCloseWindow()
{
    delete _wt;
    _wt = 0;

    deleteLater();
}


void  VcsWindow::doCloseTagWindow( WDialog* tagDialog, WLineEdit* tagEdit)
{
    if (tagDialog->result() == WDialog::Accepted) {
        QString  tagName = toQString( tagEdit->text());
        if (!tagName.isEmpty() && (tagEdit->validate() == WValidator::Valid)) {
            emit _sapiVcs.pv_vcsTag( tagName, getRef());
        }
    }
    delete tagDialog;
}


void  VcsWindow::doCloseCommitWindow( WDialog* commitDialog, WTextArea* msgEdit)
{
    if (commitDialog->result() == WDialog::Accepted) {
        QString  msgText = toQString( msgEdit->text());
        if (!msgText.isEmpty()) {
            emit _sapiVcs.pv_vcsCommit( msgText, getSelFiles());
        }
    }
    delete commitDialog;
}


void  VcsWindow::sapiVcsNotify( QString msg)
{
    WString  text = toWString( msg);

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::appendText, this, text));
}


void  VcsWindow::sapiVcsProgress( int percent, QString msg)
{
    Q_UNUSED( percent);
    Q_UNUSED( msg);
}


void  VcsWindow::sapiVcsUserSettingsR( QString name, QString eMail)
{
    Q_UNUSED( name);
    Q_UNUSED( eMail);
}


void  VcsWindow::sapiVcsFilesR( QStringList files)
{
    _fileList     = files;
    _filesUpdated = true;

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::doUpdate, this));
}


void  VcsWindow::sapiVcsTagR()
{
}


void  VcsWindow::sapiVcsCommitR()
{
}


void  VcsWindow::sapiVcsCheckoutR()
{
}


void  VcsWindow::sapiVcsBranchesR( QStringList branches)
{
    Q_UNUSED( branches);
}


void  VcsWindow::sapiVcsTagsR( QStringList tags)
{
    _refTagList = tags;
    _refUpdated = true;

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::doUpdate, this));
}


void  VcsWindow::sapiVcsStatusR( QString status)
{
    WString  text = toWString( status);

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::appendText, this, text));
}


void  VcsWindow::sapiVcsDiffR( QString txt)
{
    WString  text = toWString( txt);

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::appendText, this, text));
}


void  VcsWindow::sapiVcsLogRecordR( QString txt)
{
    WString  text = toWString( txt);

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::appendText, this, text));
}


void  VcsWindow::sapiVcsLogR( QStringList refs, QStringList msgs)
{
    _refIdList    = refs;
    _refIdMsgList = msgs;
    _refUpdated   = true;

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::doUpdate, this));
}


void  VcsWindow::sapiLsR( QStringList files)
{
    _fileList     = files;
    _filesUpdated = true;

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &VcsWindow::doUpdate, this));
}
