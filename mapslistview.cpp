/***************************************************************************
                          mapslistview.cpp  -  description
                             -------------------
    begin                : Weg Feb 26 2003
    copyright            : (C) 2003 by Jan SchÃ?fer
    email                : janschaefer@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QListWidget>

// KDE
#include <klocale.h>
#include <kdebug.h>

// locale
#include "mapslistview.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <kvbox.h>


MapsListView::MapsListView(QWidget *parent)
: KVBox(parent) {
    _listView = new QTreeWidget(this);
    _listView->setColumnCount(1);
    _listView->setHeaderLabel(i18n("Maps"));
    _listView->setRootIsDecorated(false);
//FIXME:    _listView->setFullWidth(true);
//    _listView->setItemsRenameable(true);
  _listView->setSelectionMode(QAbstractItemView::SingleSelection);
  _listView->setSortingEnabled(false);
    

    connect( _listView, SIGNAL( itemSelectionChanged()),
             this, SLOT( slotSelectionChanged()));

  connect( _listView, SIGNAL( itemChanged( QTreeWidgetItem*,int)),
           this, SLOT( slotItemRenamed(QTreeWidgetItem*)));
}


MapsListView::~MapsListView() {
}

void MapsListView::addMap(const QString & name = QString::null) {
    new QTreeWidgetItem(_listView,QStringList() << name);
    //kDebug() << "MapsListView::addMap : Added map '" << name << "'" << endl;

}

void MapsListView::addMaps(const QList<MapTag*> & maps) {
    QListIterator<MapTag*> it(maps);
    while (it.hasNext()) {
        addMap(it.next()->name);
    }
}

void MapsListView::selectMap(const QString & name) {
    QList<QTreeWidgetItem *> items = _listView->findItems(name,Qt::MatchExactly);
    if (items.count()>0) {
       selectMap(items[0]);
    } else {
       kWarning() << "MapsListView::selectMap : Couldn't found map '" << name << "'" << endl;
    }

}

void MapsListView::selectMap(QTreeWidgetItem* item) {
    if (item) {
        item->setSelected(true);
    }
}


QString MapsListView::selectedMap() {
    QString result;

    QList<QTreeWidgetItem *> items = _listView->selectedItems();
    if (items.count()>0)
        result = items[0]->text(0);
    else
        kWarning() << "MapsListView::selectedMap : No map selected !" << endl;

    return result;
}

void MapsListView::removeMap(const QString & name) {
    QList<QTreeWidgetItem *> items = _listView->findItems(name,Qt::MatchExactly);
    if (items.count()>0) {
        int i = _listView->invisibleRootItem()->indexOfChild(items[0]);
        _listView->takeTopLevelItem(i);
        if (_listView->currentItem())
            _listView->currentItem()->setSelected(true);
//        kDebug() << "MapsListView::removeMap : Removed map '" << name << "'" << endl;
    } else
        kWarning() << "MapsListView::removeMap : Couldn't found map '" << name << "'" << endl;
}

void MapsListView::clear() {
    _listView->clear();
}

void MapsListView::slotSelectionChanged() {
    QList<QTreeWidgetItem *> list = _listView->selectedItems();
    if (list.count()>0) {
        QString name = list[0]->text(0);
        emit mapSelected(name);
    }
}

void MapsListView::slotItemRenamed(QTreeWidgetItem* item) {
    QString name = item->text(0);
    emit mapRenamed(name);
}

void MapsListView::changeMapName(const QString & oldName, const QString & newName) {
//    kDebug() << "MapsListView::changeMapName : " << oldName << " to " << newName << endl;
    QList<QTreeWidgetItem *> items = _listView->findItems(oldName,Qt::MatchExactly);
    if (items.count()>0) {
        items[0]->setText(0,newName);
//        kDebug() << "MapsListView::changeMapName : successful" << endl;
    }
    else {
        kWarning() << "MapsListView::changeMapName : Chouldn't find map with name '" << oldName << "'" << endl;
    }

}


bool MapsListView::nameAlreadyExists(const QString & name) {
    return _listView->findItems(name, Qt::MatchExactly).count() > 0;  
}

QStringList MapsListView::getMaps() {
    QStringList result;

    for (int i=0; i<_listView->topLevelItemCount(); i++) {
         result << _listView->topLevelItem(i)->text(0);
    }

    return result;
}

QString MapsListView::getUnusedMapName() {
    QString result;
    QString attempt;
    int i=0;
    while(result.isEmpty()) {
        i++;
        attempt = i18n("unnamed");
        attempt += QString::number(i);
        if (nameAlreadyExists(attempt))
            continue;

        result = attempt;
    }

//    kDebug() << "MapsListView::getUnusedMapName : Found an unused name : '" << result << "'" << endl;
    return result;
}

int MapsListView::count() {
    return _listView->topLevelItemCount();
}

#include "mapslistview.moc"
