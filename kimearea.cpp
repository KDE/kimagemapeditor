/***************************************************************************
                          kimearea.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 2001
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

#include <qbitmap.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpalette.h>
#include <qcolor.h>
#include <qlistview.h>

#include <kdebug.h>

#include "kimearea.h"
#include "kimecommon.h"


#define SELSIZE 7


bool Area::highlightArea;
bool Area::showAlt;


Area::Area()
{
	_coords=new QPointArray();
	_selectionPoints= new SelectionPointList();
	_selectionPoints->setAutoDelete(true);
	_finished=false;
	_isSelected=false;
	_name=i18n("noname");
	_listViewItem=0L;
	currentHighlighted=-1;
	_type=Area::None;
	_highlightedPixmap=0L;
	
}

Area* Area::clone() const
{
	Area* areaClone = new Area();
	areaClone->setArea( *this );
	return areaClone;
}

QPointArray* Area::coords() const {
	return _coords;
}

QString Area::getHTMLAttributes() const
{
	QString retStr="";
	
	for (AttributeIterator it = firstAttribute();it!=lastAttribute();++it)
	{
    retStr+=it.key()+"=\""+it.data()+"\" ";	
	}
	
  return retStr;
}


Area::~Area() {
  delete _coords;
  delete _selectionPoints;
  delete _highlightedPixmap;

}

bool Area::contains(const QPoint &) const {
  return false;
}

QString Area::getHTMLCode() const {
  return "";
}

QString Area::attribute(const QString & name) const
{
  return _attributes[name.lower()];
}

void Area::setAttribute(const QString & name, const QString & value)
{
  _attributes.replace(name.lower(),value);
  if (value.isEmpty())
     _attributes.remove(name.lower());
}

AttributeIterator Area::firstAttribute() const
{
  return _attributes.begin();
}

AttributeIterator Area::lastAttribute() const
{
  return _attributes.end();
}


bool Area::setCoords(const QString &) {
  return true;
}

void Area::moveSelectionPoint(QRect*, const QPoint &)
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
	delete _coords;
	delete _selectionPoints;
 	_coords=new QPointArray(copy.coords()->copy());
	_selectionPoints= new SelectionPointList();
	currentHighlighted=-1;
	
	// Need a deep copy of the list
 	for (QRect *r=copy.selectionPoints()->first();r!=0L;r=copy.selectionPoints()->next())
 			_selectionPoints->append(new QRect( r->topLeft(),r->bottomRight() ) );
  			
	_finished=copy.finished();
	_isSelected=copy.isSelected();
  _rect = copy.rect();
	
	for (AttributeIterator it = copy.firstAttribute();it!=copy.lastAttribute();++it)
	{
    setAttribute(it.key(),it.data());	
	}
	
	setMoving(copy.isMoving());

//	_listViewItem=0L;
	
}

void Area::setListViewItem(QListViewItem* item) {
	_listViewItem=item;
}

void Area::deleteListViewItem()
{
	delete _listViewItem;
	_listViewItem = 0L;
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
	_rect.moveBy(dx,dy);
	for (uint i=0;i<_coords->size();i++) {
		int newX=_coords->point(i).x()+dx;
		int newY=_coords->point(i).y()+dy;
		_coords->setPoint(i,newX,newY);
	}
	
	for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next()) {
		r->moveBy(dx,dy);
	}
}


void Area::moveTo(int x, int y) {
	int dx=x-rect().left();
	int dy=y-rect().top();
	moveBy(dx,dy);
}

uint Area::countSelectionPoints() const
{
  return (uint) selectionPoints()->count();
}

int Area::addCoord(const QPoint & p)
{
	_coords->resize(_coords->size()+1);
	_coords->setPoint(_coords->size()-1,p);
	
	QRect *r= new QRect(0,0,SELSIZE,SELSIZE);
	r->moveCenter(p);
	_selectionPoints->append(r);
	setRect(_coords->boundingRect());

  return _coords->size()-1;
}

void Area::insertCoord(int pos, const QPoint & p)
{

/*
  kdDebug() << p.x() << "," << p.y() << endl;

  if ( _coords->size()>0 )
  {
    for (int i=0; i<_coords->size(); i++)
    {
      if (p==_coords->point(i))
      {
        kdDebug() << "same Point already exists" << endl;
        return;
      }

    }
  }
*/
	_coords->resize(_coords->size()+1);
	

	for (int i=_coords->size()-1;i>pos;i--) {
		_coords->setPoint(i,_coords->point(i-1));
	}
	_coords->setPoint(pos, p);
	
	QRect *r= new QRect(0,0,SELSIZE,SELSIZE);
	r->moveCenter(p);
	_selectionPoints->insert(pos,r);
	setRect(_coords->boundingRect());
}

