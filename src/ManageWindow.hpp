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

#ifndef MANAGEWINDOW_HPP
#define MANAGEWINDOW_HPP

#include <Wt/WObject>
#include <ArnInc/ArnPersistSapi.hpp>
#include <ArnInc/MQFlags.hpp>
#include <ArnInc/ArnItem.hpp>
#include <QTimer>
#include <QStringList>
#include <QObject>

using namespace Wt;

namespace Wt {
    class WLineEdit;
    class WText;
    class WCheckBox;
    class WRadioButton;
    class WButtonGroup;
    class WGroupBox;
    class WDialog;
    class WContainerWidget;
}


class ArnItemUnidir : public ArnItem
{
public:
    explicit ArnItemUnidir( QObject *parent = 0) : ArnItem( parent)
    {
        setForceKeep();
    }
};


class ManageWindow : public QObject, public WObject
{
public:
Q_OBJECT
public:
    struct Sel {
        enum E {
            None        = 0x00,
            Normal      = 0x00,
            Pipe        = 0x01,
            Bidir       = 0x02,
            PersistDB   = 0x04,
            PersistFile = 0x08,
            Mandatory   = 0x10
        };
        MQ_DECLARE_FLAGS( Sel)
    };

    explicit ManageWindow( const QString& path, QObject *parent = 0);

    void  onNewButtonClicked();
    void  onDelButtonClicked();
    void  onSaveButtonClicked();
    void  onCancelButtonClicked();
    void  onResetButtonClicked();
    void  onFolderButtonClicked();

    void  doCloseWindow();

signals:

private slots:
    void  lsR( QStringList files);
    void  mandatoryLsR( QStringList files);
    void  doTypeUpdate();
    void  doPostUpdate();
    void  timeoutReset();
    void  timeoutSave();

private:
    void  doUpdate();
    void  doPersistUpdate();

    WApplication*  _app;
    WDialog*  _wt;
    WPushButton*  _newButton;
    WPushButton*  _delButton;
    WPushButton*  _saveButton;
    WPushButton*  _cancelButton;
    WPushButton*  _resetButton;
    WRadioButton*  _typeNormalButton;
    WRadioButton*  _typeBidirButton;
    WRadioButton*  _typePipeButton;
    WRadioButton*  _persistNoneButton;
    WButtonGroup*  _typeSel;
    WButtonGroup*  _persistSel;
    WCheckBox*  _armDelButton;
    WCheckBox*  _folderButton;
    WCheckBox*  _mandatoryButton;
    WLineEdit*  _pathEdit;
    WLineEdit*  _itemEdit;
    WGroupBox*  _persistBox;
    WGroupBox*  _typeBox;

    ArnPersistSapi  _persistSapi;
    QByteArray  _storeValue;
    ArnItemUnidir  _arnPath;
    QString  _path;
    bool  _isNewMode;
    bool  _isPersistFile;
    bool  _isMandatory;
};

MQ_DECLARE_OPERATORS_FOR_FLAGS( ManageWindow::Sel)

#endif // MANAGEWINDOW_HPP
