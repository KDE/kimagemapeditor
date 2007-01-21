/***************************************************************************
                          arealistview.h  -  description
                             -------------------
    begin                : Weg Feb 26 2003
    copyright            : (C) 2003 by Jan SchÃ¤fer
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

#ifndef _AREALISTVIEW_H_
#define _AREALISTVIEW_H_

#include <QTreeWidget>
#include <qwidget.h>
#include <kvbox.h>


class QPushButton;
class Area;

/**
 * This class consists of a ListView and two arrow buttons on the bottom.
 * It shows all Areas of the current map.
 * Jan Schaefer
 **/
class AreaListView : public KVBox
{
  Q_OBJECT

public:
  explicit AreaListView(QWidget *parent);
  ~AreaListView();

  QTreeWidget* listView;
	QPushButton *upBtn;
	QPushButton *downBtn;

};

#endif
