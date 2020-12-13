/***************************************************************************
                          kimearea.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 2001
    copyright            : (C) 2001 by Jan Schaefer
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

#include "kimearea.h"

#include <QBitmap>
#include <QBrush>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QPolygon>

#include "kimagemapeditor_debug.h"

#include "kimecommon.h"


// The size of Selection Points

SelectionPoint::SelectionPoint(QPoint p, QCursor c)
{
  point = p;
  state = Normal;
  _cursor = c;
}

SelectionPoint::~SelectionPoint() {
}

void SelectionPoint::setState(SelectionPoint::State s) {
  state = s;
}

SelectionPoint::State SelectionPoint::getState() const {
  return state;
}

void SelectionPoint::setPoint(QPoint p) {
  point = p;
}

void SelectionPoint::translate(int dx, int dy) {
  point += QPoint(dx,dy);
}


QPoint SelectionPoint::getPoint() const {
  return point;
}

QRect SelectionPoint::getRect() const {
  QRect r(0,0,SELSIZE,SELSIZE);
  r.moveCenter(point);
  return r;
}

QCursor SelectionPoint::cursor() {
  return _cursor;
}

void SelectionPoint::setCursor(QCursor c) {
  _cursor = c;
}


void SelectionPoint::draw(QPainter* p, double scalex) {
  QColor brushColor;

  switch (state) {
  case Normal: 
    brushColor = Qt::white;
    break;
  case HighLighted:
    brushColor = Qt::green;
    break;
  case AboutToRemove:
    brushColor = Qt::red;
    break;
  case Inactive:
    brushColor = Qt::gray;
    break;
  }

  QPoint scaledCenter((int)(point.x()*scalex),
		      (int)(point.y()*scalex));

  if (state == HighLighted || state == AboutToRemove) {
    QRect r2(0,0,SELSIZE+4,SELSIZE+4);
    
    r2.moveCenter(scaledCenter);
    QColor color(brushColor);
    color.setAlpha(100);
    p->setPen(QPen(color,4,Qt::SolidLine));
    p->setBrush(Qt::NoBrush);
    p->drawRect(r2);
    
  }

  //  brushColor.setAlpha(230);
  brushColor.setAlpha(200);
  p->setBrush(QBrush(brushColor,Qt::SolidPattern));

  QColor penColor = Qt::black;
  penColor.setAlpha(120);
  QPen pen(penColor, 2, Qt::SolidLine);

  QRect r(0,0,SELSIZE,SELSIZE);
  r.moveCenter( scaledCenter );
  
  p->setPen(pen);
  p->drawRect(r);


}


bool Area::highlightArea;
bool Area::showAlt;


Area::Area()
{
	_finished=false;
	_isSelected=false;
	_name=i18n("noname");
	_listViewItem = nullptr;
	currentHighlighted=-1;
	_type=Area::None;
}

Area* Area::clone() const
{
	Area* areaClone = new Area();
	areaClone->setArea( *this );
	return areaClone;
}

QPolygon Area::coords() const {
	return _coords;
}

QString Area::getHTMLAttributes() const
{
	QString retStr="";

	AttributeIterator it = attributeIterator();
	while (it.hasNext())
	{
	  it.next();
    retStr+=it.key()+"=\""+it.value()+"\" ";
	}

  return retStr;
}

void Area::resetSelectionPointState() {
  setSelectionPointStates(SelectionPoint::Normal);
}

void Area::setSelectionPointStates(SelectionPoint::State st) {
  for (int i=0;i<_selectionPoints.size();i++) {
    _selectionPoints.at(i)->setState(st);
  }
}




void Area::deleteSelectionPoints() {
  for (int i=0;i<_selectionPoints.size();i++) {
    delete _selectionPoints.at(i);
  }
  _selectionPoints.clear();
}

Area::~Area() {
  deleteSelectionPoints();
}

bool Area::contains(const QPoint &) const {
  return false;
}

QString Area::getHTMLCode() const {
  return "";
}

QString Area::attribute(const QString & name) const
{
  return _attributes[name.toLower()];
}

void Area::setAttribute(const QString & name, const QString & value)
{
  _attributes.insert(name.toLower(), value);
  if (value.isEmpty())
     _attributes.remove(name.toLower());
}

AttributeIterator Area::attributeIterator() const
{
  return AttributeIterator(_attributes);
}

bool Area::setCoords(const QString &) {
  return true;
}

void Area::moveSelectionPoint(SelectionPoint*, const QPoint &)
{}

// Default implementation; is specified by subclasses
QString Area::coordsToString() const
{
  return "";
}


Area::ShapeType Area::type() const {
	return _type;
}

void Area::setArea(const Area & copy)
{
  deleteSelectionPoints();
  _coords.clear();
  _coords += copy.coords();
  currentHighlighted=-1;
  
  SelectionPointList points = copy.selectionPoints();
  for (int i=0; i<points.size(); i++) {
    SelectionPoint* np = 
      new SelectionPoint(points.at(i)->getPoint(),points.at(i)->cursor());
    _selectionPoints.append(np);
  }
   
  _finished=copy.finished();
  _isSelected=copy.isSelected();
  _rect = copy.rect();

  AttributeIterator it = copy.attributeIterator();
  while (it.hasNext()) {
    it.next();
    setAttribute(it.key(),it.value());
  }

  setMoving(copy.isMoving());
}

void Area::setFinished(bool b, bool ) { 
  _finished=b; 
}


void Area::setListViewItem(QTreeWidgetItem* item) {
	_listViewItem=item;
}

void Area::deleteListViewItem()
{
	delete _listViewItem;
	_listViewItem = nullptr;
}


void Area::setRect(const QRect & r)
{
  _rect=r;
	updateSelectionPoints();
}

QRect Area::rect() const {
	return _rect;
}

void Area::setMoving(bool b) {
  _isMoving=b;
}


void Area::moveBy(int dx, int dy) {
  _rect.translate(dx,dy);
  _coords.translate(dx,dy);

  for (int i=0;i < _selectionPoints.size(); i++) {
    _selectionPoints.at(i)->translate(dx,dy);
  }
}


void Area::moveTo(int x, int y) {
	int dx = x-rect().left();
	int dy = y-rect().top();
	moveBy(dx,dy);
}

int Area::countSelectionPoints() const
{
  return selectionPoints().size();
}

int Area::addCoord(const QPoint & p)
{
  _coords.resize(_coords.size()+1);
  _coords.setPoint(_coords.size()-1,p);
  _selectionPoints.append(new SelectionPoint(p,QCursor(Qt::PointingHandCursor)));
  setRect(_coords.boundingRect());

  return _coords.size()-1;
}

void Area::insertCoord(int pos, const QPoint & p)
{
  _coords.resize(_coords.size()+1);


  for (int i=_coords.size()-1;i>pos;i--) {
    _coords.setPoint(i,_coords.point(i-1));
  }
  _coords.setPoint(pos, p);

  _selectionPoints.insert(pos,new SelectionPoint(p,QCursor(Qt::PointingHandCursor)));
  setRect(_coords.boundingRect());
}

void Area::removeCoord(int pos) {

  int count =_coords.size();

  if (count<4){
    qCDebug(KIMAGEMAPEDITOR_LOG) << "Danger : trying to remove coordinate from Area with less than 4 coordinates !";
    return;
  }

  for (int i=pos;i<(count-1);i++)
    _coords.setPoint(i, _coords.point(i+1));

  _coords.resize(count-1);
  delete _selectionPoints.takeAt(pos);
  setRect(_coords.boundingRect());
}

bool Area::removeSelectionPoint(SelectionPoint * p)
{
  if (_selectionPoints.contains(p))
  {
    removeCoord(_selectionPoints.indexOf(p));
    return true;
  }

  return false;
}


void Area::moveCoord(int pos, const QPoint & p) {
  _coords.setPoint(pos,p);
  _selectionPoints.at(pos)->setPoint(p);
  setRect(_coords.boundingRect());
}

void Area::setSelected(bool b)
{
  _isSelected=b;
  if (_listViewItem) {
    _listViewItem->setSelected(b);
  }
}

void Area::highlightSelectionPoint(int number){
	currentHighlighted=number;
}

QRect Area::selectionRect() const {
  QRect r = rect();
  r.translate(-SELSIZE*2,-SELSIZE*2);
  r.setSize(r.size()+QSize(SELSIZE*4,SELSIZE*4));

  return r;
}

void Area::setPenAndBrush(QPainter* p) {
  QBrush brush(Qt::NoBrush);
  if (highlightArea) {
    QColor back = Qt::white;
    back.setAlpha(80);
    brush = QBrush(back,Qt::SolidPattern);
  }

  p->setBrush(brush);
  
  QColor front = Qt::white;
  front.setAlpha(200);
  p->setPen(QPen(front,1));
}


void Area::drawAlt(QPainter* p)
{
  double x,y;

  const double scalex = p->transform().m11();
//  double scaley = p.matrix().m12();

  const QTransform oldTransform = p->transform();

  p->setTransform(QTransform(1,oldTransform.m12(), oldTransform.m21(), 1, oldTransform.dx(), oldTransform.dy() ));

  x = (rect().x()+rect().width()/2)*scalex;
  y = (rect().y()+rect().height()/2)*scalex;

  const QFontMetrics metrics = p->fontMetrics();

  const int w = metrics.boundingRect(attribute("alt")).width();
  x -= w/2;
  y += metrics.height()/4;



  if (highlightArea)  {
    p->setPen(Qt::black);
  } else  {
    p->setPen(QPen(QColor("white"),1));
  }

  p->drawText(myround(x),myround(y),attribute("alt"));

  p->setTransform(oldTransform);
}

void Area::draw(QPainter * p)
{

  // Only draw the selection points at base class
  // the rest is done in the derived classes
  if (_isSelected)  {
    // We do not want to have the selection points 
    // scaled, so calculate the unscaled version
    const double scalex = p->transform().m11();
    const QTransform oldTransform = p->transform();
    p->setTransform(QTransform(1,oldTransform.m12(),
			 oldTransform.m21(), 1,
			 oldTransform.dx(),
			 oldTransform.dy() ));

    for (int i=0; i<_selectionPoints.size(); i++) {
      _selectionPoints.at(i)->draw(p,scalex);
    }
    p->setTransform(oldTransform);
  }

  if (showAlt) {
    drawAlt(p);
  }

}

SelectionPoint* Area::onSelectionPoint(const QPoint & p, double zoom) const
{
  
  for (int i=0; i<_selectionPoints.size(); i++) {
    SelectionPoint* sp = _selectionPoints.at(i);
    
    QRect r = sp->getRect();

    r.moveCenter(sp->getPoint()*zoom);

    if (r.contains(p))
    {
      return sp;
    }
  }

  return nullptr;
}




/**
 * returns only the part of the image which is
 * covered by the area
 */