void Area::removeCoord(int pos) {

	int count=_coords->size();	

	if (count<4)
	{
	   kdDebug() << "Danger : trying to remove coordinate from Area with less then 4 coordinates !" << endl;
	   return;
	}
	
	for (int i=pos;i<(count-1);i++)
		_coords->setPoint(i, _coords->point(i+1));
	
	_coords->resize(count-1);
	_selectionPoints->remove(pos);
	setRect(_coords->boundingRect());
}

bool Area::removeSelectionPoint(QRect * r)
{
  if (_selectionPoints->contains(r))
  {
    removeCoord(_selectionPoints->find(r));
    return true;
  }

  return false;
}


void Area::moveCoord(int pos, const QPoint & p) {
	_coords->setPoint(pos,p);
	_selectionPoints->at(pos)->moveCenter(p);
	setRect(_coords->boundingRect());
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
	r.moveBy(-SELSIZE*2,-SELSIZE*2);
	r.setSize(r.size()+QSize(SELSIZE*4,SELSIZE*4));
	
	return r;
}

void Area::drawHighlighting(QPainter & p)
{
	if (Area::highlightArea && !isMoving() && _highlightedPixmap)
	{
  	p.setRasterOp(Qt::CopyROP);
  	
  	QPoint point = QPoint(rect().x(),rect().y());
    if (point.x()<0)
        point.setX(0);
     if (point.y()<0)
        point.setY(0);
  	
	  p.drawPixmap( point, *_highlightedPixmap);
	
	}
}

void Area::drawAlt(QPainter & p)
{
  double x,y;

  double scalex = p.worldMatrix().m11();
//  double scaley = p.worldMatrix().m12();

  QWMatrix oldMatrix = p.worldMatrix();

  p.setWorldMatrix(QWMatrix(1,oldMatrix.m12(), oldMatrix.m21(), 1, oldMatrix.dx(), oldMatrix.dy() ));

  x = (rect().x()+rect().width()/2)*scalex;
  y = (rect().y()+rect().height()/2)*scalex;

  QFontMetrics metrics = p.fontMetrics();

  int w = metrics.width(attribute("alt"));
  x -= w/2;
  y += metrics.height()/4;



  if (highlightArea)
  {
    p.setRasterOp(Qt::CopyROP);
    p.setPen(Qt::black);
  }
  else
  {
    p.setRasterOp(Qt::XorROP);
  	p.setPen(QPen(QColor("white"),1));
	}

  p.drawText(myround(x),myround(y),attribute("alt"));

  p.setWorldMatrix(oldMatrix);
}

void Area::draw(QPainter & p)
{

  // Only draw the selection points at base class
  // the rest is done in the derived classes
	if (_isSelected)
  {
		int i=0;

    double scalex = p.worldMatrix().m11();
//    double scaley = p.worldMatrix().m12();

    QWMatrix oldMatrix = p.worldMatrix();

    p.setWorldMatrix(QWMatrix(1,oldMatrix.m12(), oldMatrix.m21(), 1, oldMatrix.dx(), oldMatrix.dy() ));

		for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next()) {

      // Draw a green circle around the selected point ( only when editing a polygon )
			if (i==currentHighlighted) {
				QRect r2(0,0,15,15);
				r2.moveCenter(r->center()*scalex);
				p.setRasterOp(Qt::CopyROP);
				p.setPen(QPen(QColor("lightgreen"),2));
				p.drawEllipse(r2);
				p.setRasterOp(Qt::XorROP);
				p.setPen(QPen(QColor("white"),1));
			}

			// Draw the selection point
			p.setRasterOp(Qt::XorROP);

			QRect r3(*r);
      int d = 1;
      if (scalex > 2) d=0;

      r3.moveCenter( QPoint((int)(r3.center().x()*scalex),(int)(r3.center().y()*scalex)) );

 			p.fillRect(r3,QBrush("white"));
/*
			QRect r3(*r);
      r3.moveTopLeft( QPoint(r3.left()*scalex+2*(scalex-1), r3.top()*scalex+2*(scalex-1)) );

			r3.setSize(r3.size()+QSize(2,2));
//+			r3.moveBy(-1,-1);	
			p.setRasterOp(Qt::CopyROP);
			p.setPen(QPen(QColor("lightgreen"),1));
			p.setBrush(QColor("lightgreen"));
			p.drawPie(r3,0,5760);
			p.setPen(QPen(QColor("black"),1));
			r3.setSize(r3.size()+QSize(2,2));
			r3.moveBy(-1,-1);			
			p.drawEllipse(r3);
*/			
  		i++;
		}
    p.setWorldMatrix(oldMatrix);


	}

  if (showAlt)
  {
     drawAlt(p);
   }
  p.setRasterOp(Qt::XorROP);
	
}

