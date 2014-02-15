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

#include <iostream>

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"
#include "Wt/WServer"

#include "ArnModel.hpp"
#include "wqlib/WQApplication.hpp"
#include "QtMainThread.hpp"
#include <ArnInc/Arn.hpp>
//#include <ArnInc/ArnLink.hpp>
#include <ArnInc/ArnMonitor.hpp>
#include <ArnInc/XStringMap.hpp>
#include <QStringList>
#include <QRegExp>
#include <QDebug>


void  ArnNode::init()
{
    _parent     = 0;
    _valueChild = 0;
    _setChild   = 0;
    _propChild  = 0;
    _infoChild  = 0;
    _helpChild  = 0;
    _arnMon     = 0;
    _setMap     = 0;
    _propMap    = 0;
}


ArnNode::ArnNode( ArnNode* parent) :
        ArnItem( parent)
{
    //// Init Dummy Node
    init();
    _parent = parent;
    parent->_children.insert(0, this);
}


ArnNode::ArnNode( const QString &path, QObject *qobjParent) :
        ArnItem( path, qobjParent)
{
    //// Init Root Node / ValueChild Node / SetChild Node
    init();
}


ArnNode::ArnNode( ArnNode* parent, const QString& item, int row) :
        ArnItem( *parent, item, parent)     // New item is also a QObject child
{
    init();
    _parent = parent;
    if (row >= 0)
        parent->_children.insert( row, this);
}


ArnNode::~ArnNode()
{
    if (_setMap)   delete _setMap;
    if (_propMap)  delete _propMap;
}


using namespace Wt;


QString  ArnNode::name( Arn::NameF nameF)  const
{
    if (isOpen())  return ArnItem::name( nameF);
    else           return "<Empty>";  // Dummy node
}


QString  ArnNode::path()  const
{
    if (isOpen())  return ArnItem::path();
    else           return _parent->ArnItem::path() + name(Arn::NameF());
}


ArnModelW::ArnModelW(WObject *parent)
  : WAbstractItemModel(parent)
{
    init();
}


void ArnModelW::init()
{
    _app = 0;
    qDebug() << "### arnModelW init:";
    _modelQ = new ArnModelQ( this);
}


ArnModelW::~ArnModelW()
{
    delete _modelQ;
}


void ArnModelW::setApplication( WApplication* app)
{
    _app = app;
}


WModelIndex  ArnModelW::indexFromNode( const ArnNode *node, int column)  const
{
    WModelIndex  index;
    if (!node || (node == _modelQ->_rootNode))  return index;

    ArnNode*  parentNode = node->_parent;
    if (parentNode && (column >= 0)) {
        int  row = parentNode->_children.indexOf( const_cast<ArnNode*>(node));
        if (row >= 0)
            index = createIndex( row, column, parentNode);
    }
    return index;
}


ArnNode*  ArnModelW::nodeFromIndex( const WModelIndex& index) const
{
    if (index.isValid()) {
        ArnNode*  parentNode = static_cast<ArnNode*>( index.internalPointer());
        Q_ASSERT(parentNode);
        ArnNode*  node = parentNode->_children.value( index.row(), 0);
        Q_ASSERT( node);
        return node;
    }
    else {
        return _modelQ->_rootNode;
    }
}


WFlags<ItemFlag> ArnModelW::flags(const WModelIndex& index) const
{
    if (!index.isValid()) {
        return WFlags<ItemFlag>(0);
    }
    if (index.column() == 1) {
        return WAbstractItemModel::flags( index) | Wt::ItemIsEditable;
    }
    else {
        return WAbstractItemModel::flags( index);
    }
}


WModelIndex ArnModelW::parent(const WModelIndex& index) const
{
    // qDebug() << "### arnModelW parent: Check";
    if (!index.isValid())
        return index;

    ArnNode*  parentNode = static_cast<ArnNode*>( index.internalPointer());

    return indexFromNode( parentNode, 0);
}