QPixmap Area::cutOut(const QImage & image)
{
	if ( 0>=rect().width()  ||
			 0>=rect().height() ||
       !rect().intersects(image.rect())   )
	{
		QPixmap dummyPix(10,10);
		dummyPix.fill();
		return dummyPix;
	}

	// Get the mask from the subclasses
	QBitmap mask=getMask();

	// The rectangle which is part of the image
	QRect partOfImage=rect();
	QRect partOfMask(0,0,mask.width(),mask.height());


	// If the area is outside of the image make the
	// preview smaller
	if ( (rect().x()+rect().width()) > image.width() ) {
		partOfImage.setWidth( image.width()-rect().x() );
		partOfMask.setWidth(  image.width()-rect().x() );
	}

	if ( (rect().x() < 0) ) {
		partOfImage.setX(0);
		partOfMask.setX(myabs(rect().x()));
	}

	if ( (rect().y()+rect().height()) > image.height() ) {
		partOfImage.setHeight( image.height()-rect().y() );
		partOfMask.setHeight ( image.height()-rect().y() );
	}

	if ( (rect().y() < 0) ) {
		partOfImage.setY(0);
		partOfMask.setY(myabs(rect().y()));
	}

        QImage tempImage=mask.toImage().copy(partOfMask);
	mask = QPixmap::fromImage(tempImage);

//  partOfImage = partOfImage.normalize();
	QImage cut=image.copy(partOfImage);

	QPixmap pix;

//  partOfMask = partOfMask.normalize();
	if (!partOfMask.isValid())
	   qCDebug(KIMAGEMAPEDITOR_LOG) << "PartofMask not valid : " << partOfMask.x() << "," << partOfMask.y() << ","
                << partOfMask.width() << "," << partOfMask.height() << ",";

/*
	QBitmap mask2(partOfMask.width(), partOfMask.height());
	QPainter p4(&mask2);
	p4.drawPixmap( QPoint(0,0) ,mask,partOfMask);
	p4.flush();
	p4.end();
*/

	pix = QPixmap::fromImage(cut);

	//	setHighlightedPixmap(cut, mask);

	QPixmap retPix(pix.width(),pix.height());
  QPainter p3(&retPix);

	// if transparent image fill the background
	// with gimp-like rectangles
	if (!pix.mask().isNull()) {
  	QPixmap backPix(32,32);

  	// Gimp like transparent rectangle
  	QPainter p2(&backPix);
  	p2.fillRect(0,0,32,32,QColor(156,149,156));
  	p2.fillRect(0,16,16,16,QColor(98,105,98));
  	p2.fillRect(16,0,16,16,QColor(98,105,98));

  	p3.setPen(QPen());
  	p3.fillRect(0,0,pix.width(),pix.height(),QBrush(QColor("black"),backPix));
	}


	p3.drawPixmap(QPoint(0,0),pix);
	p3.end();
	retPix.setMask(mask);

	return retPix;
}

