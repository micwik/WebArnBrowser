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
#include <Wt/WCheckBox>
#include <Wt/WText>
#include <Wt/WGroupBox>
#include <Wt/WRadioButton>
#include <Wt/WButtonGroup>
#include <Wt/WDialog>
#include "Wt/WHBoxLayout"
#include "Wt/WVBoxLayout"
#include "Wt/WServer"
#include "Wt/WRegExpValidator"

#include "ManageWindow.hpp"
#include "wqlib/WQApplication.hpp"
#include <ArnLib/Arn.hpp>
#include <QDebug>


using namespace Wt;


ManageWindow::ManageWindow( const QString &path, QObject *parent) :
    QObject( parent), WObject(0)
{
    _path          = path;
    _app           = WApplication::instance();
    _isNewMode     = false;
    _isPersistFile = false;
    _isMandatory   = false;

    _wt = new WDialog("Manage nodes");
    _wt->setModal(false);
    _wt->show();
    _wt->setResizable(true);
    _wt->setClosable(true);
    _wt->setMinimumSize(300, 200);

    _newButton = new WPushButton("New");
    _newButton->resize(100, 25);
    _saveButton = new WPushButton("Save");
    _saveButton->resize(100, 25);
    _cancelButton = new WPushButton("Cancel");
    _cancelButton->resize(100, 25);
    _resetButton = new WPushButton("Reset");
    _resetButton->resize(100, 25);
    _delButton = new WPushButton("Delete");
    _delButton->resize(100, 25);
    _delButton->setMargin(40, Top);
    _armDelButton = new WCheckBox("Arm Delete");

    _pathEdit = new WLineEdit();
    _pathEdit->setReadOnly(true);
    _pathEdit->setHeight(20);
    _pathEdit->setMargin(20, Bottom);
    WLabel*  pathLabel = new WLabel("Path");
    pathLabel->setBuddy( _pathEdit);

    _itemEdit = new WLineEdit();
    _itemEdit->setHeight(20);
    _itemEdit->setValidator( new WRegExpValidator("^[^/ ]+([^ ]*[^/ ])*$"));
    WLabel*  itemLabel = new WLabel("Item");
    itemLabel->setBuddy( _itemEdit);

    _folderButton = new WCheckBox("Folder");

    WRadioButton *button;
    _typeSel = new WButtonGroup( this);
    _typeBox = new WGroupBox("Type select");
    _typeBox->setMinimumSize(120, WLength::Auto);
    _typeNormalButton = new Wt::WRadioButton("Normal", _typeBox);
    _typeNormalButton->setMargin(10);
    _typeSel->addButton( _typeNormalButton, Sel::Normal);
    _typeBidirButton = new Wt::WRadioButton("BiDirectional", _typeBox);
    _typeBidirButton->setMargin(10);
    _typeSel->addButton( _typeBidirButton, Sel::Bidir);
    _typePipeButton = new Wt::WRadioButton("Pipe", _typeBox);
    _typeSel->addButton( _typePipeButton, Sel::Pipe);

    _persistSel = new WButtonGroup( this);
    _persistBox = new WGroupBox("Persist select");
    _persistNoneButton = new Wt::WRadioButton("None", _persistBox);
    _persistSel->addButton( _persistNoneButton, Sel::None);
    button = new Wt::WRadioButton("DB", _persistBox);
    _persistSel->addButton( button, Sel::PersistDB);
    button = new Wt::WRadioButton("File", _persistBox);
    button->setMargin(10);
    _persistSel->addButton( button, Sel::PersistFile);
    new Wt::WBreak( _persistBox);
    new Wt::WBreak( _persistBox);
    _mandatoryButton = new WCheckBox("Mandatory", _persistBox);

    WVBoxLayout*  butlay = new WVBoxLayout;
    butlay->addWidget( _newButton);
    butlay->addWidget( _saveButton);
    butlay->addWidget( _cancelButton);
    butlay->addWidget( _resetButton);
    butlay->addWidget( _delButton);
    butlay->addWidget( _armDelButton);
    butlay->addStretch(1);
    butlay->setSpacing(20);
    WHBoxLayout*  hlay1 = new WHBoxLayout;
    hlay1->addWidget( pathLabel);
    hlay1->addWidget( _pathEdit, 1);
    hlay1->setSpacing(8);
    WHBoxLayout*  hlay2 = new WHBoxLayout;
    hlay2->addWidget( itemLabel);
    hlay2->addWidget( _itemEdit, 1);
    hlay2->setSpacing(8);
    WVBoxLayout*  vlay1 = new WVBoxLayout;
    vlay1->addLayout( hlay1);
    vlay1->addLayout( hlay2);
    vlay1->addWidget( _folderButton);
    vlay1->addWidget( _typeBox);
    vlay1->addWidget( _persistBox);
    vlay1->addStretch(1);
    vlay1->setSpacing(20);
    WHBoxLayout*  toplay = new WHBoxLayout;
    toplay->addLayout( butlay);
    toplay->addLayout( vlay1, 1);
    toplay->setSpacing(30);

    _wt->contents()->setPadding(8);
    _wt->resize(600, 370);
    _wt->contents()->setLayout( toplay);

    //// Logics
    _persistSapi.open("//.sys/Persist/Pipes/CommonPipe");
    connect( &_persistSapi, SIGNAL(rq_lsR(QStringList)), this, SLOT(lsR(QStringList)));
    connect( &_persistSapi, SIGNAL(rq_dbMandatoryLsR(QStringList)), this, SLOT(mandatoryLsR(QStringList)));

    _arnPath.open( path);
    connect( &_arnPath, SIGNAL(arnLinkDestroyed()), this, SLOT(doPostUpdate()));
    _newButton->clicked().connect( this, &ManageWindow::onNewButtonClicked);
    _armDelButton->clicked().connect( this, &ManageWindow::doUpdate);
    _delButton->clicked().connect( this, &ManageWindow::onDelButtonClicked);
    _saveButton->clicked().connect( this, &ManageWindow::onSaveButtonClicked);
    _cancelButton->clicked().connect( this, &ManageWindow::onCancelButtonClicked);
    _resetButton->clicked().connect( this, &ManageWindow::onResetButtonClicked);
    _folderButton->clicked().connect( this, &ManageWindow::doUpdate);
    _typeSel->checkedChanged().connect( this, &ManageWindow::doUpdate);
    _persistSel->checkedChanged().connect( this, &ManageWindow::doUpdate);
    _wt->finished().connect( this, &ManageWindow::doCloseWindow);

    //ArnItem::Mode  pathMode = _arnPath.getMode();
    if (!_arnPath.isFolder()) {
        emit _persistSapi.pv_ls( path);  // Will finally trigger update of persist
        emit _persistSapi.pv_dbMandatoryLs( path);  // Will finally trigger update of mandatory
    }

    doTypeUpdate();
}


