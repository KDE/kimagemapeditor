/***************************************************************************
                          imagemapchoosedialog.h  -  description
                            -------------------
    begin                : 06-03-2007
    copyright            : (C) 2007 by Jan Schäfer
    email                : j_schaef@informatik.uni-kl.de
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KIMAGEMAPCHOOSEDIALOG_H
#define KIMAGEMAPCHOOSEDIALOG_H

#include <QDialog>
#include <QLinkedList>
#include <QUrl>

#include "kimagemapeditor.h"

class QLineEdit;
class QListWidget;
class QLabel;
class QLayout;
class QTableWidget;


class ImageMapChooseDialog : public QDialog {
Q_OBJECT
  private:
    QTableWidget *imageListTable;
    QLabel *imagePreview;		
    QListWidget *mapListBox;	
    QLineEdit *mapNameEdit;
    QList<MapTag*> maps;
    QList<ImageTag*> images;
    QUrl baseUrl;
    void initImageListTable(QLayout*);
 public:
    ImageMapChooseDialog(QWidget* parent,QList<MapTag*> _maps,
			 QList<ImageTag*> _images, const QUrl & _baseUrl);
    ~ImageMapChooseDialog() override;
    QUrl pixUrl;
    MapTag* currentMap;
 protected slots:
   void slotImageChanged();
   void slotMapChanged(int i);
   void selectImageWithUsemap(const QString & usemap);
};


#endif
