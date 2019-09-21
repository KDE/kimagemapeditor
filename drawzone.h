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

#ifndef DRAWZONE_H
#define DRAWZONE_H

#include <QCursor>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QResizeEvent>

#include "kimagemapeditor.h"

class Area;

/**
  *@short Draws the image and areas and handle the draw actions
  *@author Jan Sch&auml;fer
  *@internal
  *@see Area
  */
class DrawZone : public QWidget  {
public:

  DrawZone(QWidget *parent,KImageMapEditor* _imageMapEditor);
  ~DrawZone() override;

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

  QRect getImageRect() const { 
    return image.rect(); 
  }

  QSize sizeHint () const override;
  virtual QSize minimumSize () const;

protected:

  void mouseDoubleClickEvent(QMouseEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  //virtual void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*) override;//,int,int,int,int);
  void dropEvent(QDropEvent*) override;
  void dragEnterEvent(QDragEnterEvent*) override;
  
  /**
  * Represents whats currently going on
  * @li None : Nothing
  * @li DrawCircle : The user is drawing a circle
  * @li DrawRectangle : The user is drawing a rectangle
  * @li MoveSelectionPoint : The user is resizing an @ref Area or moving a polygon point
  * @li MoveArea : The user is moving an @ref Area
  * @li DoSelect : The user makes a selection rectangle
  */
  enum DrawAction { 
    None, 
    DrawCircle, 
    DrawRectangle, 
    DrawPolygon, 
    DrawFreehand, 
    MoveSelectionPoint, 
    MoveArea, 
    DoSelect, 
    RemovePoint, 
    AddPoint 
  };

  void createBorderRectangles(const QRect & r,QRect & rb,QRect & lb,QRect & tb,QRect & bb);

private:

  DrawAction currentAction;
  // The currently drawing area
  Area *currentArea;
  // Needed when moving selectionpoints
  SelectionPoint* currentSelectionPoint;
  // The point where the user clicked the mouse
  QPoint drawStart;
  QPoint drawCurrent;
  QPoint drawLast;
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

  QCursor rectangleCursor;
  QCursor circleCursor;
  QCursor polygonCursor;
  QCursor freehandCursor;
  QCursor addPointCursor;
  QCursor removePointCursor;

  void updateCursor(QPoint);
  void mouseMoveSelection(QPoint);
  void mouseMoveDrawCircle(QPoint);

  void mousePressNone(QMouseEvent*, QPoint startPoint, QPoint zoomedPoint);
  void mousePressRightNone(QMouseEvent*,QPoint);
  void mousePressLeftNone(QMouseEvent*, QPoint, QPoint);
  void mousePressLeftNoneOnArea(QMouseEvent*, Area*);
  void mousePressLeftNoneOnBackground(QMouseEvent*, QPoint); 

  QPoint moveIntoImage(QPoint);

  QCursor getCursorOfToolType(KImageMapEditor::ToolType);

};

inline QImage DrawZone::picture() const {
  return image;
}

#endif // DRAWZONE_H