void  ManageWindow::doUpdate()
{
    bool  isPathFolder = _arnPath.isFolder();
    bool  isPathOpen   = _arnPath.isOpen();

    _pathEdit->setText( toWString( isPathOpen ? _path : QString()));

    if (!isPathOpen) {  // Path link has been destroyed
        _isNewMode = false;
        _itemEdit->setText("");
        _itemEdit->setDisabled(true);
        _folderButton->setChecked(false);
        _folderButton->setDisabled(true);
        _armDelButton->setChecked(false);
        _armDelButton->setDisabled(true);
    }
    else if (!_arnPath.isFolder()) {  // Edit non folder item
        _itemEdit->setText( toWString( _arnPath.name( ArnLink::NameF())));
        _itemEdit->setDisabled(true);
        _folderButton->setChecked(false);
        _folderButton->setDisabled(true);
        if (_arnPath.isPipeMode()) {

        }
    }
    else if (!_isNewMode) {  // View folder
        _itemEdit->setText("");
        _itemEdit->setDisabled(true);
        _folderButton->setChecked(true);
        _folderButton->setDisabled(true);
        _armDelButton->setChecked(false);
        _armDelButton->setDisabled(true);
    }
    else {  // Edit new non folder item
        _itemEdit->setDisabled(false);
        _folderButton->setDisabled(true);  // No support yet for new folder (will not sync)
        _armDelButton->setChecked(false);
        _armDelButton->setDisabled(true);
    }

    bool  isFolder          = _folderButton->isChecked();
    bool  isTypePersistable = !isFolder && (_typeSel->checkedId() != Sel::Pipe);
    bool  isPersistDB       = isTypePersistable && (_persistSel->checkedId() == Sel::PersistDB);

    _newButton->setEnabled( !_isNewMode && isPathFolder);
    _saveButton->setEnabled((_isNewMode || !isPathFolder) && isPathOpen);
    _cancelButton->setEnabled( _isNewMode);
    _delButton->setEnabled( _armDelButton->isChecked());
    _resetButton->setEnabled( false);  // Default

    _typeBox->setDisabled( isFolder || !isPathOpen);
    _persistBox->setDisabled( !isTypePersistable || !isPathOpen);
    _mandatoryButton->setDisabled( !isPersistDB);

    ArnItem::Mode  modePath = _arnPath.getMode();
    if (!isPathFolder && isPathOpen) {
        if (modePath.is( modePath.Pipe)) {
            _typeNormalButton->setDisabled(true);
            _typeBidirButton->setDisabled(true);
            _resetButton->setEnabled( true);
        }
        else if (modePath.is( modePath.BiDir)) {
            _typeNormalButton->setDisabled(true);
            _typeBidirButton->setDisabled(false);
            _resetButton->setEnabled( true);
        }
        else {  // Normal type
            _typeNormalButton->setDisabled(false);
            _typeBidirButton->setDisabled(false);
        }

        if (_isPersistFile) {
            _typePipeButton->setDisabled(true);
        }
        else if (modePath.is( modePath.Save)) {
            _typePipeButton->setDisabled(true);
            _persistNoneButton->setDisabled(true);
            _resetButton->setEnabled( true);
        }
        else {  // No persist
            _typePipeButton->setDisabled(false);
            _persistNoneButton->setDisabled(false);
        }
    }

    _app->triggerUpdate();
}


