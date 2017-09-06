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

// Qt
#include <qpushbutton.h>
#include <qtooltip.h>
#include <QTreeWidget>
#include <QVBoxLayout>

// KDE Frameworks
#include <kiconloader.h>
#include <klocalizedstring.h>

// local
#include "kimearea.h"
#include "arealistview.h"


AreaListView::AreaListView(QWidget *parent)
  : QWidget(parent)
{
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setMargin(0);

  listView = new QTreeWidget(this);
  listView->setColumnCount(2);
  listView->setHeaderLabels(QStringList() 
    << i18n("Areas")
    << i18n("Preview"));
  listView->setRootIsDecorated(false);
  listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  listView->setSortingEnabled(false);
  // FIXME:
  //listView->setFullWidth(true);
  /*listView->setWhatsThis( i18n("<h3>Area List</h3>The area list shows you all areas of the map.<br>"
                                  "The left column shows the link associated with the area; the right "
                                  "column shows the part of the image that is covered by the area.<br>"
                                  "The maximum size of the preview images can be configured."));
    */                                  
  //listView->setToolTip( i18n("A list of all areas"));
  mainLayout->addWidget(listView);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  upBtn = new QPushButton;
  upBtn->setIcon(SmallIcon("go-up"));
  buttonsLayout->addWidget(upBtn);

  downBtn = new QPushButton;
  downBtn->setIcon(SmallIcon("go-down"));
  buttonsLayout->addWidget(downBtn);

  mainLayout->addLayout(buttonsLayout);

}


AreaListView::~AreaListView()
{
}

#include "arealistview.moc"
