/***************************************************************************
                          imagemap.cpp  -  description
                             -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan Schäfer
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

#include "imagemap.h"
#include "kimagemapeditor.h"

#include <QBitmap>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

int round(double d) {
	if ( (d-((int) d)) < 0.5 )
		return (int) d;
	else
		return ((int) d)+1;
}

ImageMap::ImageMap(QWidget *parent,KImageMapEditor* _imageMapEditor)
	: QScrollArea(parent)
{
	imageMapEditor=_imageMapEditor;
//	setPicture(QImage());
	currentAction=None;
	currentArea=0L;
	eraseOldArea=false;
	oldArea=0L;
	_zoom=1;
	widget()->setMouseTracking(true);


}

ImageMap::~ImageMap(){
}

void ImageMap::setPicture(const QImage &_image) {
	image=_image;
	zoomedImage.convertFromImage(image);
	setZoom(_zoom);
}

void ImageMap::setZoom(double z) {
	_zoom=z;
	imageRect.setHeight(image.height()*_zoom);
	imageRect.setWidth(image.width()*_zoom);
	zoomedImage=QPixmap(imageRect.width(),imageRect.height());
	QPainter p(&zoomedImage);
	p.scale(z,z);
	QPixmap pix;
	pix.convertFromImage(image);
	// if the picture has transparent areas,
	// fill them with Gimp like background
	if (pix.mask()) {
  	QPixmap backPix(32,32);
  	QPainter p2(&backPix);
  	p2.fillRect(0,0,32,32,QColor(156,149,156));
  	p2.fillRect(0,16,16,16,QColor(98,105,98));
  	p2.fillRect(16,0,16,16,QColor(98,105,98));
  	p2.flush();
  	p.setPen(QPen());
  	p.fillRect(imageRect.left(),imageRect.top(),imageRect.width(),imageRect.height(),QBrush(QColor("black"),backPix));
	}
	p.drawPixmap(imageRect.left(),imageRect.top(),pix);
	p.flush();
	resizeContents(visibleWidth()>imageRect.width() ? visibleWidth() : imageRect.width(),
								 visibleHeight()>imageRect.height() ? visibleHeight() : imageRect.height());
	repaintContents(0,0,contentsWidth(),contentsHeight(),true);
}

QPoint ImageMap::translateFromZoom(const QPoint & p) const {
	return QPoint(p.x()/_zoom,p.y()/_zoom);
}

QPoint ImageMap::translateToZoom(const QPoint & p) const {
	return QPoint(round(p.x()*_zoom),round(p.y()*_zoom));
}

QRect ImageMap::translateToZoom(const QRect & r) const {
	return QRect(round(r.x()*_zoom),round(r.y()*_zoom),
							 round(r.width()*_zoom),round(r.height()*_zoom));
}

void ImageMap::mouseDoubleClickEvent(QMouseEvent* e) {
	QPoint point=e->pos();
	point-=imageRect.topLeft();
	point=translateFromZoom(point);
	if ( currentAction==None &&
		(currentArea=imageMapEditor->onArea(point)))
		imageMapEditor->showTagEditor(currentArea);

}

void ImageMap::mousePressEvent(QMouseEvent* e) {
	drawStart=e->pos();
	// Check if it's on picture if not
	// move it to the picture's border
	if (!imageRect.contains(drawStart)) {
		if (drawStart.x()>imageRect.right())
			drawStart.setX(imageRect.right());
		if (drawStart.x()<imageRect.left())
			drawStart.setX(imageRect.left());
		if (drawStart.y()>imageRect.bottom())
			drawStart.setY(imageRect.bottom());
		if (drawStart.y()<imageRect.top())
			drawStart.setY(imageRect.top());
	}

	// Translate it to picture coordinates
	drawStart-=imageRect.topLeft();
	drawStart=translateFromZoom(drawStart);
	if (currentArea)
		oldArea=new Area(*currentArea);

	if ( currentAction==None ) {
		if (e->button()==Qt::RightButton) {
			currentArea=imageMapEditor->onArea(drawStart);
			imageMapEditor->select(currentArea);
			imageMapEditor->slotShowPopupMenu(e->globalPos());
		} else
		if ((currentArea=imageMapEditor->selected()) &&
			(currentSelectionPoint=currentArea->onSelectionPoint(drawStart)))
		{
				currentAction=MoveSelectionPoint;
		} else
		if ((currentArea=imageMapEditor->onArea(drawStart))) {
			currentAction=MoveArea;
			imageMapEditor->select(currentArea);
		} else
		if (imageMapEditor->currentShapeType()!=Area::None) {
			currentArea=new Area(imageMapEditor->currentShapeType());
 		  currentArea->setRect(QRect(drawStart,drawStart));
 		  currentArea->setSelected(false);
 		  if (imageMapEditor->selected())
 		  	imageMapEditor->selected()->setSelected(false);
			switch (currentArea->type()) {
				case Area::Rectangle : currentAction=DrawRectangle; break;
				case Area::Circle : currentAction=DrawCircle; break;
				case Area::Polygon :
  					currentAction=DrawPolygon;
      			currentArea->addCoord(drawStart);
      			currentSelectionPoint=currentArea->selectionPoints()->last();

					break;
				default: break;
			}
		}
  	// Clicked with the arrow at an areafree position
  	else {
  	  	currentArea=0L;
  	  	imageMapEditor->deselectAll();
  	}
	} else
	if ( currentAction==DrawPolygon) {

	}

	QRect r;
	if (oldArea)
		r=oldArea->selectionRect();
	if (currentArea) {
		r= r | currentArea->selectionRect();
		repaintContents(translateToZoom(r),false);
	}

}

void ImageMap::mouseReleaseEvent(QMouseEvent *e) {
	drawEnd=e->pos();

	// Check if it's on picture if not
	// move it to the picture's border
	if (!imageRect.contains(drawEnd)) {
		if (drawEnd.x()>imageRect.right())
			drawEnd.setX(imageRect.right());
		if (drawEnd.x()<imageRect.left())
			drawEnd.setX(imageRect.left());
		if (drawEnd.y()>imageRect.bottom())
			drawEnd.setY(imageRect.bottom());
		if (drawEnd.y()<imageRect.top())
			drawEnd.setY(imageRect.top());
	}
	// Translate it to picture coordinates
	drawEnd-=imageRect.topLeft();
	drawEnd=translateFromZoom(drawEnd);

	if (currentAction==DrawCircle || currentAction==DrawRectangle) {
   		imageMapEditor->addArea(currentArea);
   		imageMapEditor->select(currentArea);
   		//imageMapEditor->slotAreaChanged(currentArea);
  		currentAction=None;
  } else
	if (currentAction==DrawPolygon) {
		// If the number of Polygonpoints is more than 2
		// and clicked on the first PolygonPoint or
		// the right Button was pressed the Polygon is finished
  	if ((currentArea->selectionPoints()->count()>2)
  		&& (currentArea->selectionPoints()->first()->contains(drawEnd)
  		 || (e->button()==Qt::RightButton)))
  	{
			currentArea->setFinished(true);
   		imageMapEditor->addArea(currentArea);
  		currentAction=None;
		} else
		{
   		currentArea->addCoord(drawEnd);
   		currentSelectionPoint=currentArea->selectionPoints()->last();
   	}

//			currentArea->addCoord(drawEnd);
//			currentSelectionPoint=currentArea->selectionPoints()->last();
	} else
	if (currentAction==MoveArea || currentAction==MoveSelectionPoint) {
  		imageMapEditor->slotAreaChanged(currentArea);
  		currentAction=None;
  }
  else {
  	currentAction=None;
  }
	imageMapEditor->slotChangeStatusCoords(drawEnd.x(),drawEnd.y());
	imageMapEditor->slotUpdateSelectionCoords();

	if (currentArea)
		repaintArea(*currentArea);
//	repaintContents(0,0,contentsWidth(),contentsHeight(),false);
}


void ImageMap::mouseMoveEvent(QMouseEvent *e) {
		drawCurrent=e->pos();

		// If outside the image
		// set it to the border
		if (!imageRect.contains(drawCurrent)) {
  		if (drawCurrent.x()>imageRect.right())
  			drawCurrent.setX(imageRect.right());
  		if (drawCurrent.x()<imageRect.left())
  			drawCurrent.setX(imageRect.left());
  		if (drawCurrent.y()>imageRect.bottom())
  			drawCurrent.setY(imageRect.bottom());
  		if (drawCurrent.y()<imageRect.top())
  			drawCurrent.setY(imageRect.top());
		}

		// Translate to image coordinates
		drawCurrent-=imageRect.topLeft();
		drawCurrent=translateFromZoom(drawCurrent);

		if (currentAction==DrawRectangle) {
			// To avoid flicker, only repaint the minimum rect
			QRect oldRect=translateToZoom(currentArea->rect());
			currentArea->setRect(QRect(drawStart,drawCurrent).normalize());
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords(currentArea->rect());
		} else
		if (currentAction==DrawCircle) {
			QRect oldRect=translateToZoom(currentArea->rect());
			currentArea->setRect(QRect(drawStart,drawCurrent).normalize());
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords(currentArea->rect());
		} else
		if ( currentAction==DrawPolygon ) {
			QRect oldRect=translateToZoom(currentArea->rect());
			currentArea->moveSelectionPoint(currentSelectionPoint,drawCurrent);
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords(currentArea->rect());
		} else
		if ( currentAction==MoveArea ) {
			QRect oldRect=translateToZoom(currentArea->selectionRect());
			currentArea->translate((drawCurrent-drawStart).x(),(drawCurrent-drawStart).y());
			QRect newRect=translateToZoom(currentArea->selectionRect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			drawStart=drawCurrent;
			imageMapEditor->slotUpdateSelectionCoords();
		} else
		if ( currentAction==MoveSelectionPoint ) {
			QRect oldRect=translateToZoom(currentArea->selectionRect());
			currentArea->moveSelectionPoint(currentSelectionPoint,drawCurrent);
			QRect newRect=translateToZoom(currentArea->selectionRect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords();
		}
		imageMapEditor->slotChangeStatusCoords(drawCurrent.x(),drawCurrent.y());
}

void ImageMap::resizeEvent(QResizeEvent* e) {
	QScrollArea::resizeEvent(e);
	int width=(int) (image.width()*_zoom);
	int height=(int) (image.height()*_zoom);
	if (visibleWidth()>width)
		width=visibleWidth();
	if (visibleHeight()>height)
		height=visibleHeight();

	resizeContents(width,height);

	imageRect.setLeft(0);
	imageRect.setTop(0);
	imageRect.setHeight(image.height()*_zoom);
	imageRect.setWidth(image.width()*_zoom);

}

void ImageMap::repaintArea(const Area & a) {
	repaintContents(translateToZoom(a.selectionRect()),false);
}

void ImageMap::drawContents(QPainter* p,int clipx,int clipy,int clipw,int cliph) {
//	qCDebug(KIMAGEMAPEDITOR_LOG) << "drawing\n";
//	p.scale(rect.width()*2,rect.height()*2);
//	if (e->rect()!=rect()) {
//		p.setClipping(true);
//		p.setClipRect(e->rect());
//	} else
/*	if (currentAction==DrawRectangle) {
		p->setClipping(true);
		QRect r(currentArea->rect());
		r.translate(imageRect.left()-5,imageRect.top()-5);
		r.setSize(r.size()+QSize(10,10));
		p->setClipRegion(r);
	}
*/

	QRect updateRect(clipx,clipy,clipw,cliph);
  QPixmap doubleBuffer(updateRect.size());        // Pixmap for double-buffering
  QPainter p2(&doubleBuffer);
	p2.drawPixmap(0,0,zoomedImage,clipx,clipy,clipw,cliph);
	p2.translate(-updateRect.x(), -updateRect.y());
	p2.scale(_zoom,_zoom);

	AreaList *list=imageMapEditor->areaList();
	for (Area* s=list->first();s != 0L; s=list->next())
		s->draw(p2);

	// Draw the current drawing Area
	if (currentAction != MoveArea &&
			currentAction != MoveSelectionPoint &&
			currentAction != None)
	{
		currentArea->draw(p2);
	}

  p2.end();

  // Copy the double buffer into the widget
  p->drawPixmap(clipx,clipy,doubleBuffer);
	// Erase background without flicker
	QRegion region(contentsX(),contentsY(),visibleWidth(),visibleHeight());
	region=region.subtract(QRegion(imageRect));
	for (int i=0;i<region.rects().count();i++) {
		p->eraseRect(region.rects()[i]);
	}


	// Draw our picture
//	p->drawPixmap(imageRect.left(),imageRect.top(),zoomedImage);
//
//
//	p->scale(_zoom,_zoom);
//	p->translate(imageRect.left(),imageRect.top());
//
//	AreaList *list=imageMapEditor->areaList();
//	for (Area* s=list->first();s != 0L; s=list->next())
//		s->draw(*p);
//
//	// Draw the current drawing Area
//	if (currentAction != MoveArea &&
//			currentAction != MoveSelectionPoint &&
//			currentAction != None)
//	{
//		currentArea->draw(*p);
//	}


}
