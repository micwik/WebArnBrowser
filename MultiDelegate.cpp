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

#define QT_NO_EMIT

#include "Wt/WAbstractItemModel"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WModelIndex"
#include "Wt/WHBoxLayout"
#include "Wt/WText"
#include "Wt/WComboBox"
#include "Wt/WDatePicker"
#include "Wt/WDialog"
#include "Wt/WPushButton"
#include "Wt/Http/Response"

#include "MultiDelegate.hpp"
#include "ItemDataRole.hpp"
#include "wqlib/WQApplication.hpp"
#include <QStringList>
#include <QVariant>
#include <QDate>
#include <QImage>
#include <QBuffer>
#include <QDebug>
#include <QAtomicInt>
#include <QFile>


ImageResource::ImageResource( const QString& fileName, const char* format, const char* mime, Wt::WObject* parent)
    : Wt::WResource( parent)
{
    _format = format;
    _mime   = mime;
    suggestFileName( Wt::WString::fromUTF8( fileName.toUtf8().constData()));
}


ImageResource::~ImageResource()
{
    beingDeleted(); // see "Concurrency issues"
}


void  ImageResource::setImage( const QImage& image, int quality)
{
    QMutexLocker  mutexLock(&_mutex);

    _imageBA.resize(0);
    QBuffer  buffer( &_imageBA);
    buffer.open( QIODevice::WriteOnly);
    bool isOk = image.save( &buffer, _format.constData(), quality);  // writes image into ba in XXX format
    Q_UNUSED( isOk);
}


void  ImageResource::handleRequest( const Wt::Http::Request& request, Wt::Http::Response& response)
{
    qDebug() << "***** ImageRes HandleReq: Query=" << request.queryString().c_str();

    QByteArray  imageBA;
    _mutex.lock();
    imageBA += _imageBA;  // Force copy
    _mutex.unlock();

    response.setMimeType( _mime.constData());
    response.out().write( imageBA.constData(), imageBA.size());

    qDebug() << "***** ImageRes HandleReq: End";
}


MultiDelegate::MultiDelegate( WObject* parent)
    : WItemDelegate( parent)
{
}


WWidget*  MultiDelegate::update( WWidget* widget, const WModelIndex& index, WFlags<ViewItemRenderFlag> flags)
{
    if (flags & RenderEditing)  return WItemDelegate::update( widget, index, flags);

    boost::any  valueExt = index.data( ItemDataRoleUser::DataExt);
    if (valueExt.type() != typeid(QVariant))  return WItemDelegate::update( widget, index, flags);

    QVariant  valueQ = boost::any_cast<QVariant>(valueExt);

    switch (valueQ.type()) {
    case QMetaType::QImage: {
        if (widget) {  // MW: Workaround by allways create new widget to avoid lockup
            delete widget;
            widget = 0;
        }
        WContainerWidget* wc;
        WImage*  wImage;
        ImageResource*  imgRes;
        if (widget && (widget->objectName() == "imageC")) {
            qDebug() << "******** Use image";
            wc     = static_cast<WContainerWidget *>(widget);
            wImage = static_cast<WImage *>(widget->find("image"));
            imgRes = static_cast<ImageResource *>(wImage->imageLink().resource());
        }
        else {
            qDebug() << "******** New!!! image";
            wc = new WContainerWidget();
            wc->setObjectName("imageC");
            widget = wc;

            wImage = new WImage( wc);

            imgRes = new ImageResource( "image.png", "PNG", "image/png", wImage);
            wImage->setObjectName("image");
            wImage->setImageLink( imgRes);
            wImage->setMaximumSize(23, 23);

            WText *t = new WText( wc);  // Must have this "t" object to be able to edit
            t->setObjectName("t");
        }

        QImage  image = valueQ.value<QImage>();
        image = image.scaledToHeight(20);
        imgRes->setImage( image);
        // imgRes->setChanged();

        qDebug() << "******** End image";
        return widget;
    }
    default:;
    }

    return WItemDelegate::update( widget, index, flags);
}