void  ManageWindow::doTypeUpdate()
{
    ArnItem::Mode  pathMode = _arnPath.getMode();
    if (!_arnPath.isFolder()) {
        if (pathMode.is( pathMode.Pipe))
            _typeSel->setCheckedButton( _typeSel->button( Sel::Pipe));
        else if (pathMode.is( pathMode.BiDir))
            _typeSel->setCheckedButton( _typeSel->button( Sel::Bidir));
        else
            _typeSel->setCheckedButton( _typeSel->button( Sel::Normal));
    }
    else {
        _typeSel->setCheckedButton(0);
    }

    doUpdate();
}


void  ManageWindow::doPersistUpdate()
{
    ArnItem::Mode  modePath = _arnPath.getMode();
    if (_isPersistFile)
        _persistSel->setCheckedButton( _persistSel->button( Sel::PersistFile));
    else if (modePath.is( modePath.Save))
        _persistSel->setCheckedButton( _persistSel->button( Sel::PersistDB));
    else
        _persistSel->setCheckedButton( _persistSel->button( Sel::None));

    _mandatoryButton->setChecked( _isMandatory);

    doUpdate();
}


void  ManageWindow::timeoutSave()
{
    _arnPath.arnImport( _storeValue);

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &ManageWindow::doUpdate, this));
}


void  ManageWindow::timeoutReset()
{
    _arnPath.open( _path);
    _arnPath.arnImport( _storeValue);
    emit _persistSapi.pv_ls( _path);  // Will finally trigger update of persist
    emit _persistSapi.pv_dbMandatoryLs( _path);  // Will finally trigger update of mandatory

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &ManageWindow::doTypeUpdate, this));
}