QRect* Area::onSelectionPoint(const QPoint & p, double zoom) const
{
	for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next())
  {
    QRect r2(r->topLeft(),r->bottomRight());

    r2.moveCenter(r2.center()*zoom);

    if (r2.contains(p))
    {

			return r;
    }
	}
	
	return 0L;
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
    delete _highlightedPixmap;
    _highlightedPixmap = 0L;
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
		
  QImage tempImage=mask.convertToImage().copy(partOfMask);
	mask.convertFromImage(tempImage);

//  partOfImage = partOfImage.normalize();
	QImage cut=image.copy(partOfImage);
	
	QPixmap pix;
	
//  partOfMask = partOfMask.normalize();	
	if (!partOfMask.isValid())
	   kdDebug() << "PartofMask not valid : " << partOfMask.x() << "," << partOfMask.y() << ","
	            << partOfMask.width() << "," << partOfMask.height() << "," << endl;

/*	            	
	QBitmap mask2(partOfMask.width(), partOfMask.height());
	QPainter p4(&mask2);
	p4.drawPixmap( QPoint(0,0) ,mask,partOfMask);
	p4.flush();
	p4.end();
*/
	
	pix.convertFromImage(cut);

	setHighlightedPixmap(cut, mask);
		
	QPixmap retPix(pix.width(),pix.height());
  QPainter p3(&retPix);
	
	// if transparent image fill the background
	// with gimp-like rectangles
	if (pix.mask()) {
  	QPixmap backPix(32,32);
  	
  	// Gimp like transparent rectangle
  	QPainter p2(&backPix);
  	p2.fillRect(0,0,32,32,QColor(156,149,156));
  	p2.fillRect(0,16,16,16,QColor(98,105,98));
  	p2.fillRect(16,0,16,16,QColor(98,105,98));
  	p2.flush();
  	
  	p3.setPen(QPen());
  	p3.fillRect(0,0,pix.width(),pix.height(),QBrush(QColor("black"),backPix));
	}
	
	
	p3.drawPixmap(QPoint(0,0),pix);
	p3.flush();
	p3.end();
	retPix.setMask(mask);

	return retPix;
}

QBitmap Area::getMask() const
{
	QBitmap b;
	return b;
}

void Area::setHighlightedPixmap( QImage & im, QBitmap & mask )
{
  if (!Area::highlightArea)
     return;

  delete _highlightedPixmap;

  QImage image = im.convertDepth( 32 );
  QSize size = image.size();
  QColor pixel;
  double r,g,b;


  // highlight every pixel
  for (int y=0; y < size.height(); y++)
  {
    for (int x=0; x < size.width(); x++)
    {
      r = qRed(image.pixel(x,y));
      g = qGreen(image.pixel(x,y));
      b = qBlue(image.pixel(x,y));
      r = (r *123 / 255)+132;
      g = (g *123 / 255)+132;
      b = (b *123 / 255)+132;

      pixel.setRgb( (int) r, (int) g, (int) b);
      image.setPixel(x,y, pixel.rgb());
    }
  }

  _highlightedPixmap = new QPixmap();
  _highlightedPixmap->convertFromImage( image );
  _highlightedPixmap->setMask( mask );

  if (_highlightedPixmap->isNull())
     kdDebug() << "HighlightedPixmap is null" << endl;

}

/********************************************************************
 * RECTANGLE
 *******************************************************************/


