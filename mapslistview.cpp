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
// KDE
#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>

// locale
#include "mapslistview.h"
//Added by qt3to4:
#include <Q3PtrList>


MapsListView::MapsListView(QWidget *parent, const char *name)
: Q3VBox(parent, name) {
    _listView = new KListView(this);
    _listView->addColumn(i18n("Maps"));
    _listView->setFullWidth(true);
    _listView->setSelectionMode(Q3ListView::Single);
    _listView->setItemsRenameable(true);

    connect( _listView, SIGNAL( selectionChanged(Q3ListViewItem*)),
             this, SLOT( slotSelectionChanged(Q3ListViewItem*)));

    connect( _listView, SIGNAL( itemRenamed(Q3ListViewItem*)),
             this, SLOT( slotItemRenamed(Q3ListViewItem*)));
}


MapsListView::~MapsListView() {
}

void MapsListView::addMap(const QString & name = QString::null) {
    new Q3ListViewItem(_listView,name);
    //kdDebug() << "MapsListView::addMap : Added map '" << name << "'" << endl;

}

void MapsListView::addMaps(Q3PtrList<MapTag> * maps) {

    for (MapTag *tag = maps->first(); tag!=0L; tag=maps->next()) {
        addMap(tag->name);
    }
}

void MapsListView::selectMap(const QString & name) {
    Q3ListViewItem* item = _listView->findItem(name,0);
    if (item) {
       selectMap(item);
    } else
       kdWarning() << "MapsListView::selectMap : Couldn't found map '" << name << "'" << endl;

}

void MapsListView::selectMap(Q3ListViewItem* item) {
    if (item)
        _listView->setSelected(item,true);
}


QString MapsListView::selectedMap() {
    QString result;

    Q3ListViewItem* item = _listView->selectedItem();
    if (item)
        result = item->text(0);
    else
        kdWarning() << "MapsListView::selectedMap : No map selected !" << endl;

    return result;
}

void MapsListView::removeMap(const QString & name) {
    Q3ListViewItem* item = _listView->findItem(name,0);
    if (item) {
        _listView->takeItem(item);
        _listView->setSelected(_listView->currentItem(),true);
//        kdDebug() << "MapsListView::removeMap : Removed map '" << name << "'" << endl;
    } else
        kdWarning() << "MapsListView::removeMap : Couldn't found map '" << name << "'" << endl;
}

void MapsListView::clear() {
    _listView->clear();
}

void MapsListView::slotSelectionChanged(Q3ListViewItem* item) {
    QString name = item->text(0);
    emit mapSelected(name);
}

void MapsListView::slotItemRenamed(Q3ListViewItem* item) {
    QString name = item->text(0);
    emit mapRenamed(name);
}

void MapsListView::changeMapName(const QString & oldName, const QString & newName) {
//    kdDebug() << "MapsListView::changeMapName : " << oldName << " to " << newName << endl;
    Q3ListViewItem* item = _listView->findItem(oldName,0);
    if (item) {
        item->setText(0,newName);
//        kdDebug() << "MapsListView::changeMapName : successful" << endl;
    }
    else {
        kdWarning() << "MapsListView::changeMapName : Chouldn't find map with name '" << oldName << "'" << endl;
    }

}


bool MapsListView::nameAlreadyExists(const QString & name) {
//    kdDebug() << "MapsListView::nameAlreadyExists : " << name << " ? " << endl;
    bool result = false;
    Q3ListViewItem* item = 0L;
    for(item = _listView->firstChild(); item; item = item->nextSibling()) {
        QString otherMap = item->text(0);
        if(name == otherMap) {
            result = true;
            break;
        }
    }

//    kdDebug() << "MapsListView::nameAlreadyExists : " << name << " : " << result << endl;

    return result;
}

QStringList MapsListView::getMaps() {
    QStringList result;
    
    Q3ListViewItem* item = 0L;
    for(item = _listView->firstChild(); item; item = item->nextSibling()) {
        QString map = item->text(0);
        result << map;
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

//    kdDebug() << "MapsListView::getUnusedMapName : Found an unused name : '" << result << "'" << endl;
    return result;
}

uint MapsListView::count() {
    return _listView->childCount();
}

#include "mapslistview.moc"
