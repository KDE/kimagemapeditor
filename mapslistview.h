/***************************************************************************
                          mapslistview.h  -  description
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

#ifndef _MAPSLISTVIEW_H_
#define _MAPSLISTVIEW_H_

#include <QLinkedList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include "kimagemapeditor.h"

/**
 * Simple class that shows all map tags of the current open html file in a ListView
 *
 * Jan Schaefer
 **/
class MapsListView : public QWidget
{
Q_OBJECT
public:
  explicit MapsListView(QWidget *parent);
  ~MapsListView() override;

  /**
   * Adds the given map to the ListView
   */
  void addMap(const QString &);

  /**
   * Adds all maps of the given QList to the ListView
   */
  void addMaps(const QList<MapTag*> &);

  /**
   * Removes the given map from the ListView
   */
  void removeMap(const QString &);

  /**
   * Set to the given map selected in the ListView.
   * it does not emit mapSelected afterwards.
   */
  void selectMap(const QString &);

  /**
   * Selects the given ListViewItem and deselects the current selected item
   */
  void selectMap(QTreeWidgetItem* item);

  /**
   * Changes the name of the map with the @p oldName to @p newName
   */
  void changeMapName(const QString & oldName, const QString & newName);

  /**
   * Returns the current selected map
   */
  QString selectedMap();


  /**
   * Removes all maps from the ListView
   */
  void clear();

  /**
   * Returns a name for a map which is not used yet.
   * Returns for example Unnamed1
   */
  QString getUnusedMapName();

  /**
   * Whether or not the given map name already exists
   */
  bool nameAlreadyExists(const QString &);

  /**
   * Returns a QStringList of all maps
   */
  QStringList getMaps();

  /**
   * Returns the number of maps
   */
  int count();

  QTreeWidget* listView() { return _listView; }
protected:
  QTreeWidget* _listView;

protected slots:
  void slotSelectionChanged();
  void slotItemRenamed(QTreeWidgetItem*);

signals:

  /**
   * Gets emitted when the user selects a map in
   * the ListView
   */
  void mapSelected(const QString &);


  /**
   * Emitted when the user has renamed a map in the ListView
   */
  void mapRenamed(const QString & newName);


};

#endif