RectArea::RectArea()
	: Area()
{
 	QRect *p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
	_type=Area::Rectangle;
 	
}

RectArea::~RectArea() {
}

Area* RectArea::clone() const
{
	Area* areaClone = new RectArea();
	areaClone->setArea( *this );
	return areaClone;
}

void RectArea::draw(QPainter & p)
{

  drawHighlighting(p);
//  p.setRasterOp(Qt::CopyROP);
//  p.setRasterOp(Qt:: OrROP);
//  QBrush b(QBrush::SolidPattern);
//  QBrush b(QBrush::Dense4Pattern);
//  QBrush b(QBrush::BDiagPattern);
//	b.setColor(QColor(32,32,32));
//	p.fillRect(rect(), b);
	
	p.setRasterOp(Qt::XorROP);
	p.setPen(QPen(QColor("white"),1));
  QRect r(rect());
  r.setWidth(r.width()+1);
  r.setHeight(r.height()+1);
	p.drawRect(r);
	
	Area::draw(p);
}

QBitmap RectArea::getMask() const
{
	QBitmap mask(rect().width(),rect().height());
	
	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackgroundColor(Qt::color0);
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

void RectArea::moveSelectionPoint(QRect* selectionPoint, const QPoint & p)
{
	selectionPoint->moveCenter(p);
 	int i=0;
 	for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next()) {
 		if (r==selectionPoint)
 			break;
 		i++;
 	}
 	QRect r2(_rect);
 	switch (i) {
 		case 0 : _rect.setLeft(p.x());
 						 _rect.setTop(p.y());
 			break;
 		case 1 : _rect.setRight(p.x());
 		 				 _rect.setTop(p.y());
 			break;
 		case 2 : _rect.setLeft(p.x());
 						 _rect.setBottom(p.y());
 			break;
 		case 3 : _rect.setRight(p.x());
 	 					 _rect.setBottom(p.y());
 			break;
 	}			
 	if ( ! _rect.isValid())
 		_rect=r2;
				
 	updateSelectionPoints();
}

void RectArea::updateSelectionPoints()
{
 	_selectionPoints->first()->moveCenter(_rect.topLeft());
 	_selectionPoints->next()->moveCenter(_rect.topRight()+QPoint(1,0));
 	_selectionPoints->next()->moveCenter(_rect.bottomLeft()+QPoint(0,1));
 	_selectionPoints->next()->moveCenter(_rect.bottomRight()+QPoint(1,1));
}

bool RectArea::setCoords(const QString & s)
{
	_finished=true;
	
	QStringList list=QStringList::split(",",s);
	QRect r;
 	bool ok=true;
 	QStringList::Iterator it = list.begin();
	r.setLeft((*it).toInt(&ok,10));it++;
	r.setTop((*it).toInt(&ok,10));it++;
	r.setRight((*it).toInt(&ok,10));it++;
	r.setBottom((*it).toInt(&ok,10));
	if (ok) {
		setRect(r);
		return true;
	} else
		return false;
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
	_type=Area::Circle;
 	QRect *p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
 	p = new QRect(0,0,SELSIZE,SELSIZE);
 	_selectionPoints->append(p);
}

CircleArea::~CircleArea() {
}

Area* CircleArea::clone() const
{
	Area* areaClone = new CircleArea();
	areaClone->setArea( *this );
	return areaClone;
}

void CircleArea::draw(QPainter & p)
{
  drawHighlighting(p);

/*
  p.setRasterOp(Qt::CopyROP);
	QBrush bold = p.brush();
	QBrush b(QBrush::Dense5Pattern);
	b.setColor(QColor("green"));
	p.setBrush(b);
	QRect r = _rect;
	r.moveBy(1,1);
	r.setSize( r.size()-QSize(2,2) );
	p.drawChord(r,0,5760);
	p.setBrush(bold);
*/	
  p.setRasterOp(Qt::XorROP);
  p.setPen(QPen(QColor("white"),1));

  QRect r(_rect);
  r.setWidth(r.width()+1);
  r.setHeight(r.height()+1);
	p.drawEllipse(r);
	
	Area::draw(p);
}

QBitmap CircleArea::getMask() const
{
	QBitmap mask(_rect.width(),_rect.height());
	
	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackgroundColor(Qt::color0);
	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);
	p.drawPie(QRect(0,0,_rect.width(),_rect.height()),0,5760);
	p.flush();
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

