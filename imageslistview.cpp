/***************************************************************************
                          imageslistview.cpp  -  description
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
#include "imageslistview.h"

// Qt
#include <QLinkedList>

// KDE Frameworks
#include <KLocalizedString>
#include "kimagemapeditor_debug.h"

ImagesListViewItem::ImagesListViewItem(ImagesListView* parent, ImageTag* tag)
  : QTreeWidgetItem(parent)
{
    _imageTag = tag;
    update();
}


void ImagesListViewItem::update() {
    QString src="";
    QString usemap="";
    if (_imageTag->contains("src"))
      src= _imageTag->value("src");
    if (_imageTag->contains("usemap"))
      usemap=_imageTag->value("usemap");

    setText(0,src);
    setText(1,usemap);
}

ImageTag* ImagesListViewItem::imageTag() {
  return _imageTag;
}


ImagesListView::ImagesListView(QWidget *parent)
  : QTreeWidget(parent)
{
  setColumnCount(2);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setHeaderLabels(QStringList() 
    << i18n("Images")
    << i18n("Usemap"));
  setRootIsDecorated(false);

  //listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  //listView->setSortingEnabled(false);
  //addColumn(i18n("Preview"));
//  setFullWidth(true);


  connect( this, SIGNAL(itemSelectionChanged()),
           this, SLOT(slotSelectionChanged()));
}


ImagesListView::~ImagesListView()
{
}

void ImagesListView::addImage(ImageTag* tag)
{
  new ImagesListViewItem(this, tag);
}

void ImagesListView::addImages(const QList<ImageTag*> & images)
{
    QListIterator<ImageTag*> it(images);
    while (it.hasNext()) {
        addImage(it.next());
	}
}

void ImagesListView::clear() {
  QTreeWidget::clear();
}

void ImagesListView::removeImage(ImageTag* tag) {
  ImagesListViewItem *item = findListViewItem(tag);
  if (item) {
     int i = invisibleRootItem()->indexOfChild(item);
     takeTopLevelItem(i);
     if (currentItem()) {
        currentItem()->setSelected(true);
     }     
  }
  else {
    qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageListView::removeImage: ListViewItem was not found !";
  }
}

void ImagesListView::updateImage(ImageTag* tag) {
  ImagesListViewItem *item = findListViewItem(tag);
  if (item)
    item->update();
  else {
    qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageListView::updateImage: ListViewItem was not found !";
  }
}

ImagesListViewItem* ImagesListView::findListViewItem(ImageTag* tag) {
  for (int i = 0; i < topLevelItemCount(); i++) {
    QTreeWidgetItem* item = topLevelItem(i);
    ImagesListViewItem *imageItem = static_cast<ImagesListViewItem*>(item);
    if (imageItem->imageTag() == tag) {
       qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageListView::findListViewItem: found it ";

       return imageItem;
    }
  }

  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageListView::findListViewItem: found nothing ";
  return nullptr;

}

void ImagesListView::slotSelectionChanged() {
  if (selectedItems().isEmpty())
    return;

  QTreeWidgetItem* item = selectedItems().first();
  QString src = item->text(0);

  if (_baseUrl.path().isEmpty() | !_baseUrl.path().endsWith('/')) {
      emit imageSelected(QUrl(_baseUrl.path() + '/').resolved(QUrl(src)));
  }
  else {
      emit imageSelected(_baseUrl.resolved(QUrl(src)));
  }
}

ImageTag* ImagesListView::selectedImage() {
  if (selectedItems().isEmpty()) {
     qCDebug(KIMAGEMAPEDITOR_LOG) << "ImagesListView::selectedImage: No Image is selected !";
     return nullptr;
  }

  ImagesListViewItem* item = static_cast<ImagesListViewItem*>(selectedItems().first());

  return item->imageTag();


}

void ImagesListView::selectImage(ImageTag* tag) {
  ImagesListViewItem* item = findListViewItem(tag);
  if (item) {
     item->setSelected(true);
  }
}