QBitmap Area::getMask() const
{
	QBitmap b;
	return b;
}

/********************************************************************
 * RECTANGLE
 *******************************************************************/


RectArea::RectArea()
	: Area()
{
  _type=Area::Rectangle;
  QPoint p(0,0);
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeFDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeBDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeBDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeFDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeVerCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeHorCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeVerCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeHorCursor));
}

RectArea::~RectArea() {
}

Area* RectArea::clone() const
{
	Area* areaClone = new RectArea();
	areaClone->setArea( *this );
	return areaClone;
}

void RectArea::draw(QPainter * p)
{
  setPenAndBrush(p);

  QRect r(rect());
  r.setWidth(r.width()+1);
  r.setHeight(r.height()+1);
  p->drawRect(r);

  Area::draw(p);
}

QBitmap RectArea::getMask() const
{
	QBitmap mask(rect().width(),rect().height());

	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackground(QBrush(Qt::color0));
	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);
	mask.fill(Qt::color1);
	p.end();

	return mask;
}

QString RectArea::coordsToString() const
{
	QString retStr=QString("%1,%2,%3,%4")
					.arg(rect().left())
					.arg(rect().top())
					.arg(rect().right())
					.arg(rect().bottom());

	return retStr;
}

bool RectArea::contains(const QPoint & p) const{
	return rect().contains(p);
}

