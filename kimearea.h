/***************************************************************************
                          kimearea.h  -  description
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

#ifndef KIMEAREA_H
#define KIMEAREA_H

#include <qrect.h>
#include <qpoint.h>
#include <QList>
//Added by qt3to4:
#include <QPixmap>
#include <klocale.h>
#include <qhash.h>
#include <QHashIterator>
#include "kdeversion.h"
#include <Q3PtrList>

class QPainter;
class QPolygon;
class Q3ListViewItem;
class QBitmap;


typedef QHash<QString,QString> AttributeMap;
typedef QHashIterator<QString,QString> AttributeIterator;

#define SELSIZE 9



class SelectionPoint 
{

public:
  enum State {
    Normal,
    HighLighted,
    AboutToRemove,
    Inactive
  };


  SelectionPoint(QPoint);
  virtual ~SelectionPoint();

  void setState(SelectionPoint::State s);
  State getState() const;

  QPoint getPoint() const;
  void setPoint(QPoint);
  void translate(int,int);

  QRect getRect() const;

  void draw(QPainter*, double);

private:
  QPoint point;
  State state;
  
};

typedef QList<SelectionPoint*> SelectionPointList;


class Area
{
public:
  enum ShapeType { 
    None, 
    Rectangle, 
    Circle, 
    Polygon, 
    Default, 
    Selection 
  };

  static bool highlightArea;
  static bool showAlt;

protected:
	QRect _rect;
	ShapeType _type;
	QString _name;
	QString _href;
	QString _alt;
	QString _target;
	AttributeMap _attributes;
	bool _isSelected;
	bool _finished;
	bool _isMoving;
	int currentHighlighted;
	Q3ListViewItem* _listViewItem;

	QPolygon _coords;
	SelectionPointList _selectionPoints;

	void setPenAndBrush(QPainter* p);
	void drawAlt(QPainter*);
	QString getHTMLAttributes() const;
	void deleteSelectionPoints();

public:
	Area();
	virtual ~Area();

	virtual Area* clone() const;
	// Default implementation; is specified by subclasses
	virtual bool contains(const QPoint &) const;
	// Default implementation; is specified by subclasses
	virtual QString coordsToString() const;
	virtual void draw(QPainter*);

	virtual QBitmap getMask() const;
	virtual	QString getHTMLCode() const;

	virtual void moveBy(int, int);
	virtual void moveTo(int, int);
	virtual void moveSelectionPoint(SelectionPoint*, const QPoint &);

	virtual SelectionPoint* onSelectionPoint(const QPoint &,double zoom) const;
	virtual bool removeSelectionPoint(SelectionPoint* );
	virtual const SelectionPointList & selectionPoints() const { return _selectionPoints; }
	virtual void resetSelectionPointState();
	virtual void setSelectionPointStates(SelectionPoint::State st);

	virtual QRect rect() const;

	virtual QRect selectionRect() const;
	virtual void setArea(const Area &);
	virtual bool setCoords(const QString &);
	/** finished drawing only important for polygon */
	virtual void setFinished(bool b, bool removeLast = true); 
	virtual void setRect(const QRect &);
  virtual void setMoving(bool b);
  virtual bool isMoving() const;
	// Default implementation; is specified by subclasses
	virtual QString typeString() const { return ""; }
	virtual ShapeType type() const;

	virtual void updateSelectionPoints() {};

	// Only interesting for Polygons
	virtual void simplifyCoords() {};
	virtual int addCoord(const QPoint &);
	virtual void insertCoord(int, const QPoint &);
	virtual void removeCoord(int);
	virtual void moveCoord(int,const QPoint &);
	virtual QPolygon coords() const;

	virtual void highlightSelectionPoint(int);

	virtual QString attribute(const QString &) const;
	virtual void setAttribute(const QString &, const QString &);
	virtual AttributeIterator attributeIterator() const;

	QPixmap cutOut(const QImage &) ;
	void setListViewItem(Q3ListViewItem*);
	void deleteListViewItem();
	Q3ListViewItem* listViewItem() const;
	void setName(const QString &);
	QString name() const;
	void setSelected(bool b);
	bool isSelected() const;
	bool finished() const;
	int countSelectionPoints() const;

};



inline Q3ListViewItem* Area::listViewItem() const {
	return _listViewItem;
}

inline void Area::setName(const QString & name) {
	_name=name;
}

inline QString Area::name() const {
	return _name;
}

inline bool Area::isMoving() const {
  return _isMoving;
}


inline bool Area::isSelected() const {
	return _isSelected;
}


inline bool Area::finished() const {
	return _finished;
}

/**
 *	Represents a Rectangle Area
 **/
class RectArea : public Area
{
	public:
		RectArea();
		virtual ~RectArea();

		virtual Area* clone() const;
		virtual bool contains(const QPoint & p) const;
		virtual QString coordsToString() const;
		virtual void draw(QPainter*);
		virtual void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p);
		virtual bool setCoords(const QString & s);
		virtual QString typeString() const { return i18n("Rectangle"); }
		virtual QBitmap getMask() const;
		virtual	QString getHTMLCode() const;
		virtual void updateSelectionPoints();
};