boost::any ArnModelW::data(const WModelIndex& index, int role) const
{
    // qDebug() << "### arnModelW data:";
    ArnNode*  node = nodeFromIndex( index);
    Q_ASSERT( node);

    switch (role) {
    case Wt::DisplayRole:
        // Fall throu

    case Wt::EditRole:
        // Fall throu

    case ItemDataRoleUser::DataExt:
        // if (!node)  return boost::any();

        if (!node) {
            return WString("<Null>");  // Should not be ...
        }
        switch (index.column()) {
        case 0:
            // qDebug() << "### arnModelW data: Col0 Disp/Edit path=" << node->path();
            return toWString( node->name( Arn::NameF::NoFolderMark));
        case 1:
            boost::any  retVal = adjustedNodeData( node, role);
            return retVal;
        }
        break;

    case Wt::ToolTipRole:
        if (node->isFolder() && node->_infoChild) {
            QString  info = node->_infoChild->toString();
            QRegExp rx("<tt>(.*)</tt>");
            if (rx.indexIn( info) >= 0) {
                return toWString( rx.cap(1));
            }
        }
        break;

    case ItemDataRoleUser::Help:
        if (node->isFolder() && node->_helpChild) {
            QString  help = node->_helpChild->toString();
            return toWString( help);
        }
        break;

    case ItemDataRoleUser::EnumList:
        if (index.column() < 1)  return boost::any();

        if (node->isFolder() && node->_setMap && node->_valueChild) {
            // qDebug() << "EnumList: " << node->_setMap->values();
            return node->_setMap->values();
        }
        break;

    case ItemDataRoleUser::Path:
        return node->path();
    }

    return boost::any();
}


boost::any  ArnModelW::adjustedNodeData( const ArnNode* node, int role) const
{
    if (!node)  return WString();

    const ArnNode*  valueNode = node;
    QString  unit;
    int  prec = -1;

    if (node->isFolder()) {
        if (!node->_valueChild)  return WString();

        valueNode = node->_valueChild;

        if (node->_setMap) {
            return toWString( node->_setMap->valueString( valueNode->toByteArray()));
        }

        if (node->_propMap) {
            if (role == Wt::DisplayRole) {
                unit = node->_propMap->valueString("unit");
                if (!unit.isEmpty())
                    unit.insert(0, " ");
            }
            prec = node->_propMap->value("prec", "-1").toInt();
        }
    }

    Arn::DataType  type = valueNode->type();
    if (prec >= 0) { // The value should be a floating-point
        return toWString( QString::number( valueNode->toDouble(), 'f', prec) + unit);
    }
    else if ((type == type.ByteArray)
         ||  (type == type.String)) {
        QString  value = valueNode->toString();
        int  i = value.indexOf('\n');
        if (i < 0) {
            // qDebug() << "### arnModelW adjustedNodeData: path=" << node->path()
            //          << " SetAs=" << node->setAs().e
            //          << " QV-type=" << node->toVariant().typeName()
            //          << " QV-val=" << node->toVariant().toString();
            return toWString( value + unit);
        }
        else
            return toWString( value.left(i));
    }
    else {
        if (role == ItemDataRoleUser::DataExt) {
            // qDebug() << "### arnModelW adjustedNodeData: ur+1 path=" << node->path()
            //          << " SetAs=" << node->setAs().e
            //          << " QV-type=" << node->toVariant().typeName()
            //          << " QV-val=" << node->toVariant().toString();
            return valueNode->toVariant();
        }
        else {
            // if (setAs.e == setAs.Variant) {
            //     qDebug() << "### arnModelW adjustedNodeData: path=" << node->path()
            //              << " SetAs=" << node->setAs().e
            //              << " QV-type=" << node->toVariant().typeName()
            //              << " QV-val=" << node->toVariant().toString();
            // }
            return toWString( valueNode->toString() + unit);
        }
    }
}


boost::any ArnModelW::headerData(int section, Orientation orientation,
                                          int role) const
{
    if ((orientation == Wt::Horizontal) && (role == Wt::DisplayRole)) {
        switch (section) {
        case 0:
            return std::string("Path");
        case 1:
            return std::string("Value");
        }
    }
    else {
        // qDebug() << "headerData: ori=" << orientation << " role=" << role;
    }
    return WAbstractItemModel::headerData( section, orientation, role);
}


WModelIndex ArnModelW::index(int row, int column,
                                      const WModelIndex& parent) const
{
    if ((row < 0) || (column < 0))  return WModelIndex();

    ArnNode*  parentNode = nodeFromIndex( parent);
    Q_ASSERT( parentNode);

    // qDebug() << "### arnModelW index: row=" << row << " col=" << column << " parent=" << parentNode->path();
    if (parentNode->_expandState.e == ArnNode::Expand::Off)
        fetchMore( parent);

    return createIndex( row, column, parentNode);
}


int ArnModelW::columnCount(const WModelIndex& parent) const
{
    Q_UNUSED( parent);

    return 2;
}