void  ManageWindow::onNewButtonClicked()
{
    _isNewMode = true;
    _isPersistFile = false;

    _itemEdit->setText("");
    _itemEdit->setFocus();
    _folderButton->setChecked(false);
    _typeSel->setCheckedButton( _typeSel->button( Sel::Normal));
    _persistSel->setCheckedButton( _persistSel->button( Sel::None));

    doUpdate();
}


void  ManageWindow::onDelButtonClicked()
{
    if (!_arnPath.isFolder()) {
        emit _persistSapi.pv_rm( _arnPath.path());  // Any persist file is deleted
        if (_arnPath.isSaveMode())
            emit _persistSapi.pv_dbMandatory( _arnPath.path(), false);
        emit _persistSapi.pv_ls( _arnPath.path());
        _arnPath.destroyLink();
    }

    doUpdate();
}


void  ManageWindow::onSaveButtonClicked()
{
    ArnItem  arnItemNew;
    ArnItem*  arnItem;

    if (_isNewMode) {
        _itemEdit->setFocus();
        if (_itemEdit->validate() != WValidator::Valid)  return;  // Item not valid
        QString  item = toQString( _itemEdit->text());
        if (item.isEmpty())  return;  // Item may not be empty

        QString  itemPath = Arn::addPath( _arnPath.path(), item);
        arnItemNew.open( itemPath);
        arnItem = &arnItemNew;
    }
    else
        arnItem = &_arnPath;
    if (!arnItem->isOpen())  return;

    ArnItem::Mode  mode;

    switch (_typeSel->checkedId()) {
    case Sel::Bidir:
        arnItem->addMode( mode.BiDir);
        break;
    case Sel::Pipe:
        arnItem->addMode( mode.Pipe);
        break;
    }

    switch (_persistSel->checkedId()) {
    case Sel::PersistDB:
        arnItem->addMode( mode.Save);
        emit _persistSapi.pv_rm( arnItem->path());
        break;
    case Sel::PersistFile:
        emit _persistSapi.pv_touch( arnItem->path());
        break;
    case Sel::None:
        emit _persistSapi.pv_rm( arnItem->path());
        break;
    }

    emit _persistSapi.pv_dbMandatory( arnItem->path(), _mandatoryButton->isChecked());

    emit _persistSapi.pv_ls( arnItem->path());
    emit _persistSapi.pv_dbMandatoryLs( arnItem->path());

    _storeValue = _arnPath.arnExport();
    QTimer::singleShot(300, this, SLOT(timeoutSave()));

    _isNewMode = false;
    doUpdate();
}


void  ManageWindow::onCancelButtonClicked()
{
    _isNewMode = false;

    doUpdate();
}


void  ManageWindow::onResetButtonClicked()
{
    if (!_arnPath.isFolder()) {
        _storeValue = _arnPath.arnExport();
        if (_arnPath.isSaveMode())
            emit _persistSapi.pv_dbMandatory( _arnPath.path(), false);
        _arnPath.destroyLink();
        QTimer::singleShot(300, this, SLOT(timeoutReset()));
    }
}


void  ManageWindow::onFolderButtonClicked()
{
}


void  ManageWindow::doCloseWindow()
{
    delete _wt;
    _wt = 0;

    deleteLater();
}


void  ManageWindow::lsR( QStringList files)
{
    _isPersistFile = false;
    foreach (QString file, files) {
        if (file == _arnPath.path())
            _isPersistFile = true;
    }

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &ManageWindow::doPersistUpdate, this));
}


void  ManageWindow::mandatoryLsR( QStringList files)
{
    _isMandatory = false;
    foreach (QString file, files) {
        if (file == _arnPath.path())
            _isMandatory = true;
    }

    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &ManageWindow::doPersistUpdate, this));
}


void  ManageWindow::doPostUpdate()
{
    WServer::instance()->post( _app->sessionId(),
                              boost::bind( &ManageWindow::doUpdate, this));
}
