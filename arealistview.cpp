/***************************************************************************
                          arealistview.cpp  -  description
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

// QT
#include <k3listview.h>
#include <qpushbutton.h>
#include <q3hbox.h>
#include <q3whatsthis.h>
#include <qtooltip.h>

// KDE
#include <kiconloader.h>
#include <klocale.h>

// local
#include "kimearea.h"
#include "arealistview.h"


AreaListView::AreaListView(QWidget *parent, const char *name)
  : Q3VBox(parent, name)
{
  listView = new K3ListView(this);
  listView->addColumn(i18n("Areas"));
  listView->addColumn(i18n("Preview"));

  listView->setMultiSelection(true);
  listView->setSelectionMode( Q3ListView::Extended );
  listView->setSorting(-1); // The user can't sort by clicking on the header
  listView->setFullWidth(true);


  Q3WhatsThis::add( listView, i18n("<h3>Area List</h3>The area list shows you all areas of the map.<br>"
                                  "The left column shows the link associated with the area; the right "
                                  "column shows the part of the image that is covered by the area.<br>"
                                  "The maximum size of the preview images can be configured."));
  QToolTip::add( listView, i18n("A list of all areas"));

  Q3HBox *hbox= new Q3HBox(this);
  upBtn= new QPushButton("",hbox);
  upBtn->setIconSet(SmallIconSet("up"));

  downBtn= new QPushButton("",hbox);
  downBtn->setIconSet(SmallIconSet("down"));

}


AreaListView::~AreaListView()
{
}

#include "arealistview.moc"