int ArnModelW::rowCount(const WModelIndex& parent) const
{
    // qDebug() << "### arnModelW rowCount: Check";
    if (parent.column() > 0)  return 0;

    ArnNode*  parentNode = nodeFromIndex( parent);
    if (!parentNode) {  // Should not be ...
        qDebug() << "### arnModelW rowCount: parent=null Grandparent=" << nodeFromIndex( this->parent( parent))->path();
        return 0;
    }

    int ret = parentNode->_children.size();
    // qDebug() << "### arnModelW rowCount: col0 parent=" << parentNode->path()
    //          << " expand=" << parentNode->_expandState.e << " ret=" << ret;
    return ret;
}


bool ArnModelW::setData(const WModelIndex& index,
                                 const boost::any& value, int role)
{
    if (index.isValid()  &&  role == Wt::EditRole) {
        ArnNode*  node = nodeFromIndex( index);
        if (!node)  return false;

        if (node->isFolder()) {
            if (node->_valueChild) {
                if (node->_setMap) {
                    node->_valueChild->setValue( node->_setMap->key( toQString( boost::any_cast<WString>(value))));
                }
                else {
                    node->_valueChild->setValue( toQString( boost::any_cast<WString>(value)));
                }
                emitDataChangedI( index);
            }
        }
        else {
            if (value.type() == typeid(QVariant))
                node->setValue( boost::any_cast<QVariant>(value));
            else
                node->setValue( toQString( boost::any_cast<WString>(value)));
            emitDataChangedI( index);
        }
        return true;
    }
    return false;
}


void *ArnModelW::toRawIndex(const WModelIndex& index) const
{
    return static_cast<void*>( nodeFromIndex( index));
}


WModelIndex ArnModelW::fromRawIndex(void* rawIndex) const
{
    return indexFromNode(static_cast<ArnNode*>( rawIndex), 0);
}


bool  ArnModelW::hasChildren( const WModelIndex& index)  const
{
    ArnNode*  parentNode = nodeFromIndex( index);
    // qDebug() << "### arnModelW hasChildren: Check";
    if (!parentNode)  return false;

    return parentNode->isFolder();
}


bool  ArnModelW::hasIndex( int row, int column, const WModelIndex& parent)  const
{
    // qDebug() << "### arnModelW hasIndex: Check";
    fetchMore( parent);

    return WAbstractItemModel::hasIndex( row, column, parent);
}


void  ArnModelW::fetchMore( const WModelIndex& parent)  const
{
    ArnNode*  parentNode = nodeFromIndex( parent);
    // qDebug() << "### arnModelW fetchMore: Check";
    if (!parentNode)  return;
    if (parentNode->_expandState.e > ArnNode::Expand::Off)
                                  return;  // Node is already exposing its children
    if (!parentNode->isFolder())  return;

    // qDebug() << "### arnModelW fetchMore: Monitor path=" << parentNode->path();
    _modelQ->arnMonStart( parentNode);

    parentNode->_expandState = ArnNode::Expand::Pre;
}


void  ArnModelW::doInsertItem( ArnNode* node, QString itemName)
{
    if (!node)  return;

    WModelIndex  index = indexFromNode( node, 0);

    if (node->_expandState.e == ArnNode::Expand::Pre) {
        if (node->_children.size() == 1) {  // This should be, otherwise internal error
            // Remove dummy node
            beginRemoveRows( index, 0, 0);
            delete node->_children.at(0);
            node->_children.removeAt(0);
            endRemoveRows();
        }
        node->_expandState = ArnNode::Expand::On;
    }

    int  insRow;
    for (insRow = 0; insRow < node->_children.size(); ++insRow) {
        ArnNode*  child = node->_children.at( insRow);
        QString  childName = child->name( Arn::NameF());
        // qDebug() << "### arnModel doInsertItem: item=" << itemName << " child=" << childName;
        if (itemName == childName)  return;  // Item already exist
        if (itemName < childName)  break;  // Sorting place found
    }

    WAbstractItemModel::beginInsertRows( index, insRow, insRow);
    // qDebug() << "### arnModelW doInsertItem: create item=" << itemName;
    ArnNode*  child = new ArnNode( node, itemName, insRow);
    QObject::connect( child, SIGNAL(changed()), _modelQ, SLOT(nodeDataChanged()));
    QObject::connect( child, SIGNAL(arnLinkDestroyed()), _modelQ, SLOT(destroyNode()));
    if (child->isFolder()) {
        new ArnNode( child);  // Add dummy node to child
    }
    WAbstractItemModel::endInsertRows();

    _app->triggerUpdate();

    if (child->isFolder()) {
        // qDebug() << "### arnModelW doInsertItem: Monitor path=" << child->path();
        _modelQ->arnMonStart( child);
    }
}