void RectArea::moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p)
{
	selectionPoint->setPoint(p);
 	int i = _selectionPoints.indexOf(selectionPoint);

 	QRect r2(_rect);
 	switch (i) {
	case 0 :
	  _rect.setLeft(p.x());
	  _rect.setTop(p.y());
	  break;
	case 1 : 
	  _rect.setRight(p.x());
	  _rect.setTop(p.y());
	  break;
	case 2 : 
	  _rect.setLeft(p.x());
	  _rect.setBottom(p.y());
	  break;
	case 3 : 
	  _rect.setRight(p.x());
	  _rect.setBottom(p.y());
	  break;
	case 4 : // top line
	  _rect.setTop(p.y());
	  break;
	case 5 : // right line
	  _rect.setRight(p.x());
	  break;
	case 6 : // bottom
	  _rect.setBottom(p.y());
	  break;
	case 7 : // left
	  _rect.setLeft(p.x());
	  break;
	  
 	}
 	if (! _rect.isValid())
	  _rect=r2;

 	updateSelectionPoints();
}

void RectArea::updateSelectionPoints()
{
  int d = 2;
  QRect r(_rect);
  r.adjust(0,0,1,1);
  int xmid = r.left()+(r.width()/d);
  int ymid = r.top()+(r.height()/d);
  

  _selectionPoints[0]->setPoint(r.topLeft());
  _selectionPoints[1]->setPoint(r.topRight());
  _selectionPoints[2]->setPoint(r.bottomLeft());
  _selectionPoints[3]->setPoint(r.bottomRight());
  _selectionPoints[4]->setPoint(QPoint(xmid,r.top()));
  _selectionPoints[5]->setPoint(QPoint(r.right(),ymid));
  _selectionPoints[6]->setPoint(QPoint(xmid,r.bottom()));
  _selectionPoints[7]->setPoint(QPoint(r.left(),ymid));
}

bool RectArea::setCoords(const QString & s)
{
	_finished=true;

	const QStringList list = s.split(',');
	QRect r;
 	bool ok=true;
 	QStringList::ConstIterator it = list.begin();
	r.setLeft((*it).toInt(&ok,10));it++;
	r.setTop((*it).toInt(&ok,10));it++;
	r.setRight((*it).toInt(&ok,10));it++;
	r.setBottom((*it).toInt(&ok,10));
	if (ok) {
	  setRect(r);
	  return true;
	} else {
	  return false;
	}
}

QString RectArea::getHTMLCode() const {
	QString retStr;
	retStr+="<area ";
	retStr+="shape=\"rect\" ";

	retStr+=getHTMLAttributes();

	retStr+="coords=\""+coordsToString()+"\" ";
	retStr+="/>";
	return retStr;

}

/********************************************************************
 * CIRCLE
 *******************************************************************/


CircleArea::CircleArea()
	: Area()
{
  _type = Area::Circle;
  QPoint p(0,0);
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeFDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeBDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeBDiagCursor));
  _selectionPoints.append(new SelectionPoint(p,Qt::SizeFDiagCursor));
}

CircleArea::~CircleArea() {
}

Area* CircleArea::clone() const
{
	Area* areaClone = new CircleArea();
	areaClone->setArea( *this );
	return areaClone;
}

void CircleArea::draw(QPainter * p)
{
  setPenAndBrush(p);

  QRect r(_rect);
  r.setWidth(r.width()+1);
  r.setHeight(r.height()+1);
  p->drawEllipse(r);

  Area::draw(p);
}

QBitmap CircleArea::getMask() const
{
	QBitmap mask(_rect.width(),_rect.height());

	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackground(QBrush(Qt::color0));
	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);
	p.drawPie(QRect(0,0,_rect.width(),_rect.height()),0,5760);
	p.end();


	return mask;

}

