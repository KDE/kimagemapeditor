/***************************************************************************
                          drawzone.cpp  -  description
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

#include "drawzone.h"

// Qt
#include <QBitmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMimeData>
#include <QResizeEvent>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QStandardPaths>

// KDE Frameworks
#include "kimagemapeditor_debug.h"

// Local
#include "kimagemapeditor.h"
#include "kimecommands.h"
#include "areacreator.h"
#include "kimearea.h"

#include "kimecommon.h"

static QCursor createCircleCursor();
static QCursor createRectangleCursor();

DrawZone::DrawZone(QWidget *parent,KImageMapEditor* _imageMapEditor)
	: QWidget(parent)
{
  imageMapEditor=_imageMapEditor;
//	setPicture(QImage());
  currentAction=None;
  currentArea = nullptr;
  oldArea = nullptr;
  _zoom=1;
  //  QWidget* w = new QWidget(this);
  //  setWidget(w);

  if (imageMapEditor->isReadWrite()) {
    setMouseTracking(true);
    setAcceptDrops(true);
  } else {
    setMouseTracking(false);
  }

  rectangleCursor = createRectangleCursor();
  circleCursor = createCircleCursor();

  QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kimagemapeditor/polygoncursor.png" );
  polygonCursor = QCursor(QPixmap(path),8,8);

  path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kimagemapeditor/freehandcursor.png" );
  freehandCursor = QCursor(QPixmap(path),8,8);

  path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kimagemapeditor/addpointcursor.png" );
  addPointCursor = QCursor(QPixmap(path),8,8);

  path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kimagemapeditor/removepointcursor.png" );
  removePointCursor = QCursor(QPixmap(path),8,8);
}

DrawZone::~DrawZone(){
}

static QCursor createCircleCursor() {
  

  // The cross circle cursor
  QBitmap b(32,32);
  QBitmap b2(32,32);
  b2.clear();
  b.clear();

  QPainter p;

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

  return QCursor(b,b2,8,8);
}

static QCursor createRectangleCursor() {
  // The cross rectangle cursor
  QBitmap b(32,32);
  QBitmap b2(32,32);
  b.clear();
  b2.clear();

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

  return QCursor(b,b2,8,8);
}

void DrawZone::setPicture(const QImage &_image) {
	image=_image;
//-	zoomedImage.convertFromImage(image);
	setZoom(_zoom);
}
 
void DrawZone::setZoom(double z)
{
  _zoom=z;
  zoomedImage = zoomedImage.fromImage(image);
  imageRect.setHeight(myround(image.height()*_zoom));
  imageRect.setWidth(myround(image.width()*_zoom));
  zoomedImage = zoomedImage.scaled(imageRect.size());
  /*
  zoomedImage = QPixmap(imageRect.width(),imageRect.height());
  QPainter p(&zoomedImage);
  p.scale(z,z);
  QPixmap pix;
  pix.fromImage(image);
  // if the picture has transparent areas,
  // fill them with Gimp like background
  if (!pix.mask().isNull()) {
    QPixmap backPix(32,32);
    QPainter p2(&backPix);
    p2.fillRect(0,0,32,32,QColor(156,149,156));
    p2.fillRect(0,16,16,16,QColor(98,105,98));
    p2.fillRect(16,0,16,16,QColor(98,105,98));
    p.setPen(QPen());
    p.fillRect(imageRect.left(),imageRect.top(),imageRect.width(),imageRect.height(),QBrush(QColor("black"),backPix));
  }
  p.drawPixmap(imageRect.left(),imageRect.top(),pix);
  */
  /*
    resizeContents(visibleWidth()>imageRect.width() ? visibleWidth() : imageRect.width(),
    visibleHeight()>imageRect.height() ? visibleHeight() : imageRect.height());
  */

  
  resize(zoomedImage.size());
  repaint();//0,0,width(),height());
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

void DrawZone::mouseDoubleClickEvent(QMouseEvent* e) {
  if ( ! imageMapEditor->isReadWrite())
     return;

  QPoint point=e->pos();
  point-=imageRect.topLeft();
  point=translateFromZoom(point);
  Area* a;
  if ( currentAction==None &&
       (a=imageMapEditor->onArea(point)))
  {
    imageMapEditor->deselectAll();
    imageMapEditor->select(currentArea);
    currentArea=imageMapEditor->selected();
    imageMapEditor->showTagEditor(imageMapEditor->selected());
  }

}

