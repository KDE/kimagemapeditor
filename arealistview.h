/***************************************************************************
                          arealistview.h  -  description
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

#ifndef _AREALISTVIEW_H_
#define _AREALISTVIEW_H_

#include <qwidget.h>
#include <q3vbox.h>

class K3ListView;
class QPushButton;
class Area;

/**
 * This class consists of a ListView and two arrow buttons on the bottom.
 * It shows all Areas of the current map.
 * Jan Schaefer
 **/
class AreaListView : public Q3VBox
{
  Q_OBJECT

public:
  AreaListView(QWidget *parent, const char *name);
  ~AreaListView();
  
  K3ListView* listView;
	QPushButton *upBtn;
	QPushButton *downBtn;
  
};

#endif
