/***************************************************************************
                          imagemap.h  -  description
                            -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan Schäfer
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

#ifndef IMAGEMAP_H
#define IMAGEMAP_H

#include <q3scrollview.h>
#include <qimage.h>
#include <qpoint.h>
#include <qrect.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QDropEvent>

#include "kdeversion.h"

class KImageMapEditor;
class Area;

/**
  *@short Draws the image and areas and handle the draw actions
  *@author Jan Sch&auml;fer
  *@internal
  *@see Area
  */
class DrawZone : public Q3ScrollView  {
public:

  DrawZone(QWidget *parent,KImageMapEditor* _imageMapEditor);
  ~DrawZone();

  QImage picture() const;
  void repaintArea(const Area & a);
  void repaintRect(const QRect & r);
  void cancelDrawing();

  void setPicture(const QImage &_image);
  void setZoom(double z);

  QPoint translateFromZoom(const QPoint & p) const;
  QRect translateFromZoom(const QRect & p) const;
  QPoint translateToZoom(const QPoint & p) const;
  QRect translateToZoom(const QRect & p) const;

  QRect getImageRect() const { return image.rect(); }


protected:

  virtual void contentsMouseDoubleClickEvent(QMouseEvent*);
  virtual void contentsMousePressEvent(QMouseEvent*);
  virtual void contentsMouseReleaseEvent(QMouseEvent*);
  virtual void contentsMouseMoveEvent(QMouseEvent*);
  virtual void resizeEvent(QResizeEvent*);
  virtual void drawContents(QPainter*,int,int,int,int);
  virtual void viewportDropEvent(QDropEvent*);
  virtual void contentsDragEnterEvent(QDragEnterEvent*);
  virtual void contentsDropEvent(QDropEvent*);
  
  /**
  * Represents whats currently going on
  * @li None : Nothing
  * @li DrawCircle : The user is drawing a circle
  * @li DrawRectangle : The user is drawing a rectangle
  * @li MoveSelectionPoint : The user is resizing an @ref Area or moving a polygon point
  * @li MoveArea : The user is moving an @ref Area
  * @li DoSelect : The user makes a selection rectangle
  */
  enum DrawAction { None, DrawCircle, DrawRectangle, DrawPolygon, DrawFreehand, MoveSelectionPoint, MoveArea, DoSelect, RemovePoint, AddPoint };

  void createBorderRectangles(const QRect & r,QRect & rb,QRect & lb,QRect & tb,QRect & bb);

private:

  DrawAction currentAction;
  // The currently drawing area
  Area *currentArea;
  // Needed when moving selectionpoints
  QRect *currentSelectionPoint;
  // The point where the user clicked the mouse
  QPoint drawStart;
  QPoint drawCurrent;
  // The original image
  QImage image;
  KImageMapEditor *imageMapEditor;
  // Only the rect of the zoomed image, perhaps redundant
  QRect imageRect;
  // Only for repaint issues
  Area *oldArea;

  QRect oldSelectionRect;
  // Holds the zoomed image for efficiency reasons
  QPixmap zoomedImage;
  // The current zoom-factor
  double _zoom;

  QCursor RectangleCursor;
  QCursor CircleCursor;
  QCursor PolygonCursor;
  QCursor FreehandCursor;
  QCursor AddPointCursor;
  QCursor RemovePointCursor;
};

inline QImage DrawZone::picture() const {
  return image;
}


#endif