/**
 *	Represents a Circle Area
 **/
class CircleArea : public Area
{
	public:
		CircleArea();
		virtual ~CircleArea();

		virtual Area* clone() const;
		virtual bool contains(const QPoint & p) const;
		virtual QString coordsToString() const;
		virtual void draw(QPainter*);
		virtual void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p);
		virtual bool setCoords(const QString & s);
		virtual void setRect(const QRect & r);
		virtual QString typeString() const { return i18n("Circle"); }
		virtual QBitmap getMask() const;
		virtual QString getHTMLCode() const;
		virtual void updateSelectionPoints();

};

/**
 *	Represents a Rectangle Area
 **/
class PolyArea :public Area
{
	public:
		PolyArea();
		virtual ~PolyArea();

		virtual Area* clone() const;
		virtual bool contains(const QPoint & p) const;
		virtual QString coordsToString() const;
		virtual void draw(QPainter*);
		virtual void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p);
		virtual void simplifyCoords();
  	virtual int addCoord(const QPoint & p);
		virtual bool setCoords(const QString & s);
		virtual QRect selectionRect() const;
		virtual void setFinished(bool b, bool removeLast);
		virtual QString typeString() const { return i18n("Polygon"); }
		virtual QBitmap getMask() const;
		virtual QString getHTMLCode() const;
		virtual void updateSelectionPoints();

  private:
   static int distance(const QPoint &p1, const QPoint &p2);
   static bool isBetween(const QPoint &p, const QPoint &p1, const QPoint &p2);

};

/**
 *	Represents the default Area
 **/
class DefaultArea :public Area
{
 public:
  DefaultArea();
  virtual ~DefaultArea();

  virtual Area* clone() const;
  // the default area isn't drawn
  virtual void draw(QPainter*);
  virtual QString typeString() const { 
    return i18n("Default"); 
  }
  virtual QString getHTMLCode() const;

};


typedef Q3PtrList<Area> AreaList;
typedef Q3PtrListIterator<Area> AreaListIterator;

/**
 *	This class represents a selection of areas
 *  all operations performed on this class
 *  will be performed on the selected Areas
 *  the only actions that can be used is the
 *  move action
 **/
class AreaSelection : public Area {
public :
  AreaSelection();
  virtual ~AreaSelection();

  /**
   * New Methods
   */

  // Adding automatically selects the area
  void add(Area *a);

  // Removing automatically deselects the area
  void remove(Area *a);

  // Removes all areas from the list and deselects them
  void reset();

  int count() const;

  AreaList getAreaList() const;
  AreaListIterator getAreaListIterator() const;
  void setAreaList( const AreaList & areas );

  bool isEmpty() const;

  /**
   * Overiden Methods of the Area class
   */
  virtual bool contains(const QPoint & p) const;

  /**
   *
   **/
  virtual SelectionPoint* onSelectionPoint(const QPoint & p, double zoom) const;

  /**
   * Only if one Area is selected the moveSelectionPoint method
   * of that Area will be called
   **/
  virtual void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p);

  virtual const SelectionPointList & selectionPoints() const;

  /**
   * All Areas will be moved by dx and dy
   **/
  virtual void moveBy(int dx, int dy);

  /**
   * Calls for every selected Area the setArea with the
   * corresponding Area in the copy Selection.
   * IMPORTANT : works only if the copy Area is an AreaSelection
   * and have the same number of Areas
   **/
  virtual void setArea(const Area & copy);
  virtual void setAreaSelection(const AreaSelection & copy);

  /**
   * If only one Area is selected the setRect method of that Area
   * will be called
   **/
  virtual void setRect(const QRect & r);
  virtual QRect rect() const;


  virtual QString typeString() const;
  virtual ShapeType type() const;

  // The selection is only a container
  // so it is never drawn
  virtual void draw(QPainter*);


  /**
   * A deep copy of the Areas
   **/
  virtual Area* clone() const;

  virtual void resetSelectionPointState();
  virtual void setSelectionPointStates(SelectionPoint::State st);
  void updateSelectionPointStates();

  virtual void updateSelectionPoints();
  virtual int addCoord(const QPoint & p);
  virtual void insertCoord(int pos, const QPoint & p);
  virtual void removeCoord(int pos);
  virtual bool removeSelectionPoint(SelectionPoint *);
  virtual void moveCoord(int pos,const QPoint & p);
  virtual QPolygon coords() const;
  virtual void highlightSelectionPoint(int);

  virtual QRect selectionRect() const;

  virtual QString attribute(const QString & name) const;
  virtual void setAttribute(const QString & name, const QString & value);
  virtual AttributeIterator attributeIterator() const;

  virtual void setMoving(bool b);
  virtual bool isMoving() const;


  bool allAreasWithin(const QRect & r) const;

  // makes the cache invalid
  void invalidate();

private :
  AreaList *_areas;

  // The selectionRect and the rect are cached
  // so even in const functions they must be changeable
  mutable QRect _cachedSelectionRect;
  mutable QRect _cachedRect;
  mutable bool _selectionCacheValid;
  mutable bool _rectCacheValid;

};


#endif


