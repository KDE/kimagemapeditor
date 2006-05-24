/***************************************************************************
                          drawzone.cpp  -  description
                             -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan Sch�er
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

// QT
#include <qbitmap.h>
#include <qpainter.h>
#include <q3dragobject.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QDropEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

// KDE
#include <kdebug.h>
#include <k3urldrag.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmimetype.h>

// Local
#include "drawzone.h"
#include "kimagemapeditor.h"
#include "kimecommands.h"
#include "areacreator.h"

#include "kimecommon.h"

DrawZone::DrawZone(QWidget *parent,KImageMapEditor* _imageMapEditor)
	: Q3ScrollView(parent)
{
	imageMapEditor=_imageMapEditor;
//	setPicture(QImage());
	currentAction=None;
	currentArea=0L;
	oldArea=0L;
	_zoom=1;
  if (imageMapEditor->isReadWrite()) {
	    viewport()->setMouseTracking(true);
    	viewport()->setAcceptDrops(true);
      this->setAcceptDrops(true);
  }
  else
      viewport()->setMouseTracking(false);

	setDragAutoScroll(true);

	// The cross rectangle cursor
	QBitmap b(32,32,true);
	QBitmap b2(32,32,true);
	QPainter p(&b);
	// the cross
  p.drawLine(0,8,6,8);
  p.drawLine(10,8,16,8);
	p.drawLine(8,0,8,6);
	p.drawLine(8,10,8,16);
	// the rectangle
	p.drawRect(17,17,8,6);

	p.end();

	p.begin(&b2);
	// the cross black lines
  p.drawLine(0,8,6,8);
  p.drawLine(10,8,16,8);
	p.drawLine(8,0,8,6);
	p.drawLine(8,10,8,16);

	// the cross white lines
  p.drawLine(0,7,6,7);
  p.drawLine(10,7,16,7);
	p.drawLine(7,0,7,6);
	p.drawLine(7,10,7,16);

	// the cross white lines
  p.drawLine(0,9,6,9);
  p.drawLine(10,9,16,9);
	p.drawLine(9,0,9,6);
	p.drawLine(9,10,9,16);

	// the rectangles
	p.drawRect(17,17,8,6);	// black
	p.drawRect(18,18,6,4);  // white
	p.drawRect(16,16,10,8); // white

	p.end();

	RectangleCursor = QCursor(b,b2,8,8);


	// The cross circle cursor
	b = QBitmap(32,32,true);
	b2 = QBitmap(32,32,true);
	p.begin(&b);
	// the cross
  p.drawLine(0,8,6,8);
  p.drawLine(10,8,16,8);
	p.drawLine(8,0,8,6);
	p.drawLine(8,10,8,16);
	// the circle
	p.drawEllipse(17,17,8,8);

	p.end();

	p.begin(&b2);
	// the cross black lines
  p.drawLine(0,8,6,8);
  p.drawLine(10,8,16,8);
	p.drawLine(8,0,8,6);
	p.drawLine(8,10,8,16);

	// the cross white lines
  p.drawLine(0,7,6,7);
  p.drawLine(10,7,16,7);
	p.drawLine(7,0,7,6);
	p.drawLine(7,10,7,16);

	// the cross white lines
  p.drawLine(0,9,6,9);
  p.drawLine(10,9,16,9);
	p.drawLine(9,0,9,6);
	p.drawLine(9,10,9,16);

	// the circles
	p.drawEllipse(17,17,8,8);  // black
	p.drawEllipse(16,16,10,10);  // white
	p.drawEllipse(18,18,6,6);  // white

	p.end();

	CircleCursor = QCursor(b,b2,8,8);

	QString path = KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/polygoncursor.png" ) + "kimagemapeditor/polygoncursor.png";
	PolygonCursor = QCursor(QPixmap(path),8,8);

	path = KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/freehandcursor.png" ) + "kimagemapeditor/freehandcursor.png";
	FreehandCursor = QCursor(QPixmap(path),8,8);

	path = KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/addpointcursor.png" ) + "kimagemapeditor/addpointcursor.png";
	AddPointCursor = QCursor(QPixmap(path),8,8);

	path = KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/removepointcursor.png" ) + "kimagemapeditor/removepointcursor.png";
	RemovePointCursor = QCursor(QPixmap(path),8,8);
}

DrawZone::~DrawZone(){
}

void DrawZone::setPicture(const QImage &_image) {
	image=_image;
//-	zoomedImage.convertFromImage(image);
	setZoom(_zoom);
}

void DrawZone::setZoom(double z)
{
	_zoom=z;
	imageRect.setHeight(myround(image.height()*_zoom));
	imageRect.setWidth(myround(image.width()*_zoom));
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

QPoint DrawZone::translateFromZoom(const QPoint & p) const {
	return QPoint((int)(p.x()/_zoom),(int)(p.y()/_zoom));
}

QRect DrawZone::translateFromZoom(const QRect & p) const {
	return QRect((int)(p.x()/_zoom),(int) (p.y()/_zoom),
							 (int)(p.width()/_zoom),(int)(p.height()/_zoom));
}

QPoint DrawZone::translateToZoom(const QPoint & p) const {
	return QPoint(myround(p.x()*_zoom),myround(p.y()*_zoom));
}

QRect DrawZone::translateToZoom(const QRect & r) const {
//	return QRect(round(r.x()*_zoom),round(r.y()*_zoom),
//							 round(r.width()*_zoom),round(r.height()*_zoom));
	return QRect((int)(r.x()*_zoom),(int)(r.y()*_zoom),
							 (int)(r.width()*_zoom+2),(int)(r.height()*_zoom+2));
}

void DrawZone::contentsMouseDoubleClickEvent(QMouseEvent* e) {
  if ( ! imageMapEditor->isReadWrite())
     return;

	QPoint point=e->pos();
	point-=imageRect.topLeft();
	point=translateFromZoom(point);
	if ( currentAction==None &&
		(currentArea=imageMapEditor->onArea(point)))
	{
		imageMapEditor->deselectAll();
		imageMapEditor->select(currentArea);
		currentArea=imageMapEditor->selected();
		imageMapEditor->showTagEditor(imageMapEditor->selected());
	}

}

void DrawZone::contentsMousePressEvent(QMouseEvent* e)
{
  if ( ! imageMapEditor->isReadWrite())
     return;

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
  QPoint zoomedPoint = drawStart;
	drawStart=translateFromZoom(drawStart);
	delete oldArea;
	oldArea=0L;

	if (currentArea)
	{
		oldArea=currentArea->clone();
	}

	if ( currentAction==None ) {
		if (e->button()==Qt::RightButton)
		{
			if ( (currentArea=imageMapEditor->onArea(drawStart)) )
			{
  			if ( ! currentArea->isSelected())
  			{
  				imageMapEditor->deselectAll();
  				imageMapEditor->select(currentArea);
  			}
 				currentArea=imageMapEditor->selected();
  		}

  		imageMapEditor->slotShowMainPopupMenu(e->globalPos());

		}
		else
		if (e->button()==Qt::MidButton) {
			contentsMouseDoubleClickEvent(e);
		}
		else  // LeftClick on selectionpoint
		if ((currentArea=imageMapEditor->selected()) &&
			(currentSelectionPoint=currentArea->onSelectionPoint(zoomedPoint,_zoom)))
		{
			oldArea=currentArea->clone();

		  if ( (imageMapEditor->currentToolType() == KImageMapEditor::RemovePoint) &&
		       (imageMapEditor->selected()->selectionPoints()->count()>3) )
		  {
      	currentAction=RemovePoint;
		  }
		  else
		  {
  			currentAction=MoveSelectionPoint;
        currentArea->setMoving(true);
      }

		} else // leftclick not on selectionpoint but on area
		if ((currentArea=imageMapEditor->onArea(drawStart)))
		{
		  if ( imageMapEditor->currentToolType() == KImageMapEditor::AddPoint )
		  {
		    currentAction=AddPoint;
		    viewport()->setCursor(AddPointCursor);
   			oldArea=currentArea->clone();
		  }
		  else
		  {
  			currentAction=MoveArea;
  			viewport()->setCursor(Qt::SizeAllCursor);

  			if ( currentArea->isSelected() ) {
  				if ( (e->state() & Qt::ControlModifier) )
   					imageMapEditor->deselect(currentArea);
   			} else
  			{
  				if ( (e->state() & Qt::ControlModifier) )
  					imageMapEditor->select( currentArea );
  				else {
  					imageMapEditor->deselectAll();
  					imageMapEditor->select( currentArea );
  				}
  			}

  			currentArea = imageMapEditor->selected();
        currentArea->setMoving(true);

  			oldArea=currentArea->clone();
      }
  	}
		else  // leftclick on the background
		if ( (imageMapEditor->currentToolType()==KImageMapEditor::Rectangle) ||
		     (imageMapEditor->currentToolType()==KImageMapEditor::Circle) ||
		     (imageMapEditor->currentToolType()==KImageMapEditor::Polygon) ||
		     (imageMapEditor->currentToolType()==KImageMapEditor::Freehand))
		{
			currentArea=AreaCreator::create(imageMapEditor->currentToolType());

 		  currentArea->setRect(QRect(drawStart,drawStart));
 		  currentArea->setSelected(false);
 		  imageMapEditor->deselectAll();

			switch (imageMapEditor->currentToolType())	{
				case KImageMapEditor::Rectangle : currentAction=DrawRectangle; break;
				case KImageMapEditor::Circle : currentAction=DrawCircle; break;
				case KImageMapEditor::Polygon :
  					currentAction=DrawPolygon;
      			currentArea->addCoord(drawStart);
      			currentSelectionPoint=currentArea->selectionPoints()->last();
      			break;
        case KImageMapEditor::Freehand :
            currentAction=DrawFreehand;
      			//currentArea->addCoord(drawStart);
      			currentArea->setFinished(false);
 			  		break;
				default: break;
			}
		}
		else
  	// leftclicked with the arrow at an areafree position
		if (imageMapEditor->currentToolType()==KImageMapEditor::Selection)
  	{
  	  	currentArea=0L;
  	  	imageMapEditor->deselectAll();
  	  	// Start drawing a selection rectangle
  	  	currentAction=DoSelect;
  	  	oldSelectionRect = imageRect;
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

void DrawZone::contentsMouseReleaseEvent(QMouseEvent *e) {
  if ( ! imageMapEditor->isReadWrite())
     return;

  QPoint drawEnd=e->pos();

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
  QPoint zoomedPoint=drawEnd;

	drawEnd=translateFromZoom(drawEnd);

	if (currentAction==DrawCircle || currentAction==DrawRectangle) {
  		currentAction=None;
   		imageMapEditor->commandHistory()->addCommand(
   			new CreateCommand( imageMapEditor, currentArea ), true);
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
  		currentAction=None;
   		imageMapEditor->commandHistory()->addCommand(
   			new CreateCommand( imageMapEditor, currentArea ), true);
		} else
		{
   		currentArea->insertCoord(currentArea->countSelectionPoints()-1, drawEnd);
   		currentSelectionPoint=currentArea->selectionPoints()->last();
   	}
	} else
	if (currentAction==DrawFreehand)
	{
			currentArea->setFinished(true);
			currentArea->simplifyCoords();
  		currentAction=None;
   		imageMapEditor->commandHistory()->addCommand(
   			new CreateCommand( imageMapEditor, currentArea ), true);
	} else
	if (currentAction==MoveArea) {
	    QPoint p1 = oldArea->rect().topLeft();
	    QPoint p2 = imageMapEditor->selected()->rect().topLeft();

	    if (p1 != p2)
	    {
  			imageMapEditor->commandHistory()->addCommand(
	  			new MoveCommand( imageMapEditor, imageMapEditor->selected(), oldArea->rect().topLeft()),true);
  	  	imageMapEditor->slotAreaChanged(currentArea);
  	  } else
     	  imageMapEditor->updateSelection();

  		currentAction=None;
  } else
  if (currentAction==MoveSelectionPoint) {
			imageMapEditor->commandHistory()->addCommand(
				new ResizeCommand( imageMapEditor, imageMapEditor->selected(), oldArea),true);
  		imageMapEditor->slotAreaChanged(currentArea);
  		currentAction=None;
  } else
  if (currentAction==RemovePoint) {
     if (currentSelectionPoint==currentArea->onSelectionPoint(zoomedPoint,_zoom))
     {
       currentArea->removeSelectionPoint(currentSelectionPoint);

       imageMapEditor->commandHistory()->addCommand(
         new RemovePointCommand( imageMapEditor, imageMapEditor->selected(), oldArea),true);
    	 imageMapEditor->slotAreaChanged(currentArea);
     }
 		 currentAction=None;
  } else
  if (currentAction==AddPoint)
  {
      if (currentArea==imageMapEditor->onArea(drawEnd))
      {
        imageMapEditor->commandHistory()->addCommand(
          new AddPointCommand( imageMapEditor, imageMapEditor->selected(), drawEnd),true);
    		imageMapEditor->slotAreaChanged(currentArea);
  		}
   		currentAction=None;
  } else
  if (currentAction==DoSelect) {
  	currentAction=None;

   	QRect r(drawStart.x(),drawStart.y(),drawCurrent.x()-drawStart.x(),drawCurrent.y()-drawStart.y());
   	r = r.normalize();

    	AreaListIterator it=imageMapEditor->areaList();
  		for ( ; it.current() != 0L ; ++it )	{
    		if ( it.current()->rect().intersects(r) )
    		{
   				if (!it.current()->isSelected() )
    				imageMapEditor->selectWithoutUpdate( it.current() );
    		}
    		else
     		  if (it.current()->isSelected())
      			imageMapEditor->deselectWithoutUpdate( it.current() );
			}

    imageMapEditor->updateActionAccess();
  	imageMapEditor->updateSelection();
    repaintContents(imageRect,false);
  } else {
  	currentAction=None;
  }
	imageMapEditor->slotChangeStatusCoords(drawEnd.x(),drawEnd.y());
	if (currentArea)
	{
	  currentArea->setMoving(false);
		repaintArea(*currentArea);
	}
	delete oldArea;
	oldArea=0L;
//	repaintContents(0,0,contentsWidth(),contentsHeight(),false);
  imageMapEditor->slotUpdateSelectionCoords();
}


void DrawZone::contentsMouseMoveEvent(QMouseEvent *e)
{
  if ( ! imageMapEditor->isReadWrite())
     return;


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
    QPoint zoomedPoint=drawCurrent;
		drawCurrent=translateFromZoom(drawCurrent);

		if (currentAction==DrawRectangle) {
			// To avoid flicker, only repaint the minimum rect
			QRect oldRect=translateToZoom(currentArea->rect());
			currentArea->setRect(QRect(drawStart,drawCurrent).normalize());
			QRect newRect=translateToZoom(currentArea->selectionRect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords( currentArea->rect() );
		} else
		if (currentAction==DrawCircle) {
			QRect oldRect=translateToZoom(currentArea->rect());

			// We don't want ellipses
			int maxDistance=myabs(drawStart.x()-drawCurrent.x()) >
											myabs(drawStart.y()-drawCurrent.y()) ?
											myabs(drawStart.x()-drawCurrent.x()) :
											myabs(drawStart.y()-drawCurrent.y()) ;

			int xDiff=maxDistance;
			int yDiff=maxDistance;

			if ( drawStart.x()-drawCurrent.x() > 0)
				xDiff=-xDiff;

			if ( drawStart.y()-drawCurrent.y() > 0)
				yDiff=-yDiff;

			QPoint endPoint( drawStart.x()+xDiff, drawStart.y()+yDiff);

			currentArea->setRect(QRect(drawStart,endPoint).normalize());
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
			imageMapEditor->slotUpdateSelectionCoords( currentArea->rect() );
		} else
		if ( currentAction==DrawPolygon ) {
			QRect oldRect=translateToZoom(currentArea->rect());
			currentArea->moveSelectionPoint(currentSelectionPoint,drawCurrent);
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
		} else
		if ( currentAction==DrawFreehand) {
			QRect oldRect=translateToZoom(currentArea->rect());
		  currentArea->insertCoord(currentArea->countSelectionPoints(), drawCurrent);
			QRect newRect=translateToZoom(currentArea->rect());
			QRect r=oldRect | newRect;
			repaintContents(r,false);
		} else
		if ( currentAction==MoveArea ) {
			QRect oldRect=translateToZoom(currentArea->selectionRect());
			currentArea->moveBy((drawCurrent-drawStart).x(),(drawCurrent-drawStart).y());
			QRect newRect=translateToZoom(currentArea->selectionRect());
			QRect r=oldRect | newRect;
			currentArea->setMoving(true);
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
		} else
	  if (currentAction==DoSelect) {

    	QRect r(drawStart.x(),drawStart.y(),drawCurrent.x()-drawStart.x(),drawCurrent.y()-drawStart.y());
    	r = r.normalize();
//    	r = translateFromZoom(r);
/*
    	AreaListIterator it=imageMapEditor->areaList();
  		for ( ; it.current() != 0L ; ++it )	{
    		if ( it.current()->rect().intersects(r) )
    		{
   				if (!it.current()->isSelected() )
    				imageMapEditor->selectWithoutUpdate( it.current() );
    		}
    		else
     		  if (it.current()->isSelected())
      			imageMapEditor->deselectWithoutUpdate( it.current() );
			}
*/
      // We don't have to repaint the hole selection rectangle
      // only the borders have to be repainted.
      // So we have to create 4 rectangles for every rectangle
      // which represent the borders and then repaint them.

      QRect lb,rb,tb,bb;
      createBorderRectangles(translateToZoom(r),lb,rb,tb,bb);
      repaintContents(lb,false);
      repaintContents(rb,false);
      repaintContents(tb,false);
      repaintContents(bb,false);

      createBorderRectangles(translateToZoom(oldSelectionRect),lb,rb,tb,bb);
      repaintContents(lb,false);
      repaintContents(rb,false);
      repaintContents(tb,false);
      repaintContents(bb,false);