QString CircleArea::coordsToString() const
{
	QString retStr=QString("%1,%2,%3")
					.arg(_rect.center().x())
					.arg(_rect.center().y())
					.arg(_rect.width()/2);

	return retStr;
}

bool CircleArea::contains(const QPoint & p) const
{
	QRegion r(_rect,QRegion::Ellipse);
	return r.contains(p);
}

void CircleArea::moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p)
{
  selectionPoint->setPoint(p);

  int i = _selectionPoints.indexOf(selectionPoint);

  // The code below really sucks, but I have no better idea.
  // it only makes sure that the circle is perfectly round

  QPoint newPoint;
  int diff=myabs(p.x()-_rect.center().x());
  if (myabs(p.y()-_rect.center().y())>diff)
    diff=myabs(p.y()-_rect.center().y());

  newPoint.setX( p.x()-_rect.center().x()<0
		 ? _rect.center().x()-diff
		 :	_rect.center().x()+diff);

  newPoint.setY( p.y()-_rect.center().y()<0
		 ? _rect.center().y()-diff
		 :	_rect.center().y()+diff);

  switch (i) {
  case 0 : if (newPoint.x() < _rect.center().x() &&
	       newPoint.y() < _rect.center().y())
    {
      _rect.setLeft(newPoint.x());
      _rect.setTop(newPoint.y());
    }
    break;
  case 1 : if (newPoint.x() > _rect.center().x() &&
	       newPoint.y() < _rect.center().y())
    {
      _rect.setRight(newPoint.x());
      _rect.setTop(newPoint.y());
    }
    break;
  case 2 : if (newPoint.x() < _rect.center().x() &&
	       newPoint.y() > _rect.center().y())
    {
      _rect.setLeft(newPoint.x());
      _rect.setBottom(newPoint.y());
    }
    break;
  case 3 : if (newPoint.x() > _rect.center().x() &&
	       newPoint.y() > _rect.center().y())
    {
      _rect.setRight(newPoint.x());
      _rect.setBottom(newPoint.y());
    }
    break;
  }



  updateSelectionPoints();

}

void CircleArea::setRect(const QRect & r)
{
	QRect r2 = r;
	if ( r2.height() != r2.width() )
	   r2.setHeight( r2.width() );

	Area::setRect(r2);
}


void CircleArea::updateSelectionPoints()
{
  _selectionPoints[0]->setPoint(_rect.topLeft());
  _selectionPoints[1]->setPoint(_rect.topRight());
  _selectionPoints[2]->setPoint(_rect.bottomLeft());
  _selectionPoints[3]->setPoint(_rect.bottomRight());
}

bool CircleArea::setCoords(const QString & s)
{
	_finished=true;
	const QStringList list = s.split(',');
 	bool ok=true;
 	QStringList::ConstIterator it = list.begin();
 	int x=(*it).toInt(&ok,10);it++;
 	int y=(*it).toInt(&ok,10);it++;
 	int rad=(*it).toInt(&ok,10);
 	if (!ok) return false;
 	QRect r;
 	r.setWidth(rad*2);
 	r.setHeight(rad*2);
 	r.moveCenter(QPoint(x,y));
 	setRect(r);
	return true;
}

QString CircleArea::getHTMLCode() const {
	QString retStr;
	retStr+="<area ";
	retStr+="shape=\"circle\" ";

	retStr+=getHTMLAttributes();

	retStr+="coords=\""+coordsToString()+"\" ";
	retStr+="/>";
	return retStr;

}


/********************************************************************
 * POLYGON
 *******************************************************************/


PolyArea::PolyArea()
	: Area()
{
  _type = Area::Polygon;
}

PolyArea::~PolyArea() {
}

Area* PolyArea::clone() const
{
	Area* areaClone = new PolyArea();
	areaClone->setArea( *this );
	return areaClone;
}

void PolyArea::draw(QPainter * p)
{
  setPenAndBrush(p);

  if (_finished)
    p->drawPolygon( _coords.constData(),_coords.count());
  else {
    p->drawPolyline(_coords.constData(),_coords.count());
  }

  Area::draw(p);
}

QBitmap PolyArea::getMask() const
{
	QBitmap mask(_rect.width(),_rect.height());

	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackground(QBrush(Qt::color0));
	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);
 	p.setClipping(true);
 	QRegion r(_coords);
 	r.translate(-_rect.left(),-_rect.top());
 	p.setClipRegion(r);
 	p.fillRect(QRect(0,0,_rect.width(),_rect.height()),Qt::color1);
	p.end();

	return mask;
}