void CircleArea::moveSelectionPoint(QRect* selectionPoint, const QPoint & p)
{
	selectionPoint->moveCenter(p);

 	int i=0;
 	for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next()) {
 		if (r==selectionPoint)
 			break;
 		i++;
 	}
			
 	// The code below really sucks, but I have no better idea.
 	// it only makes sure that the circle is perfektly round
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
 	_selectionPoints->first()->moveCenter(_rect.topLeft());
 	_selectionPoints->next()->moveCenter(_rect.topRight());
 	_selectionPoints->next()->moveCenter(_rect.bottomLeft());
 	_selectionPoints->next()->moveCenter(_rect.bottomRight());
}

bool CircleArea::setCoords(const QString & s)
{
	_finished=true;
	QStringList list=QStringList::split(",",s);
 	bool ok=true;
 	QStringList::Iterator it = list.begin();
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
	_type=Area::Polygon;
}

PolyArea::~PolyArea() {
}

Area* PolyArea::clone() const
{
	Area* areaClone = new PolyArea();
	areaClone->setArea( *this );
	return areaClone;
}

void PolyArea::draw(QPainter & p)
{
  drawHighlighting(p);
	
	p.setRasterOp(Qt::XorROP);
	p.setPen(QPen(QColor("white"),1));
 	if (_coords->count()==0) return;



 	if (_finished)
    p.drawPolygon ( *_coords,false,0,_coords->count());
  else
    p.drawPolyline ( *_coords,0,_coords->count());

/*
 	p.moveTo(_coords->point(0));
 	for (int i=1;i<_coords->count();i++)
 		p.lineTo(_coords->point(i));
									
 	if (_finished)
 		p.lineTo(_coords->point(0));
*/ 		
	Area::draw(p);
}

QBitmap PolyArea::getMask() const
{
	QBitmap mask(_rect.width(),_rect.height());
	
	mask.fill(Qt::color0);
	QPainter p(&mask);
	p.setBackgroundColor(Qt::color0);
	p.setPen(Qt::color1);
	p.setBrush(Qt::color1);
 	p.setClipping(true);
 	QRegion r(*_coords);
 	r.translate(-_rect.left(),-_rect.top());
 	p.setClipRegion(r);
 	p.fillRect(QRect(0,0,_rect.width(),_rect.height()),Qt::color1);
 	p.flush();
	p.end();
	
	return mask;
}