WWidget*  MultiDelegate::createEditor( const WModelIndex& index, WFlags<ViewItemRenderFlag> flags) const
{
    WContainerWidget *const result = new WContainerWidget();
    result->setSelectable(true);

    boost::any  value    = index.data( EditRole);
    boost::any  valueExt = index.data( ItemDataRoleUser::DataExt);
    boost::any  path     = index.data( ItemDataRoleUser::Path);

    if (valueExt.type() == typeid(QVariant)) {
        QVariant  valueQ = boost::any_cast<QVariant>(valueExt);
        switch (valueQ.type()) {
        case QMetaType::QDate: {
            WDatePicker *editor = new WDatePicker;
            // setupCalenderWidget( editor);
            // editor->setMaximumWidth( editor->sizeHint().width());
            editor->setFormat("yyyy-MM-dd");
            result->enterPressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, true));
            result->escapePressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
            result->escapePressed().preventPropagation();
            if (flags & RenderFocused)
                editor->lineEdit()->setFocus();

            result->addWidget( editor);
            setEditState( result, value);
            return result;
        }
        case QMetaType::QImage: {
            WDialog*  editor = new WDialog("Image Viewer");
            WImage*  wImage = new WImage( editor->contents());
            new WBreak( editor->contents());
            WPushButton*  closeButton = new WPushButton("Close", editor->contents());

            ImageResource*  imgRes = new ImageResource( "image.jpg", "JPG", "image/jpeg", wImage);
            wImage->setObjectName("image");
            wImage->setImageLink( imgRes);

            editor->setModal(false);
            editor->show();
            editor->setResizable(true);
            closeButton->clicked().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
            closeButton->enterPressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
            closeButton->escapePressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
            closeButton->escapePressed().preventPropagation();
            if (flags & RenderFocused)
                closeButton->setFocus();

            result->addWidget( editor);
            setEditState( result, valueExt);
            return result;
        }
        default:;
        }
    }
    else if (value.type() == typeid(WString)) {
        if (boost::any_cast<QString>(path).endsWith("/info")) {
            QRegExp rx("<help>(.*)</help>");
            if (rx.indexIn( toQString( boost::any_cast<WString>(value))) >= 0) {
                WString  helpText = toWString( rx.cap(1));

                WDialog*  editor = new WDialog("Help");
                // WText*  wText = new WText( helpText, editor->contents());
                WText*  wText = new WText( "helpText dittan dutt", editor->contents());
                new WLineEdit( "helpText dhhhhittan dutt", editor->contents());
                wText->setText("Hej hopp nsdnsdnsmdnsndm,s snd,nsdnsmd");
                wText->setObjectName("text");
                //wText->show();
                new WBreak( editor->contents());
                WPushButton*  closeButton = new WPushButton("Close", editor->contents());

                editor->setModal(false);
                editor->show();
                editor->setResizable(true);
                closeButton->clicked().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
                closeButton->enterPressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
                closeButton->escapePressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
                closeButton->escapePressed().preventPropagation();
                if (flags & RenderFocused)
                    closeButton->setFocus();

                result->addWidget( editor);
                // setEditState( result, valueExt);
                return result;
            }
        }
        else {
            boost::any  varList = index.model()->data( index, ItemDataRoleUser::EnumList);
            if (!varList.empty()) {
                WComboBox*  editor = new WComboBox;
                foreach( QString item, boost::any_cast<QStringList>(varList)) {
                    editor->addItem( toWString( item));
                }
                editor->enterPressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, true));
                editor->escapePressed().connect( boost::bind( &MultiDelegate::doCloseEditor, this, result, false));
                editor->escapePressed().preventPropagation();
                if (flags & RenderFocused)
                    editor->setFocus();

                result->addWidget( editor);
                setEditState( result, value);
                return result;
            }
        }
    }

    return WItemDelegate::createEditor( index, flags);
}


void  MultiDelegate::doCloseEditor( WWidget* editor, bool save)  const
{
    closeEditor().emit(editor, save);
}


boost::any  MultiDelegate::editState( WWidget* editor)  const
{
    WContainerWidget  *cw = dynamic_cast<WContainerWidget *>(editor);
    Q_ASSERT(cw);
    WWidget  *w = cw->widget(0);
    Q_ASSERT(w);

    if (dynamic_cast<WComboBox *>(w)) {
        WComboBox  *ed = static_cast<WComboBox *>(w);
        return boost::any( ed->currentText());
    }
    else if (dynamic_cast<WDatePicker *>(w)) {
        WDatePicker  *ed = static_cast<WDatePicker *>(w);
        return boost::any( QVariant( QDate::fromString( toQString( ed->date().toString("yyyy-MM-dd")), "yyyy-MM-dd")));
    }
    else {
        return WItemDelegate::editState( editor);
    }

    return boost::any();
}


void  MultiDelegate::setEditState( WWidget* editor, const boost::any& value)  const
{
    WContainerWidget  *cw = dynamic_cast<WContainerWidget *>(editor);
    Q_ASSERT(cw);
    WWidget  *w = cw->widget(0);
    Q_ASSERT(w);

    if (dynamic_cast<WComboBox *>(w)) {
        WComboBox  *ed = static_cast<WComboBox *>(w);
        ed->setCurrentIndex( ed->findText( boost::any_cast<WString>(value)));
        return;
    }
    else if (dynamic_cast<WDatePicker *>(w)) {
        WDatePicker  *ed = static_cast<WDatePicker *>(w);
        ed->setDate( WDate::fromString( boost::any_cast<WString>(value), "yyyy-MM-dd"));
    }
    else if (dynamic_cast<WDialog *>(w)) {
        WImage  *edImg = static_cast<WImage *>( static_cast<WDialog *>(w)->contents()->find("image"));
        if (edImg) {
            ImageResource*  imgRes = static_cast<ImageResource *>(edImg->imageLink().resource());
            Q_ASSERT( imgRes);

            QImage image = boost::any_cast<QVariant>(value).value<QImage>();
            imgRes->setImage( image);
            imgRes->setChanged();
            return;
        }
        return;

        WText  *edTxt = static_cast<WText *>( static_cast<WDialog *>(w)->contents()->find("text"));
        if (edTxt) {
            QRegExp rx("<help>(.*)</help>");
            if (rx.indexIn( toQString( boost::any_cast<WString>(value))) >= 0) {
                WString  helpText = toWString( rx.cap(1));
                edTxt->setText("Hej hopp lddfödäfldäfldfldfldäfldälfäödlfödlfädlfä sdfläsldfäsdlf");
            }
            return;
        }
    }
    else {
        return WItemDelegate::setEditState( editor, value);
    }
}
