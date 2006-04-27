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

// QT
#include <q3ptrlist.h>
 
 
// KDE
#include <klocale.h> 
#include <kdebug.h>

// locale 
#include "imageslistview.h"

ImagesListViewItem::ImagesListViewItem(ImagesListView* parent, ImageTag* tag) 
  : Q3ListViewItem(parent) 
{
    _imageTag = tag;
    update();
}


void ImagesListViewItem::update() {
    QString src="";
    QString usemap="";
    if (_imageTag->find("src"))
      src=*_imageTag->find("src");
    if (_imageTag->find("usemap"))
      usemap=*_imageTag->find("usemap");

    setText(0,src);
    setText(1,usemap); 
}

ImageTag* ImagesListViewItem::imageTag() {
  return _imageTag;
}


ImagesListView::ImagesListView(QWidget *parent, const char *name)
  : K3ListView(parent, name)
{
  addColumn(i18n("Images"));
  addColumn(i18n("Usemap"));
  //addColumn(i18n("Preview"));
  setFullWidth(true);
  
  
  connect( this, SIGNAL( selectionChanged(Q3ListViewItem*)),
           this, SLOT( slotSelectionChanged(Q3ListViewItem*)));
}


ImagesListView::~ImagesListView()
{
}

void ImagesListView::addImage(ImageTag* tag)
{
  if (!tag) {
    kDebug() << "ImageListView::addImage: Parameter is null !" << endl;
    return;
  }

  new ImagesListViewItem(this, tag);
}

void ImagesListView::addImages(Q3PtrList<ImageTag> * images)
{
	for (ImageTag *tag = images->first(); tag!=0L; tag=images->next()) {
    addImage(tag);
	}
}

void ImagesListView::clear() {
  Q3ListView::clear();
}

void ImagesListView::removeImage(ImageTag* tag) {
  ImagesListViewItem *item = findListViewItem(tag);
  if (item) {
     takeItem(item);
     setSelected(currentItem(),true);
  }
  else {
    kDebug() << "ImageListView::removeImage: ListViewItem was not found !" << endl;
  }
}

void ImagesListView::updateImage(ImageTag* tag) {
  ImagesListViewItem *item = findListViewItem(tag);
  if (item)
    item->update();
  else {
    kDebug() << "ImageListView::updateImage: ListViewItem was not found !" << endl;
  }     
}

ImagesListViewItem* ImagesListView::findListViewItem(ImageTag* tag) {
  
  kDebug() << "ImageListView::findListViewItem: start searching ... " << endl;

  for (Q3ListViewItem* item = firstChild(); item ; item = item->nextSibling()) {
     ImagesListViewItem *imageItem = static_cast<ImagesListViewItem*>(item);
     if (imageItem->imageTag() == tag) {
        kDebug() << "ImageListView::findListViewItem: found it " << endl;
     
        return imageItem;
     }        
  } 
  
  kDebug() << "ImageListView::findListViewItem: found nothing " << endl;
  return 0L;
  
}

void ImagesListView::slotSelectionChanged(Q3ListViewItem* item) {
  QString src = item->text(0);
  
  emit imageSelected(KUrl(_baseUrl,src));
}

ImageTag* ImagesListView::selectedImage() {
  ImagesListViewItem* item = static_cast<ImagesListViewItem*>(selectedItem());
  if ( ! item) {
     kDebug() << "ImagesListView::selectedImage: No Image is selected !" << endl;
     return 0L;
  }
  
  return item->imageTag();
     
  
}

void ImagesListView::selectImage(ImageTag* tag) {
  ImagesListViewItem* item = findListViewItem(tag);
  if (item) {
     setSelected(item, true);
  }
}

#include "imageslistview.moc"