QPoint DrawZone::moveIntoImage(QPoint p) {
  // Check if it's on picture if not
  // move it to the picture's border
  if (!imageRect.contains(p)) {
    if (p.x()>imageRect.right())
      p.setX(imageRect.right());
    if (p.x()<imageRect.left())
      p.setX(imageRect.left());
    if (p.y()>imageRect.bottom())
      p.setY(imageRect.bottom());
    if (p.y()<imageRect.top())
      p.setY(imageRect.top());
  }
  return p;
}


void DrawZone::mousePressRightNone(QMouseEvent* e, QPoint drawStart) {
  if ( (currentArea=imageMapEditor->onArea(drawStart)) ) {
    if ( ! currentArea->isSelected()) {
      imageMapEditor->deselectAll();
      imageMapEditor->select(currentArea);
    }
    currentArea=imageMapEditor->selected();
  }
  imageMapEditor->slotShowMainPopupMenu(e->globalPos());
}

void DrawZone::mousePressLeftNoneOnArea(QMouseEvent* e, Area* area) {

  if ( imageMapEditor->currentToolType() == KImageMapEditor::AddPoint )
  {
    oldArea=area->clone();
    currentAction=AddPoint;
    setCursor(addPointCursor);
  } else {
    currentAction=MoveArea;
    setCursor(Qt::SizeAllCursor);
	
    if ( area->isSelected() ) {
      if ( (e->modifiers() & Qt::ControlModifier) )
	imageMapEditor->deselect(area);
    } else {
      if ( (e->modifiers() & Qt::ControlModifier) )
	imageMapEditor->select( area );
      else {
	imageMapEditor->deselectAll();
	imageMapEditor->select( area );
      }
    }
	
    currentArea = imageMapEditor->selected();
    currentArea->setMoving(true);

    oldArea=currentArea->clone();
  }
}


void DrawZone::mousePressLeftNoneOnBackground(QMouseEvent*, QPoint drawStart) {   
  KImageMapEditor::ToolType toolType = imageMapEditor->currentToolType();
  
  if ( (toolType==KImageMapEditor::Rectangle) ||
       (toolType==KImageMapEditor::Circle) ||
       (toolType==KImageMapEditor::Polygon) ||
       (toolType==KImageMapEditor::Freehand))
  {
    currentArea = AreaCreator::create(toolType);

    currentArea->setRect(QRect(drawStart,drawStart));
    currentArea->setSelected(false);
    imageMapEditor->deselectAll();
    
    switch (toolType)	{
    case KImageMapEditor::Rectangle : 
      currentAction = DrawRectangle; 
      break;
    case KImageMapEditor::Circle : 
      currentAction = DrawCircle; 
      break;
    case KImageMapEditor::Polygon :
      currentAction = DrawPolygon;
      currentArea->addCoord(drawStart);
      currentSelectionPoint = currentArea->selectionPoints().last();
      break;
    case KImageMapEditor::Freehand :
      currentAction = DrawFreehand;
      //currentArea->addCoord(drawStart);
      currentArea->setFinished(false);
      break;
    default: 
      break;
    }
  } else {
    // leftclicked with the arrow at an areafree position
    if (toolType==KImageMapEditor::Selection)
    {
      currentArea = nullptr;
      imageMapEditor->deselectAll();
      // Start drawing a selection rectangle
      currentAction=DoSelect;
      oldSelectionRect = imageRect;
    }
  }
}


void DrawZone::mousePressLeftNone(QMouseEvent* e, QPoint drawStart, QPoint zoomedPoint) {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "mousePressLeftNone";
  Area* a;
  if ((a = imageMapEditor->selected()) &&
      (currentSelectionPoint=a->onSelectionPoint(zoomedPoint,_zoom)))
  {
    currentArea = a;
    if ( (imageMapEditor->currentToolType() == KImageMapEditor::RemovePoint) &&
	 (imageMapEditor->selected()->selectionPoints().count()>3) )
    {
      currentAction=RemovePoint;
    } else {
      currentAction=MoveSelectionPoint;
      currentArea->setMoving(true);
    }
  } else { // leftclick not on selectionpoint but on area
    if ((a = imageMapEditor->onArea(drawStart))) {
      currentArea = a;
      mousePressLeftNoneOnArea(e,currentArea);
    } else { 
      mousePressLeftNoneOnBackground(e, drawStart);
    }
  }
}


