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

#ifndef ARNMODEL_HPP
#define ARNMODEL_HPP

#include <Wt/WAbstractItemModel>

#include "ItemDataRole.hpp"
#include <ArnInc/ArnItem.hpp>
#include <ArnInc/MQFlags.hpp>

class ArnMonitor;
class ArnClient;
class ArnModelW;

namespace Arn {
class XStringMap;
}


class ArnNode : public ArnItem
{
Q_OBJECT
public:
    struct Expand {
        enum E {
            Off = 0,
            Pre = 1,
            On  = 2
        };
        MQ_DECLARE_ENUM( Expand)
    };
    ArnNode( ArnNode* parent);
    ArnNode( const QString &path, QObject *qobjParent = 0);
    ArnNode( ArnNode* parent, const QString& item, int row);
    ~ArnNode();
    QString  name( Arn::NameF nameF)  const;
    QString  path()  const;
    void  init();

    ArnNode*  _parent;
    ArnNode*  _valueChild;
    ArnNode*  _setChild;
    ArnNode*  _propChild;
    ArnNode*  _infoChild;
    ArnNode*  _helpChild;
    ArnMonitor*  _arnMon;
    typedef Arn::XStringMap  SetMap;
    typedef Arn::XStringMap  PropMap;
    SetMap*  _setMap;
    PropMap*  _propMap;

    Expand   _expandState;
    QList<ArnNode*>  _children;
};


using namespace Wt;

namespace Wt {
    class WStandardItem;
    class WApplication;
}


class ArnModelQ : public QObject
{
Q_OBJECT
public:
    explicit ArnModelQ( ArnModelW* wt, QObject* parent = 0);

    void  arnMonStart( ArnNode* node);

    ArnNode*  _rootNode;
    ArnClient*  _arnClient;

signals:

public slots:
    void  arnChildFound( QString path);
    void  nodeDataChanged();
    void  updateSetMap( ArnNode* node = 0);
    void  updatePropMap( ArnNode* node = 0);
    void  destroyNode();

private:
    void  emitDataChanged( const ArnNode*  node);

    ArnModelW*  _wt;
};


class ArnModelW : public WAbstractItemModel
{
public:
    ArnModelW(WObject *parent = 0);
    ~ArnModelW();

    void setApplication( WApplication* app);

    WModelIndex  indexFromNode( const ArnNode* node, int column)  const;
    ArnNode*  nodeFromIndex( const WModelIndex& index)  const;

    using WAbstractItemModel::setData;
    using WAbstractItemModel::data;

    virtual WFlags<ItemFlag> flags(const WModelIndex& index) const;
    virtual WModelIndex parent(const WModelIndex& index) const;

    virtual boost::any data(const WModelIndex& index, int role = DisplayRole)  const;
    boost::any  adjustedNodeData( const ArnNode *node, int role = DisplayRole)  const;

    virtual boost::any headerData(int section,
                                Orientation orientation = Horizontal,
                                int role = DisplayRole) const;

    virtual WModelIndex index(int row, int column,
                            const WModelIndex& parent = WModelIndex()) const;

    virtual int columnCount(const WModelIndex& parent = WModelIndex()) const;

    virtual int rowCount(const WModelIndex& parent = WModelIndex()) const;

    virtual bool setData(const WModelIndex& index, const boost::any& value,
                       int role = EditRole);

    virtual void *toRawIndex(const WModelIndex& index) const;
    virtual WModelIndex fromRawIndex(void *rawIndex) const;
    bool  hasChildren( const WModelIndex& index)  const;
    bool  hasIndex( int row, int column, const WModelIndex &parent=WModelIndex()) const;
    void  fetchMore( const WModelIndex& parent)  const;
    void  doInsertItem( ArnNode* node, QString itemName);
    void  doRemoveItem( ArnNode* itemNode);
    void  emitDataChangedI( const WModelIndex index);
    void  emitDataChanged( const ArnNode* node);

protected:

private:
    typedef std::map<int, boost::any> HeaderData;

    void init();
    friend class ArnModelQ;

    ArnModelQ*  _modelQ;
    WApplication*  _app;
};


#endif // ARNMODEL_HPP