QString PolyArea::coordsToString() const
{
	QString retStr;				
 	
 	for (uint i=0;i<_coords->count();i++) {
 		retStr.append(QString("%1,%2,")
 			.arg(_coords->point(i).x())
 			.arg(_coords->point(i).y()));
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
  if (_coords->size()<4)
     return;

  QPoint p = _coords->point(0) - _coords->point(1);

  uint i = 1;


  while( (i<_coords->size()) && (_coords->size() > 3) )
  {
    p = _coords->point(i-1) - _coords->point(i);

    if (p.manhattanLength() < 3)
      removeCoord(i);
    else
      i++;
  }

  p = _coords->point(0) - _coords->point(1);

  double angle2;
  double angle1;

  if (p.y()==0)
     angle1 = 1000000000;
  else
    angle1 = (double) p.x() / (double) p.y();

  i=2;

  while( (i<_coords->size()) && (_coords->size() > 3) )
  {
    p = _coords->point(i-1) - _coords->point(i);

    if (p.y()==0)
        angle2 = 1000000000;
    else
      angle2 = (double) p.x() / (double) p.y();

    if ( angle2==angle1 )
    {
      kdDebug() << "removing " << i-1 << endl;
      removeCoord(i-1);
    }
    else
    {
      i++;
      kdDebug() << "skipping " << i-1 << " cause " << angle1 << "!= " << angle2 << endl;
      angle1 = angle2;

    }

  }



}


int PolyArea::addCoord(const QPoint & p)
{
  if (_coords->size()<3)
  {
     return Area::addCoord(p);
  }

  if (_coords->point(_coords->size()-1) == p)
  {
     kdDebug() << "equal Point added" << endl;
     return -1;

  }

  int n=_coords->size();

//  QPoint temp = p-_coords->point(0);
  int nearest = 0;
  int olddist = distance(p,_coords->point(0));
  int mindiff = 999999999;

  // find the two points, which are the nearest one to the new point
  for (int i=1; i <= n; i++)
  {
    int dist = distance(p,_coords->point(i%n));
    int dist2 = distance(_coords->point(i-1),_coords->point(i%n));
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
 	if (_coords->count() >2 ) {
 		QRegion r(*_coords);
 		return r.contains(p);
 	}
 	else
 		return false;
}

void PolyArea::moveSelectionPoint(QRect* selectionPoint, const QPoint & p)
{
	selectionPoint->moveCenter(p);

 	int i=0;
 	for (QRect *r=_selectionPoints->first();r!=0L;r=_selectionPoints->next()) {
 		if (r==selectionPoint)
 			break;
 		i++;
 	}
 	_coords->setPoint(i,p);
 	_rect=_coords->boundingRect();
}

void PolyArea::updateSelectionPoints()
{
	QRect *r;
 	r=_selectionPoints->first();
 	
 	for (uint i=0;i<_coords->size();i++)	
 	{
 		r->moveCenter(_coords->point(i));				
 		r=_selectionPoints->next();
 	}

}

bool PolyArea::setCoords(const QString & s)
{
	_finished=true;
	QStringList list=QStringList::split(",",s);
	_coords=new QPointArray();
	_selectionPoints= new SelectionPointList();
	
	for (QStringList::Iterator it = list.begin(); it !=list.end(); it++)
	{
		bool ok=true;
		int newXCoord=(*it).toInt(&ok,10);
		if (!ok) return false;
		it++;
		if (it==list.end())	break;
		int newYCoord=(*it).toInt(&ok,10);
		if (!ok) return false;
		insertCoord(_coords->size(), QPoint(newXCoord,newYCoord));
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

void PolyArea::setFinished(bool b)
{
	// The last Point is the same as the first
	// so delete it
	_coords->resize(_coords->size()-1);
	_selectionPoints->removeLast();
	_finished=b;
}

QRect PolyArea::selectionRect() const
{
	QRect r = _rect;
	
	r.moveBy(-10,-10);
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

void DefaultArea::draw(QPainter &)
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

  for ( ; it.current() != 0L; ++it )
  {
		areaClone->add( it.current()->clone() );
	}
	
//	areaClone->setArea( *this );
	
	return areaClone;
}


void AreaSelection::add(Area *a)
{

	// if a selection of areas was added get the areas of it
	AreaSelection *selection=0L;
	if ( (selection = dynamic_cast <AreaSelection*> ( a ) ) )
	{
		AreaList list = selection->getAreaList();
		
		for (Area* area = list.first(); area != 0L; area = list.next() )
		{
			if ( _areas->find( area ) == -1 ) {
				_areas->append( area );  // Must come before area->setSelected
				area->setSelected( true );
			}
		}
	}
	else
	{
		if ( _areas->find( a ) == -1 ) {
  		_areas->append( a );  // Must come before a->setSelected
  		a->setSelected( true );
  	}
  }

	invalidate();
}

void AreaSelection::remove(Area *a)
{
	if (_areas->find(a) == -1)
		return;
		
	a->setSelected( false );	
	_areas->remove( a );
	invalidate();
}

void AreaSelection::reset()
{
  AreaListIterator it=getAreaListIterator();

  for ( ; it.current() != 0L; ++it )
  {
  	it.current()->setSelected( false );
  }
 	
 	_areas->clear();
	invalidate();
}

bool AreaSelection::contains(const QPoint & p) const
{
	bool b=false;
  AreaListIterator it=getAreaListIterator();

  for ( ; it.current() != 0L; ++it )
  {
  	if ( it.current()->contains( p ) )
  	{
  		b=true;
  		break;
  	}
  }

  return b;
}

QRect* AreaSelection::onSelectionPoint(const QPoint & p, double zoom) const
{
  AreaListIterator it=getAreaListIterator();
	
	if (it.count() != 1)
		return 0L;

	QRect* retRect=0L;		
		
  for ( ; it.current() != 0L; ++it )
  {
  	if ( (retRect = it.current()->onSelectionPoint( p , zoom) ) )
		{  		
  		break;
  	}
  }

  return retRect;
}

void AreaSelection::moveSelectionPoint(QRect* selectionPoint, const QPoint & p)
{
	// It's only possible to move a SelectionPoint if only one Area is selected
	if (_areas->count() != 1)
		return;
		
	_areas->getFirst()->moveSelectionPoint(selectionPoint,p);		

	invalidate();
}


void AreaSelection::moveBy(int dx, int dy)
{
  AreaListIterator it=getAreaListIterator();

  for ( ; it.current() != 0L; ++it )
  	it.current()->moveBy(dx,dy);

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
		return _areas->getFirst()->typeString();
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
		return _areas->getFirst()->type();
	else
		return Area::Selection;
}

void AreaSelection::updateSelectionPoints()
{

  AreaListIterator it=getAreaListIterator();
	
  for ( ; it.current() != 0L; ++it )
  {
		it.current()->updateSelectionPoints();
  }

 	invalidate();

}



QRect AreaSelection::selectionRect() const
{
	if (!_selectionCacheValid)
	{
		_selectionCacheValid=true;
  	QRect r;
    AreaListIterator it=getAreaListIterator();

    for ( ; it.current() != 0L; ++it )
    	r = r | it.current()->selectionRect();

   	_cachedSelectionRect=r;
  }

  return _cachedSelectionRect;
}

uint AreaSelection::count() const {
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
  AreaListIterator it=getAreaListIterator();
	AreaListIterator it2=copy.getAreaListIterator();
		
	if (it.count() != it2.count())
		return;

  for ( ; it.current() != 0L; ++it, ++it2 )
   	it.current()->setArea(*it2.current());

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
		_areas->getFirst()->setRect(r);
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

    for ( ; it.current() != 0L; ++it )
    	r = r | it.current()->rect();

    _cachedRect=r;
  }

  return _cachedRect;
}


int AreaSelection::addCoord(const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->addCoord(p);
		invalidate();
	}

  return 0;	
}

void AreaSelection::insertCoord(int pos, const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		_areas->getFirst()->insertCoord(pos, p);
		invalidate();
	}	
}

void AreaSelection::removeCoord(int pos)
{
	if ( _areas->count()==1 )
	{
		_areas->getFirst()->removeCoord(pos);
		invalidate();
	}	
}

bool AreaSelection::removeSelectionPoint(QRect * r)
{
  bool result=false;

	if ( _areas->count()==1 )
	{
		result = _areas->getFirst()->removeSelectionPoint(r);
		invalidate();
	}	
	
	return result;
}

SelectionPointList* AreaSelection::selectionPoints() const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->selectionPoints();
	}	
	
	return _selectionPoints;
}