void DrawZone::mousePressNone(QMouseEvent* e, QPoint drawStart, QPoint zoomedPoint) {
  if (e->button()==Qt::RightButton) {
    mousePressRightNone(e,drawStart);
  } else {
    if (e->button()==Qt::MidButton) {
      mouseDoubleClickEvent(e);
    } else {
      mousePressLeftNone(e,drawStart,zoomedPoint);
    }
      
  } 

}

void DrawZone::mousePressEvent(QMouseEvent* e)
{
  if ( ! imageMapEditor->isReadWrite())
    return;

  drawStart = moveIntoImage(e->pos());
  drawLast = drawStart;
 
  // Translate it to picture coordinates
  //  drawStart-=imageRect.topLeft();
  QPoint zoomedPoint = drawStart;
  drawStart=translateFromZoom(drawStart);

  delete oldArea;
  oldArea = nullptr;

  if (currentArea) {
    oldArea = currentArea->clone();
  } 

  if (currentAction == None) {
    mousePressNone(e,drawStart,zoomedPoint);
  }

  QRect r;
  if (oldArea)
    r = oldArea->selectionRect();

  if (currentArea) {
    r = r | currentArea->selectionRect();
    repaint(translateToZoom(r));
  }


}

void DrawZone::mouseReleaseEvent(QMouseEvent *e) {
  if ( ! imageMapEditor->isReadWrite())
     return;

  QPoint drawEnd= moveIntoImage(e->pos());

  // Translate it to picture coordinates
  //  drawEnd-=imageRect.topLeft();
  QPoint zoomedPoint=drawEnd;
  drawEnd=translateFromZoom(drawEnd);

  switch (currentAction) {
  case DrawCircle:
  case DrawRectangle:
    currentAction = None;
    imageMapEditor->commandHistory()->push(
	  new CreateCommand( imageMapEditor, 
			     currentArea ));
    break;
  case DrawPolygon:
    // If the number of Polygonpoints is more than 2
    // and clicked on the first PolygonPoint or
    // the right Button was pressed the Polygon is finished
    if ((currentArea->selectionPoints().count()>2)
	&& (currentArea->selectionPoints().first()->getRect().contains(drawEnd)
	    || (e->button()==Qt::RightButton)))
    {
      currentArea->setFinished(true);
      currentAction=None;
      imageMapEditor->commandHistory()->push(
	  new CreateCommand( imageMapEditor, currentArea ));
    } else {
      currentArea->insertCoord(currentArea->countSelectionPoints()-1, drawEnd);
      currentSelectionPoint=currentArea->selectionPoints().last();
    }
    break;
  case DrawFreehand:
    currentArea->setFinished(true,false);
    currentArea->simplifyCoords();
    currentAction=None;
    imageMapEditor->commandHistory()->push(
	new CreateCommand( imageMapEditor, currentArea ));
    break;
  case MoveArea: {
    QPoint p1 = oldArea->rect().topLeft();
    QPoint p2 = imageMapEditor->selected()->rect().topLeft();

    if (p1 != p2) {
      imageMapEditor->commandHistory()->push(
	new MoveCommand( imageMapEditor, 
			 imageMapEditor->selected(), 
			 oldArea->rect().topLeft()));
      imageMapEditor->slotAreaChanged(currentArea);
    } else {
      imageMapEditor->updateSelection();
    }
    
    currentAction=None;
    break;
  }
  case MoveSelectionPoint:
    imageMapEditor->commandHistory()->push(
	new ResizeCommand( imageMapEditor, 
			   imageMapEditor->selected(), 
			   oldArea));
    imageMapEditor->slotAreaChanged(currentArea);
    currentAction=None;
    break;
  case RemovePoint:
    if (currentSelectionPoint ==
	currentArea->onSelectionPoint(zoomedPoint,_zoom)) 
    {
      currentArea->removeSelectionPoint(currentSelectionPoint);
      imageMapEditor->commandHistory()->push(
	new RemovePointCommand( imageMapEditor, 
				imageMapEditor->selected(), 
				oldArea));
      imageMapEditor->slotAreaChanged(currentArea);
    }
    currentAction=None;
    break;
  case AddPoint:
    if (currentArea == imageMapEditor->onArea(drawEnd)) {
      imageMapEditor->commandHistory()->push(
         new AddPointCommand( imageMapEditor, 
			      imageMapEditor->selected(), 
			      drawEnd));
      imageMapEditor->slotAreaChanged(currentArea);
    }
    currentAction=None;
    break;
  case DoSelect: {
    currentAction=None;
    QRect r(drawStart.x(),
	    drawStart.y(),
	    drawCurrent.x()-drawStart.x(),
	    drawCurrent.y()-drawStart.y());
    r = r.normalized();

    AreaListIterator it = imageMapEditor->areaList();
    while (it.hasNext()) {
      Area* a = it.next();
      if ( a->rect().intersects(r) )	{
	    if (!a->isSelected() ) {
	       imageMapEditor->selectWithoutUpdate( a );
	    }
      } else {
	    if (a->isSelected()) {
	       imageMapEditor->deselectWithoutUpdate( a );
	    }
      }
    }

    imageMapEditor->updateActionAccess();
    imageMapEditor->updateSelection();
    repaint(imageRect);
    break;
  }
  default:  
    currentAction=None;
  }

  imageMapEditor->slotChangeStatusCoords(drawEnd.x(),drawEnd.y());
  if (currentArea) {
    currentArea->setMoving(false);
    repaintArea(*currentArea);
  }
  
  delete oldArea;
  oldArea = nullptr;
  imageMapEditor->slotUpdateSelectionCoords();
}