void  ArnModelW::doRemoveItem( ArnNode* itemNode)
{
    if (!itemNode)  return;

    ArnNode*  node = itemNode->_parent;
    if (!node)  return;
    WModelIndex  index = indexFromNode( node, 0);

    int  remRow = node->_children.indexOf( itemNode);
    if (remRow < 0)  return;  // Item not found

    beginRemoveRows( index, remRow, remRow);
    // qDebug() << "### arnModelW doRemoveItem: remove row=" << remRow;
    delete node->_children.at( remRow);  // Delete node
    node->_children.removeAt( remRow);
    endRemoveRows();

    _app->triggerUpdate();
}


void  ArnModelW::emitDataChanged( const ArnNode*  node)
{
    WModelIndex  index = indexFromNode( node, 1);

    emitDataChangedI( index);
}


void  ArnModelW::emitDataChangedI( const WModelIndex index)
{
    dataChanged().emit( index, index);
    _app->triggerUpdate();
}


ArnModelQ::ArnModelQ( ArnModelW* wt, QObject* parent) :
    QObject( parent)
{
    // qDebug() << "+++ arnModelQ Creator:";
    _wt = wt;

    _rootNode = new ArnNode("/", this);
    new ArnNode( _rootNode);  // Add dummy node to root

    _arnClient = QtMainThread::instance().arnClient();
}


void  ArnModelQ::arnMonStart( ArnNode* node)
{
    if (!node)  return;

    ArnMonitor*  arnMon = node->_arnMon;
    if (arnMon) {  // NetMon already exist
        // qDebug() << "+++ arnModelQ arnMonReStart: Monitor path=" << node->path();
        arnMon->reStart();
    }
    else {
        // qDebug() << "+++ arnModelQ arnMonStart: Monitor path=" << node->path();
        arnMon = new ArnMonitor( node);
        node->_arnMon = arnMon;
        connect( arnMon, SIGNAL(arnChildFound(QString)), this, SLOT(arnChildFound(QString)));
        arnMon->setMonitorPath( node->path(), _arnClient);
    }
}


void  ArnModelQ::arnChildFound( QString path)
{
    // qDebug() << "+++ arnModelQ netChildFound: path=" << path;
    ArnMonitor*  arnMon = qobject_cast<ArnMonitor*>( sender());
    Q_ASSERT( arnMon);

    ArnNode*  node = qobject_cast<ArnNode*>( arnMon->parent());
    Q_ASSERT( node);

    QString itemName = Arn::itemName( path);

    if (node->_expandState.e > ArnNode::Expand::Off) {
        WServer::instance()->post( _wt->_app->sessionId(),
                                   boost::bind( &ArnModelW::doInsertItem, _wt, node, itemName));
    }

    if (!node->_valueChild && (itemName == "value")) {
        //// Peek at child item "value"
        node->_valueChild = new ArnNode( path, node);
        connect( node->_valueChild, SIGNAL(changed()), node, SIGNAL(changed()));
        connect( node->_valueChild, SIGNAL(arnLinkDestroyed()), this, SLOT(destroyNode()));
        emitDataChanged( node);
    }
    else if (!node->_setChild && (itemName == "set")) {
        //// Peek at child item "set"
        node->_setMap   = new ArnNode::SetMap;
        node->_setChild = new ArnNode( path, node);
        connect( node->_setChild, SIGNAL(changed()), this, SLOT(updateSetMap()));
        connect( node->_setChild, SIGNAL(arnLinkDestroyed()), this, SLOT(destroyNode()));
        updateSetMap( node->_setChild);
    }
    else if (!node->_propChild && (itemName == "property")) {
        //// Peek at child item "property"
        node->_propMap   = new ArnNode::PropMap;
        node->_propChild = new ArnNode( path, node);
        connect( node->_propChild, SIGNAL(changed()), this, SLOT(updatePropMap()));
        connect( node->_propChild, SIGNAL(arnLinkDestroyed()), this, SLOT(destroyNode()));
        updatePropMap( node->_propChild);
    }
    else if (!node->_infoChild && (itemName == "info")) {
        //// Peek at child item "info"
        node->_infoChild = new ArnNode( path, node);
        connect( node->_infoChild, SIGNAL(changed()), node, SIGNAL(changed()));
        connect( node->_infoChild, SIGNAL(arnLinkDestroyed()), this, SLOT(destroyNode()));
        emitDataChanged( node);
    }
    else if (!node->_helpChild && (itemName.startsWith("help."))) {
        //// Peek at child item "help"
        node->_helpChild = new ArnNode( path, node);
        connect( node->_helpChild, SIGNAL(changed()), node, SIGNAL(changed()));
        connect( node->_helpChild, SIGNAL(arnLinkDestroyed()), this, SLOT(destroyNode()));
        emitDataChanged( node);
    }
}