void AreaSelection::moveCoord(int pos,const QPoint & p)
{
	if ( _areas->count()==1 )
	{
		_areas->getFirst()->moveCoord(pos,p);
		invalidate();
	}	
}

void AreaSelection::highlightSelectionPoint(int i)
{
	if ( _areas->count()==1 )
	{
		_areas->getFirst()->highlightSelectionPoint(i);
		invalidate();
	}	
}


QPointArray* AreaSelection::coords() const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->coords();
	}	
	
	return Area::coords();
}

QString AreaSelection::attribute(const QString & name) const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->attribute(name);
	}	
	
	return Area::attribute(name);
}

void AreaSelection::setAttribute(const QString & name, const QString & value)
{
  AreaListIterator it=getAreaListIterator();

  for ( ; it.current() != 0L; ++it )
   	it.current()->setAttribute(name,value);
	
	Area::setAttribute(name,value);
}

AttributeIterator AreaSelection::firstAttribute() const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->firstAttribute();
	}	

  return _attributes.begin();
}

AttributeIterator AreaSelection::lastAttribute() const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->lastAttribute();
	}	

  return _attributes.end();
}

void AreaSelection::setMoving(bool b)
{
  AreaListIterator it=getAreaListIterator();

  for ( ; it.current() != 0L; ++it )
   	it.current()->setMoving(b);
	
	Area::setMoving(b);
}

bool AreaSelection::isMoving() const
{
	if ( _areas->count()==1 )
	{
		return _areas->getFirst()->isMoving();
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

    for ( ; it.current() != 0L; ++it )
      if (!it.current()->rect().intersects(r))
          return false;
  }

  return true;
}


void AreaSelection::draw(QPainter &)
{}