QCursor DrawZone::getCursorOfToolType(KImageMapEditor::ToolType toolType) {
  switch(toolType) {
  case KImageMapEditor::Rectangle:
    return rectangleCursor;
  case KImageMapEditor::Circle:
    return circleCursor;
  case KImageMapEditor::Polygon:
    return polygonCursor;
  case KImageMapEditor::Freehand:
    return freehandCursor;
  default:
    return Qt::ArrowCursor;
  }
  return Qt::ArrowCursor;
}


void DrawZone::updateCursor(QPoint zoomedPoint) {
  AreaSelection* selected = imageMapEditor->selected();
  KImageMapEditor::ToolType toolType = imageMapEditor->currentToolType();
  SelectionPoint* selectionPoint;


  if ( imageMapEditor->onArea(drawCurrent) ) {
    if (toolType==KImageMapEditor::AddPoint) {
	setCursor(addPointCursor);
    } else {
      setCursor(Qt::SizeAllCursor);
    }
  } else {
    setCursor(getCursorOfToolType(toolType));
  }


  if ( selected )
  {
    selected->resetSelectionPointState();
    selectionPoint = selected->onSelectionPoint(zoomedPoint,_zoom );
    if (selectionPoint) {
      selectionPoint->setState(SelectionPoint::HighLighted);
      setCursor(selectionPoint->cursor());
      if (selected->type()==Area::Polygon) {
	if ((toolType==KImageMapEditor::RemovePoint) &&
	    (selected->selectionPoints().count()>3) )
	{
	  setCursor(removePointCursor);
	  selectionPoint->setState(SelectionPoint::AboutToRemove);
	}
      }
    } 
  } 
}

void DrawZone::mouseMoveSelection(QPoint drawCurrent) {
    
  QRect r(drawStart.x(),
	  drawStart.y(),
	  drawCurrent.x()-drawStart.x(),
	  drawCurrent.y()-drawStart.y());
  r = r.normalized();

  /*
  QRect r1,r2,r3,r4;
  createBorderRectangles(translateToZoom(oldSelectionRect),r1,r2,r3,r4);
  repaint(r1);
  repaint(r2);
  repaint(r3);
  repaint(r4);

  createBorderRectangles(translateToZoom(r),r1,r2,r3,r4);
  repaint(r1);
  repaint(r2);
  repaint(r3);
  repaint(r4);
  */

  QRect r2 = r.adjusted(-2,-2,2,2);
  oldSelectionRect.adjust(-2,-2,2,2);

  repaint(translateToZoom(r2) | translateToZoom(oldSelectionRect));

  oldSelectionRect = r;
}

void DrawZone::mouseMoveDrawCircle(QPoint drawCurrent) {
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

    currentArea->setRect(QRect(drawStart,endPoint).normalized());
}

