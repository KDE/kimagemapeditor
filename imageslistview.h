/***************************************************************************
                          imageslistview.h  -  description
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

#ifndef _IMAGESLISTVIEW_H_
#define _IMAGESLISTVIEW_H_

#include <QLinkedList>
#include <QTreeWidget>
#include <QUrl>

#include "kimagemapeditor.h"

class ImagesListView;

class ImagesListViewItem : public QTreeWidgetItem
{
  public:
    ImagesListViewItem(ImagesListView*, ImageTag*);
    ImageTag* imageTag();

    /**
     * Re-reads the contents of the ImageTag and updates
     * itself accordingly
     */
    void update();
  protected:
    ImageTag* _imageTag;
};

/**
 * Simple class that shows a list of imagenames with a preview
 * Jan Schaefer
 **/
class ImagesListView : public QTreeWidget
{
  Q_OBJECT

public:
  explicit ImagesListView(QWidget *parent);
  ~ImagesListView() override;

  /**
   * Adds an image
   */
  void addImage(ImageTag*);

  /**
   * Adds images
   */
  void addImages(const QList<ImageTag*> &);

  /**
   * Removes the given image from the list
   */
  void removeImage(ImageTag*);

  /**
   * Updates the listview item with the given ImageTag
   */
  void updateImage(ImageTag *);

  /**
   * Removes all images
   */
  void clear();

  /**
   * Returns the filename of the current selected Image
   */
  ImageTag* selectedImage();

  /**
   * Selects the given image
   */
  void selectImage(ImageTag*);

  /**
   * Sets the base URL of all images
   */
  void setBaseUrl(const QUrl & url) { _baseUrl = url; };

protected slots:
  void slotSelectionChanged();

signals:
  void imageSelected(const QUrl &);

protected:
  QUrl _baseUrl;

  /**
   * Finds the first ImageListViewItem with the given ImageTag
   * Returns 0L if no item was found
   */
  ImagesListViewItem* findListViewItem(ImageTag*);
};

#endif