QString PolyArea::coordsToString() const
{
	QString retStr;

 	for (int i=0;i<_coords.count();i++) {
 		retStr.append(QString("%1,%2,")
 			.arg(_coords.point(i).x())
 			.arg(_coords.point(i).y()));
 	}

 	retStr.remove(retStr.length()-1,1);

 	return retStr;
}

int PolyArea::distance(const QPoint &p1, const QPoint &p2)
{
  QPoint temp = p1-p2;
  return temp.manhattanLength();
}

bool PolyArea::isBetween(const QPoint &p, const QPoint &p1, const QPoint &p2)
{
  int dist = distance(p,p1)+distance(p,p2)-distance(p1,p2);

  if (myabs(dist)<1)
     return true;
  else
     return false;
}

void PolyArea::simplifyCoords()
{
  if (_coords.size()<4)
     return;

  QPoint p = _coords.point(0) - _coords.point(1);

  int i = 1;


  while( (i<_coords.size()) && (_coords.size() > 3) )
  {
    p = _coords.point(i-1) - _coords.point(i);

    if (p.manhattanLength() < 3)
      removeCoord(i);
    else
      i++;
  }

  p = _coords.point(0) - _coords.point(1);

  double angle2;
  double angle1;

  if (p.y()==0)
     angle1 = 1000000000;
  else
    angle1 = (double) p.x() / (double) p.y();

  i=2;

  while( (i<_coords.size()) && (_coords.size() > 3) )
  {
    p = _coords.point(i-1) - _coords.point(i);

    if (p.y()==0)
        angle2 = 1000000000;
    else
      angle2 = (double) p.x() / (double) p.y();

    if ( angle2==angle1 )
    {
      qCDebug(KIMAGEMAPEDITOR_LOG) << "removing " << i-1;
      removeCoord(i-1);
    }
    else
    {
      i++;
      qCDebug(KIMAGEMAPEDITOR_LOG) << "skipping " << i-1 << " cause " << angle1 << "!= " << angle2;
      angle1 = angle2;

    }

  }



}


int PolyArea::addCoord(const QPoint & p)
{
  if (_coords.size()<3)
  {
     return Area::addCoord(p);
  }

  if (_coords.point(_coords.size()-1) == p)
  {
     qCDebug(KIMAGEMAPEDITOR_LOG) << "equal Point added";
     return -1;

  }

  int n=_coords.size();

//  QPoint temp = p-_coords.point(0);
  int nearest = 0;
  int olddist = distance(p,_coords.point(0));
  int mindiff = 999999999;

  // find the two points, which are the nearest one to the new point
  for (int i=1; i <= n; i++)
  {
    int dist = distance(p,_coords.point(i%n));
    int dist2 = distance(_coords.point(i-1),_coords.point(i%n));
    int diff = myabs(dist+olddist-dist2);
    if ( diff<mindiff )
    {
      mindiff = diff;
      nearest = i%n;
    }
    olddist=dist;
  }

  insertCoord(nearest, p);

  return nearest;

}

bool PolyArea::contains(const QPoint & p) const
{
	// A line can't contain a point
 	if (_coords.count() >2 ) {
 		QRegion r(_coords);
 		return r.contains(p);
 	}
 	else
 		return false;
}

void PolyArea::moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p)
{
  selectionPoint->setPoint(p);

  int i = _selectionPoints.indexOf(selectionPoint);
  _coords.setPoint(i,p);
  _rect=_coords.boundingRect();
}

void PolyArea::updateSelectionPoints()
{
  for (int i = 0; i < _selectionPoints.size(); ++i) {
    _selectionPoints.at(i)->setPoint(_coords.point(i));
  }
}

bool PolyArea::setCoords(const QString & s)
{
	_finished=true;
	const QStringList list = s.split(',');
	_coords.clear();
	_selectionPoints.clear();

	for (QStringList::ConstIterator it = list.begin(); it !=list.end(); ++it)
	{
		bool ok=true;
		int newXCoord=(*it).toInt(&ok,10);
		if (!ok) return false;
		it++;
		if (it==list.end())	break;
		int newYCoord=(*it).toInt(&ok,10);
		if (!ok) return false;
		insertCoord(_coords.size(), QPoint(newXCoord,newYCoord));
	}

 	return true;

}

QString PolyArea::getHTMLCode() const {
	QString retStr;
	retStr+="<area ";
	retStr+="shape=\"poly\" ";

	retStr+=getHTMLAttributes();

	retStr+="coords=\""+coordsToString()+"\" ";
	retStr+="/>";
	return retStr;

}

void PolyArea::setFinished(bool b, bool removeLast = true)
{
	// The last Point is the same as the first
	// so delete it
  if (b && removeLast) {
    _coords.resize(_coords.size()-1);
    _selectionPoints.removeLast();
  }

  _finished = b;
}