void DrawZone::mouseMoveEvent(QMouseEvent *e)
{
  if ( ! imageMapEditor->isReadWrite())
    return;

  drawLast = drawCurrent;
  drawCurrent=moveIntoImage(e->pos());

  // Translate to image coordinates
  //  drawCurrent-=imageRect.topLeft();
  QPoint zoomedPoint=drawCurrent;
  drawCurrent=translateFromZoom(drawCurrent);

  QRect oldRect;
  if (currentArea)
    oldRect=currentArea->rect();

  switch(currentAction) {
  case None:
    updateCursor(zoomedPoint);
    break;
  case DoSelect:
    mouseMoveSelection(drawCurrent);
    break;
  case DrawRectangle:
    currentArea->setRect(QRect(drawStart,drawCurrent).normalized());
    break;
  case DrawCircle:
    mouseMoveDrawCircle(drawCurrent);
    break;
  case DrawPolygon:
    currentArea->moveSelectionPoint(currentSelectionPoint,drawCurrent);
    break;
  case DrawFreehand:
    currentArea->insertCoord(currentArea->countSelectionPoints(), drawCurrent);
    break;
  case MoveArea: {
    QPoint d = drawCurrent - drawLast;
    currentArea->moveBy(d.x(),d.y());
    currentArea->setMoving(true);
    break;
  }
  case MoveSelectionPoint:
    currentArea->moveSelectionPoint(currentSelectionPoint,drawCurrent);
    break;
  case RemovePoint:
  case AddPoint:
    break;
  }
  
  if (currentArea && (currentAction != DoSelect)) {
    QRect newRect = currentArea->selectionRect();
    newRect.adjust(-SELSIZE, -SELSIZE, SELSIZE, SELSIZE);
    QRect r = oldRect;
    r.adjust(-SELSIZE, -SELSIZE, SELSIZE, SELSIZE);
    repaint(translateToZoom(r) | translateToZoom(newRect));
    imageMapEditor->slotUpdateSelectionCoords( currentArea->rect() );
  }

  imageMapEditor->slotChangeStatusCoords(drawCurrent.x(),drawCurrent.y());
  //repaint();
}

void DrawZone::createBorderRectangles(
  const QRect & r,
  QRect & rb,
  QRect & lb,
  QRect & tb,
  QRect & bb)
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
    currentArea = nullptr;
    repaint(r);
    imageMapEditor->slotUpdateSelectionCoords();
  }
}

void DrawZone::repaintArea(const Area & a) {
	repaint(translateToZoom(a.selectionRect()));
}

void DrawZone::repaintRect(const QRect & r) {
	repaint(translateToZoom(r));
}

QSize DrawZone::sizeHint () const {
  return zoomedImage.size();
}

QSize DrawZone::minimumSize() const {
  return zoomedImage.size();
}



void DrawZone::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.drawPixmap(0,0,zoomedImage);//,clipx,clipy,clipw,cliph);
  p.setRenderHint(QPainter::Antialiasing);
  p.scale(_zoom,_zoom);

  AreaListIterator it=imageMapEditor->areaList();
  while (it.hasNext()) {
	it.next()->draw(&p);
  }

  // Draw the current drawing Area
  if (currentAction != MoveArea &&
      currentAction != MoveSelectionPoint &&
      currentAction != None &&
      currentAction != DoSelect)
    {
      currentArea->draw(&p);
    }

  if (currentAction == DoSelect ) {
    QColor front = Qt::white;
    front.setAlpha(200);
    QPen pen = QPen(front,1);
    //    pen.setStyle(Qt::DotLine);
    p.setPen(pen);
    p.setBrush(QBrush(Qt::NoBrush));

    QRect r( drawStart.x(),
	     drawStart.y(),
	     drawCurrent.x()-drawStart.x(),
	     drawCurrent.y()-drawStart.y());
    r = r.normalized();
    p.drawRect(r);
  }


  p.end();

  //  p->end();

  // Copy the double buffer into the widget
  //  p->drawPixmap(clipx,clipy,doubleBuffer);


}

void DrawZone::dragEnterEvent(QDragEnterEvent*e) {
  QList<QUrl> uris = e->mimeData()->urls();

  if ( uris.isEmpty() )
    return;

  QMimeDatabase db;
  QMimeType draggedMIME = db.mimeTypeForUrl(uris.first());
  // qCDebug(KIMAGEMAPEDITOR_LOG) << "***** " << draggedMIME.name();
  if ((draggedMIME.name() == "text/html")
      || (draggedMIME.name().left(6) == "image/"))
    e->accept();
}

void DrawZone::dropEvent( QDropEvent* e) {
  QList<QUrl> urlList = e->mimeData()->urls();
  // A file from konqueror was dropped
	if (!urlList.isEmpty()) {
		imageMapEditor->openFile(urlList.first());
	}
}