void  ArnModelQ::nodeDataChanged()
{
    ArnNode*  node = qobject_cast<ArnNode*>( sender());
    if (!node)  return;

    // qDebug() << "+++ arnModelQ nodeDataChanged: path=" << node->path()
    //          << " SetAs=" << node->setAs().e
    //          << " QV-type=" << node->toVariant().typeName()
    //          << " QV-val=" << node->toVariant().toString();

    emitDataChanged( node);
}


void  ArnModelQ::updateSetMap( ArnNode* node_)
{
    ArnNode*  node = node_;
    if (!node)
        node = qobject_cast<ArnNode*>( sender());
    Q_ASSERT( node);
    ArnNode*  parent = qobject_cast<ArnNode*>( node->parent());
    Q_ASSERT( parent);
    Q_ASSERT( parent->_setMap);

    if (node != parent->_setChild)  return;

    parent->_setMap->fromXString( node->toByteArray());
    parent->_setMap->setEmptyKeysToValue();

    // qDebug() << "+++ arnModelQ updateSetMap: path=" << node->path()
    //          << " SetAs=" << node->setAs().e
    //          << " QV-type=" << node->toVariant().typeName()
    //          << " QV-val=" << node->toVariant().toString();

    emitDataChanged( parent);
}


void  ArnModelQ::updatePropMap( ArnNode* node_)
{
    ArnNode*  node = node_;
    if (!node)
        node = qobject_cast<ArnNode*>( sender());
    Q_ASSERT( node);
    ArnNode*  parent = qobject_cast<ArnNode*>( node->parent());
    Q_ASSERT( parent);
    Q_ASSERT( parent->_propMap);

    if (node != parent->_propChild)  return;

    parent->_propMap->fromXString( node->toByteArray());
    parent->_propMap->setEmptyKeysToValue();

    // qDebug() << "+++ arnModelQ updatePropMap: path=" << node->path()
    //          << " SetAs=" << node->setAs().e
    //          << " QV-type=" << node->toVariant().typeName()
    //          << " QV-val=" << node->toVariant().toString();

    emitDataChanged( parent);
}


void  ArnModelQ::destroyNode()
{
    ArnNode*  node = qobject_cast<ArnNode*>( sender());
    Q_ASSERT( node);
    // qDebug() << "+++ arnModelQ destroyNode: path=" << node->path();
    ArnNode*  parent = qobject_cast<ArnNode*>( node->parent());
    Q_ASSERT( parent);

    if (node == parent->_valueChild) { // Remove Value-child
        parent->_valueChild = 0;
        emitDataChanged( parent);
        delete node;
        return;
    }
    if (node == parent->_setChild) { // Remove Set-child
        delete parent->_setMap;
        parent->_setMap = 0;
        parent->_setChild = 0;
        emitDataChanged( parent);
        delete node;
        return;
    }
    if (node == parent->_propChild) { // Remove Property-child
        delete parent->_propMap;
        parent->_propMap = 0;
        parent->_propChild = 0;
        emitDataChanged( parent);
        delete node;
        return;
    }
    if (node == parent->_infoChild) { // Remove Info-child
        parent->_infoChild = 0;
        emitDataChanged( parent);
        delete node;
        return;
    }
    if (node == parent->_helpChild) { // Remove Help-child
        parent->_helpChild = 0;
        emitDataChanged( parent);
        delete node;
        return;
    }

    //// Remove normal child node
    parent->_arnMon->foundChildDeleted( node->path());  // Remove from found list
    WServer::instance()->post( _wt->_app->sessionId(),
                               boost::bind( &ArnModelW::doRemoveItem, _wt, node));
}


void  ArnModelQ::emitDataChanged( const ArnNode* node)
{
    WServer::instance()->post( _wt->_app->sessionId(), boost::bind( &ArnModelW::emitDataChanged, _wt, node));
}