//    	repaintContents(oldSelectionRect | r,false);
    	oldSelectionRect = r;
//    	repaintContents(translateToZoom(r),false);
//+			imageMapEditor->updateSelection();


//			QRect r(drawStart.x(),drawStart.y(),drawCurrent.x()-drawStart.x(),drawCurrent.y()-drawStart.y());
//			r = r.normalize();
//			QRect r2(drawStart.x(),drawStart.y(),drawOld.x()-drawStart.x(),drawOld.y()-drawStart.y());
//			r2 = r2.normalize();
//			r = translateToZoom(r | r2);
//			repaintContents(r,false);
  	} else
		if ( currentAction==None )
		{
			if ( imageMapEditor->selected() &&
					 imageMapEditor->selected()->onSelectionPoint(zoomedPoint,_zoom ))
			{
 				if (imageMapEditor->selected()->type()==Area::Polygon)
 				{
			    if ((imageMapEditor->currentToolType()==KImageMapEditor::RemovePoint) &&
			        (imageMapEditor->selected()->selectionPoints()->count()>3) )
			    {
		        viewport()->setCursor(RemovePointCursor);
			    }
			    else
			    {
 						viewport()->setCursor(Qt::PointingHandCursor);
 				  }
 				}
 				else
 				{
   				QPoint center=imageMapEditor->selected()->rect().center();
   				if (drawCurrent.x() < center.x()) {
   					if (drawCurrent.y() < center.y())
   						viewport()->setCursor(Qt::SizeFDiagCursor);
   					else
   						viewport()->setCursor(Qt::SizeBDiagCursor);
   				}
   				else {
   					if (drawCurrent.y() < center.y())
   						viewport()->setCursor(Qt::SizeBDiagCursor);
   					else
   						viewport()->setCursor(Qt::SizeFDiagCursor);
 					}
 				}
			} else
			if ( imageMapEditor->onArea(drawCurrent) )
			{
  			if (imageMapEditor->currentToolType()==KImageMapEditor::AddPoint)
  			{
			    viewport()->setCursor(AddPointCursor);
			  }
			  else
        {
				  viewport()->setCursor(Qt::SizeAllCursor);
        }
			}
			else
			if (imageMapEditor->currentToolType()==KImageMapEditor::Rectangle) {
			    viewport()->setCursor(RectangleCursor);
//          kDebug() << "KImageMapEditor::DrawZone: viewport()->setCursor to Rectangle" << endl;
      }
			else
			if (imageMapEditor->currentToolType()==KImageMapEditor::Circle)
			    viewport()->setCursor(CircleCursor);
			else
			if (imageMapEditor->currentToolType()==KImageMapEditor::Polygon)
			    viewport()->setCursor(PolygonCursor);
			else
			if (imageMapEditor->currentToolType()==KImageMapEditor::Freehand)
			    viewport()->setCursor(FreehandCursor);
			else
		 		viewport()->setCursor(Qt::ArrowCursor);

		}
		imageMapEditor->slotChangeStatusCoords(drawCurrent.x(),drawCurrent.y());
}