QRect PolyArea::selectionRect() const
{
	QRect r = _rect;

	r.translate(-10,-10);
	r.setSize(r.size()+QSize(21,21));

	return r;
}



/********************************************************************
 * DEFAULT
 *******************************************************************/


DefaultArea::DefaultArea()
	: Area()
{
	_type=Area::Default;
}

DefaultArea::~DefaultArea() {
}

Area* DefaultArea::clone() const
{
	Area* areaClone = new DefaultArea();
	areaClone->setArea( *this );
	return areaClone;
}

void DefaultArea::draw(QPainter *)
{}


QString DefaultArea::getHTMLCode() const {
	QString retStr;
	retStr+="<area ";
	retStr+="shape=\"default\" ";

	retStr+=getHTMLAttributes();

	retStr+="/>";
	return retStr;

}


/********************************************************************
 * AreaSelection
 *******************************************************************/

AreaSelection::AreaSelection()
	: Area()
{
	_areas = new AreaList();
	_name = "Selection";
	invalidate();
}

AreaSelection::~AreaSelection() {
	delete _areas;
}

Area* AreaSelection::clone() const
{
  AreaSelection* areaClone = new AreaSelection();

  // we want a deep copy of the Areas
  AreaListIterator it=getAreaListIterator();
  while (it.hasNext()) {
    areaClone->add( it.next()->clone() );
  }

  return areaClone;
}


void AreaSelection::add(Area *a)
{
  // if a selection of areas was added get the areas of it
  AreaSelection *selection = nullptr;
  if ( (selection = dynamic_cast <AreaSelection*> ( a ) ) ) {
    AreaList list = selection->getAreaList();
    Area* area;
    foreach(area,list) {
      if ( !_areas->contains( area )) {
	_areas->append( area );  // Must come before area->setSelected
	area->setSelected( true );
      }
    }
  } else {
    if ( !_areas->contains( a )) {
      _areas->append( a );  // Must come before a->setSelected
      a->setSelected( true );
    }
  }

  invalidate();
}


void AreaSelection::setSelectionPointStates(SelectionPoint::State st) {
  AreaListIterator it=getAreaListIterator();
  while(it.hasNext()) {
    it.next()->setSelectionPointStates( st );
  }
}

void AreaSelection::updateSelectionPointStates() {
  SelectionPoint::State st = SelectionPoint::Normal;
  
  if (_areas->count() > 1) 
    st = SelectionPoint::Inactive;

  setSelectionPointStates(st);
}


void AreaSelection::remove(Area *a)
{
  if (!_areas->contains(a))
    return;

  a->setSelected( false );
  _areas->removeAt(_areas->indexOf(a));
  invalidate();
}

void AreaSelection::reset()
{
  AreaListIterator it=getAreaListIterator();
  while (it.hasNext()) {
    it.next()->setSelected( false );
  }

  _areas->clear();
  invalidate();
}

bool AreaSelection::contains(const QPoint & p) const
{
  AreaListIterator it=getAreaListIterator();
  while (it.hasNext()) {
    if ( it.next()->contains( p ) ) {
      return true;
    }
  }

  return false;
}

SelectionPoint* AreaSelection::onSelectionPoint(const QPoint & p, double zoom) const
{

  if (_areas->count() != 1)
    return nullptr;

  return _areas->first()->onSelectionPoint(p,zoom);
}

void AreaSelection::moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p)
{
  // It's only possible to move a SelectionPoint if only one Area is selected
  if (_areas->count() != 1)
    return;

  _areas->first()->moveSelectionPoint(selectionPoint,p);

  invalidate();
}


void AreaSelection::moveBy(int dx, int dy)
{
  AreaListIterator it=getAreaListIterator();
  while (it.hasNext()) {
    it.next()->moveBy(dx,dy);
  }

  Area::moveBy( dx, dy );
  invalidate();
}

QString AreaSelection::typeString() const
{
  // if there is only one Area selected
  // show the name of that Area
  if ( _areas->count()==0 )
    return "";
  else if ( _areas->count()==1 )
    return _areas->first()->typeString();
  else
    return i18n("Number of Areas");

}

Area::ShapeType AreaSelection::type() const
{
  // if there is only one Area selected
  // take the type of that Area
  if ( _areas->count()==0 )
    return Area::None;
  else if ( _areas->count()==1 )
    return _areas->first()->type();
  else
    return Area::Selection;
}

void AreaSelection::resetSelectionPointState() {
  updateSelectionPointStates();
}

void AreaSelection::updateSelectionPoints()
{
  AreaListIterator it=getAreaListIterator();
  while (it.hasNext()) {
    it.next()->updateSelectionPoints();
  }

  invalidate();
}



