/***************************************************************************
                          imagemap.h  -  description
                             -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan Sch�fer
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

#include <qscrollview.h>
#include <qimage.h>
#include <qpoint.h>
#include <qrect.h>

#include "kdeversion.h"

/**
  *@author Jan Sch�fer
  */
class KImageMapEditor;
class Area;

class ImageMap : public QScrollView  {
public:
	enum DrawAction { None, DrawCircle, DrawRectangle, DrawPolygon, MoveSelectionPoint, MoveArea };
private:
	QRect imageRect;
	QPoint drawStart;
	QPoint drawCurrent;
	QPoint drawEnd;
	bool eraseOldArea;
	Area *oldArea;
	// Holds the original image
	QImage image;
	// Holds the zoomed image for efficiency reasons
	QPixmap zoomedImage;
	Area *currentArea;
	DrawAction currentAction;
	QRect *currentSelectionPoint;
	KImageMapEditor *imageMapEditor;
	double _zoom;
public:
	ImageMap(QWidget *parent,KImageMapEditor* _imageMapEditor);
	~ImageMap();
	void setZoom(double z);
	void setPicture(const QImage &_image);
	void repaintArea(const Area & a);
	QImage picture() const;
	QPoint translateFromZoom(const QPoint & p) const;
	QPoint translateToZoom(const QPoint & p) const;
	QRect translateToZoom(const QRect & p) const;
protected:
	virtual void contentsMousePressEvent(QMouseEvent* e);
	virtual void contentsMouseDoubleClickEvent(QMouseEvent* e);
	virtual void contentsMouseReleaseEvent(QMouseEvent *e);
	virtual void contentsMouseMoveEvent(QMouseEvent *e);
	virtual void resizeEvent(QResizeEvent* e);
	virtual void drawContents(QPainter* p,int clipx,int clipy,int clipw,int cliph);
};

inline QImage ImageMap::picture() const {
	return image;
}


#endif