void DrawZone::createBorderRectangles(const QRect & r,QRect & rb,QRect & lb,QRect & tb,QRect & bb)
{
  int bw;
  bw = (int) (2+2*_zoom); // Border width

  rb.setX(r.x()+r.width()-bw);
  rb.setY(r.y());
  rb.setWidth(bw+1);
  rb.setHeight(r.height());

  lb.setX(r.x());
  lb.setY(r.y());
  lb.setWidth(bw);
  lb.setHeight(r.height());

  tb.setX(r.x());
  tb.setY(r.y());
  tb.setWidth(r.width());
  tb.setHeight(bw);

  bb.setX(r.x());
  bb.setY(r.y()+r.height()-bw);
  bb.setWidth(r.width());
  bb.setHeight(bw+1);
}


void DrawZone::resizeEvent(QResizeEvent* e) {
	Q3ScrollView::resizeEvent(e);
	int width=(int) (image.width()*_zoom);
	int height=(int) (image.height()*_zoom);
	if (visibleWidth()>width)
		width=visibleWidth();
	if (visibleHeight()>height)
		height=visibleHeight();

	resizeContents(width,height);

	imageRect.setLeft(0);
	imageRect.setTop(0);
	imageRect.setHeight((int)(image.height()*_zoom));
	imageRect.setWidth((int)(image.width()*_zoom));

}

