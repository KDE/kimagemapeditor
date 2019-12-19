/***************************************************************************
                          mapslistview.cpp  -  description
                             -------------------
    begin                : Weg Feb 26 2003
    copyright            : (C) 2003 by Jan Schäfer
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
// local
#include "mapslistview.h"

#include <QListWidget>
#include <QVBoxLayout>

// KDE Frameworks
#include <KLocalizedString>
#include "kimagemapeditor_debug.h"


MapsListView::MapsListView(QWidget *parent)
: QWidget(parent) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  _listView = new QTreeWidget(this);
  _listView->setColumnCount(1);
  _listView->setHeaderLabel(i18n("Maps"));
  _listView->setRootIsDecorated(false);
//FIXME:    _listView->setFullWidth(true);
//    _listView->setItemsRenameable(true);
  _listView->setSelectionMode(QAbstractItemView::SingleSelection);
  _listView->setSortingEnabled(false);
  mainLayout->addWidget(_listView);

  connect( _listView, SIGNAL(itemSelectionChanged()),
           this, SLOT(slotSelectionChanged()));

  connect( _listView, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this, SLOT(slotItemRenamed(QTreeWidgetItem*)));
}


MapsListView::~MapsListView() {
}

void MapsListView::addMap(const QString & name = "") {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::addMap: " << name;
  QStringList list(name);
  new QTreeWidgetItem(_listView,list);
    //qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::addMap : Added map '" << name << "'";

}

void MapsListView::addMaps(const QList<MapTag*> & maps) {
    QListIterator<MapTag*> it(maps);
    while (it.hasNext()) {
      MapTag *tag = it.next();
      QString s = tag->name;
      qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::addMaps:" << s;
      addMap(s);
    }
}

void MapsListView::selectMap(const QString & name) {
    QList<QTreeWidgetItem *> items = _listView->findItems(name,Qt::MatchExactly);
    if (items.count()>0) {
       selectMap(items[0]);
    } else {
       qCWarning(KIMAGEMAPEDITOR_LOG) << "MapsListView::selectMap : Couldn't found map '" << name << "'";
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
        qCWarning(KIMAGEMAPEDITOR_LOG) << "MapsListView::selectedMap : No map selected !";

    return result;
}

void MapsListView::removeMap(const QString & name) {
    QList<QTreeWidgetItem *> items = _listView->findItems(name,Qt::MatchExactly);
    if (items.count()>0) {
        int i = _listView->invisibleRootItem()->indexOfChild(items[0]);
        _listView->takeTopLevelItem(i);
        if (_listView->currentItem())
            _listView->currentItem()->setSelected(true);
//        qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::removeMap : Removed map '" << name << "'";
    } else
        qCWarning(KIMAGEMAPEDITOR_LOG) << "MapsListView::removeMap : Couldn't found map '" << name << "'";
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
//    qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::changeMapName : " << oldName << " to " << newName;
    QList<QTreeWidgetItem *> items = _listView->findItems(oldName,Qt::MatchExactly);
    if (items.count()>0) {
        items[0]->setText(0,newName);
//        qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::changeMapName : successful";
    }
    else {
        qCWarning(KIMAGEMAPEDITOR_LOG) << "MapsListView::changeMapName : Couldn't find map with name '" << oldName << "'";
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

//    qCDebug(KIMAGEMAPEDITOR_LOG) << "MapsListView::getUnusedMapName : Found an unused name : '" << result << "'";
    return result;
}

int MapsListView::count() {
    return _listView->topLevelItemCount();
}