QRect AreaSelection::selectionRect() const
{
  if (!_selectionCacheValid) {
    _selectionCacheValid=true;
    QRect r;
    AreaListIterator it=getAreaListIterator();
    while (it.hasNext()) {
    	r = r | it.next()->selectionRect();
    }
    _cachedSelectionRect=r;
  }

  return _cachedSelectionRect;
}

int AreaSelection::count() const {
  return _areas->count();
}

bool AreaSelection::isEmpty() const
{
  return _areas->isEmpty();
}


AreaList AreaSelection::getAreaList() const {
  AreaList list(*_areas);
  return list;
}

AreaListIterator AreaSelection::getAreaListIterator() const {
  AreaListIterator it(*_areas);
  return it;
}

void AreaSelection::setArea(const Area & copy)
{
  Area *area = copy.clone();
  AreaSelection *selection = dynamic_cast<AreaSelection*>(area);
  if (selection)
    setAreaSelection(*selection);
  else {
    Area::setArea(copy);
    invalidate();
  }
}

void AreaSelection::setAreaSelection(const AreaSelection & copy)
{
  AreaList* areasCopy = copy._areas;

  if (_areas->count() != areasCopy->count())
    return;

  AreaListIterator it(*_areas);
  AreaListIterator it2(*areasCopy);
  while (it.hasNext()) {
   	it.next()->setArea(*it2.next());
  }

  Area::setArea(copy);
	invalidate();
}

void AreaSelection::setAreaList( const AreaList & areas )
{
  delete _areas;
  _areas = new AreaList(areas);
  invalidate();
}

void AreaSelection::setRect(const QRect & r)
{
	if ( _areas->count()==1 )
	{
		_areas->first()->setRect(r);
	}

	invalidate();
	_rect=rect();
	updateSelectionPoints();
}

QRect AreaSelection::rect() const
{
	if (!_rectCacheValid)
	{
		_rectCacheValid=true;
  	QRect r;
    AreaListIterator it=getAreaListIterator();

    while (it.hasNext()) {
    	r = r | it.next()->rect();
    }

    _cachedRect=r;
  }

  return _cachedRect;
}


int AreaSelection::addCoord(const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->addCoord(p);
		invalidate();
	}

  return 0;
}

void AreaSelection::insertCoord(int pos, const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		_areas->first()->insertCoord(pos, p);
		invalidate();
	}
}

void AreaSelection::removeCoord(int pos)
{
	if ( _areas->count()==1 )
	{
		_areas->first()->removeCoord(pos);
		invalidate();
	}
}

bool AreaSelection::removeSelectionPoint(SelectionPoint* p)
{
  bool result=false;

	if ( _areas->count()==1 )
	{
		result = _areas->first()->removeSelectionPoint(p);
		invalidate();
	}

	return result;
}

const SelectionPointList & AreaSelection::selectionPoints() const
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->selectionPoints();
	}

	return _selectionPoints;
}


void AreaSelection::moveCoord(int pos,const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		_areas->first()->moveCoord(pos,p);
		invalidate();
	}
}

void AreaSelection::highlightSelectionPoint(int i)
{
	if ( _areas->count()==1 )
	{  
		_areas->first()->highlightSelectionPoint(i);
		invalidate();
	}
}


QPolygon AreaSelection::coords() const
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->coords();
	}

	return Area::coords();
}

QString AreaSelection::attribute(const QString & name) const
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->attribute(name);
	}

	return Area::attribute(name);
}

void AreaSelection::setAttribute(const QString & name, const QString & value)
{
  AreaListIterator it=getAreaListIterator();

  while (it.hasNext()) {
    it.next()->setAttribute(name,value);
  }

	Area::setAttribute(name,value);
}

AttributeIterator AreaSelection::attributeIterator() const
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->attributeIterator();
	}

  return AttributeIterator(_attributes);
}

void AreaSelection::setMoving(bool b)
{
  AreaListIterator it=getAreaListIterator();
  
  while (it.hasNext()) {
    it.next()->setMoving(b);
  }

	Area::setMoving(b);
}

bool AreaSelection::isMoving() const
{
	if ( _areas->count()==1 )
	{
		return _areas->first()->isMoving();
	}

	return Area::isMoving();
}


/**
 * Checks if an area is outside the rectangle parameter
 * returns false if an area has no pixel in common with the rectangle parameter
 **/
bool AreaSelection::allAreasWithin(const QRect & r) const
{
  if ( ! r.contains(rect()) )
  {
    AreaListIterator it=getAreaListIterator();

    while (it.hasNext()) {
      if (!it.next()->rect().intersects(r))
        return false;
    }
  }

  return true;
}


void AreaSelection::draw(QPainter *)
{}

void AreaSelection::invalidate() {
  _selectionCacheValid=false;
  _rectCacheValid=false;
  updateSelectionPointStates();
}