void DrawZone::cancelDrawing()
{
  if (   (currentAction == DrawPolygon )
      || (currentAction == DrawRectangle )
      || (currentAction == DrawCircle )
     )
  {
    currentAction = None;
    QRect r = translateToZoom(currentArea->selectionRect());
    delete currentArea;
    currentArea = 0L;
    repaintContents(r,false);
    imageMapEditor->slotUpdateSelectionCoords();
  }
}

void DrawZone::repaintArea(const Area & a) {
	repaintContents(translateToZoom(a.selectionRect()),false);
}

void DrawZone::repaintRect(const QRect & r) {
	repaintContents(translateToZoom(r),false);
}

void DrawZone::drawContents(QPainter* p,int clipx,int clipy,int clipw,int cliph)
{

// Erase background without flicker
	QRect updateRect(clipx,clipy,clipw,cliph);

	// Pixmap for double-buffering
  QPixmap doubleBuffer(updateRect.size());
  if (doubleBuffer.isNull())
  	return;

  QPainter p2(&doubleBuffer);
	p2.drawPixmap(0,0,zoomedImage,clipx,clipy,clipw,cliph);
	p2.setBackground(p->background());

	if (zoomedImage.width() < (clipw+clipx) ) {
		int eraseWidth = clipw+clipx - zoomedImage.width();
		p2.eraseRect( QRect(clipw-eraseWidth,0,eraseWidth,cliph) );
	}

	if (zoomedImage.height() < (cliph+clipy) ) {
		int eraseHeight = cliph+clipy - zoomedImage.height();
		p2.eraseRect( QRect(0,cliph-eraseHeight,clipw,eraseHeight) );
	}

	p2.translate(-clipx, -clipy);
	p2.scale(_zoom,_zoom);

	QRect areaUpdateRect;
	areaUpdateRect.setX(myround(clipx/_zoom)-1);
	areaUpdateRect.setY(myround(clipy/_zoom)-1);
	areaUpdateRect.setWidth(myround(clipw/_zoom)+2);
	areaUpdateRect.setHeight(myround(cliph/_zoom)+2);

	AreaListIterator it=imageMapEditor->areaList();
	for ( it.toLast();it.current() != 0L; --it)
	{
     if (it.current()->rect().intersects(areaUpdateRect))
       it.current()->draw(p2);
	}

	// Draw the current drawing Area
	if (currentAction != MoveArea &&
			currentAction != MoveSelectionPoint &&
			currentAction != None &&
			currentAction != DoSelect)
	{
		currentArea->draw(p2);
	}

	if (currentAction == DoSelect )
  {
		QPen pen = QPen(QColor("white"),1);
		p2.setRasterOp(Qt::XorROP);
		pen.setStyle(Qt::DotLine);
		p2.setPen(pen);

		QRect r( drawStart.x(),drawStart.y(),drawCurrent.x()-drawStart.x(),drawCurrent.y()-drawStart.y());
		r = r.normalize();
		p2.drawRect(r);
	}



  p2.end();

  // Copy the double buffer into the widget
  p->drawPixmap(clipx,clipy,doubleBuffer);


}

void DrawZone::contentsDragEnterEvent(QDragEnterEvent*e) {
  KUrl::List uris = KUrl::List::fromMimeData( e->mimeData() );

  if ( uris.isEmpty() )
    return;

  KMimeType::Ptr ptr = KMimeType::findByURL(uris.first());
//  kDebug() << "***** " << ptr.data()->name() << endl;
  if ((ptr.data()->name() == "text/html")
      || (ptr.data()->name().left(6) == "image/"))
    e->accept();
}

void DrawZone::contentsDropEvent( QDropEvent* e) {
  viewportDropEvent(e);
}



void DrawZone::viewportDropEvent( QDropEvent* e) {
  KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );
  // A file from konqueror was dropped
	if (!urlList.isEmpty()) {
		imageMapEditor->openFile(urlList.first());
	}
}
