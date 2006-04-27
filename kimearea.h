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
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3PointArray>
#include <klocale.h>
#include <qmap.h>

#include "kdeversion.h"

class QPainter;
class Q3PointArray;
class Q3ListViewItem;
class QBitmap;

typedef Q3PtrList<QRect> SelectionPointList;

typedef QMap<QString,QString> AttributeMap;
typedef QMapConstIterator<QString,QString> AttributeIterator;



class Area
{
public:
	enum ShapeType { None, Rectangle, Circle, Polygon, Default, Selection };
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
	// Only used for Polygons
	Q3PointArray *_coords;
	SelectionPointList *_selectionPoints;
	QPixmap *_highlightedPixmap;
	
	void drawHighlighting(QPainter & p);
	void drawAlt(QPainter & p);
	QString getHTMLAttributes() const;
	
public:
	Area();
	virtual ~Area();

	virtual Area* clone() const;	
	// Default implementation; is specified by subclasses
	virtual bool contains(const QPoint &) const;
	// Default implementation; is specified by subclasses
	virtual QString coordsToString() const;
	virtual void draw(QPainter &);
	
	virtual QBitmap getMask() const;	
	virtual	QString getHTMLCode() const;
	
	virtual void setHighlightedPixmap( QImage &, QBitmap &);
	
	virtual void moveBy(int, int);
	virtual void moveTo(int, int);
	virtual void moveSelectionPoint(QRect*, const QPoint &);
	
	virtual QRect* onSelectionPoint(const QPoint &,double zoom) const;
	virtual bool removeSelectionPoint(QRect * r);
  virtual SelectionPointList* selectionPoints() const { return _selectionPoints; }
	
	virtual QRect rect() const;
	
	virtual QRect selectionRect() const;
	virtual void setArea(const Area &);
	virtual bool setCoords(const QString &);
	/** finished drawing only important for polygon */
	virtual void setFinished(bool b) { _finished=b; }
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
	virtual Q3PointArray* coords() const;
	virtual void highlightSelectionPoint(int);

	virtual QString attribute(const QString &) const;
	virtual void setAttribute(const QString &, const QString &);
	virtual AttributeIterator firstAttribute() const;
	virtual AttributeIterator lastAttribute() const;
	
	QPixmap cutOut(const QImage &) ;		
	void setListViewItem(Q3ListViewItem*);
	void deleteListViewItem();
	Q3ListViewItem* listViewItem() const;
	void setName(const QString &);
	QString name() const;
	void setSelected(bool b);
	bool isSelected() const;
	bool finished() const;
	uint countSelectionPoints() const;

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
		virtual void draw(QPainter & p);
		virtual void moveSelectionPoint(QRect* selectionPoint, const QPoint & p);
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
		virtual void draw(QPainter & p);
		virtual void moveSelectionPoint(QRect* selectionPoint, const QPoint & p);
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
		virtual void draw(QPainter & p);
		virtual void moveSelectionPoint(QRect* selectionPoint, const QPoint & p);
		virtual void simplifyCoords();
  	virtual int addCoord(const QPoint & p);
		virtual bool setCoords(const QString & s);
		virtual QRect selectionRect() const;		
		virtual void setFinished(bool b);
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
		virtual void draw(QPainter & p);
		virtual QString typeString() const { return i18n("Default"); }
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
		
		uint count() const;
		
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
		virtual QRect* onSelectionPoint(const QPoint & p, double zoom) const;
		
		/**
		 * Only if one Area is selected the moveSelectionPoint method
		 * of that Area will be called
		 **/
		virtual void moveSelectionPoint(QRect* selectionPoint, const QPoint & p);				
		
    virtual SelectionPointList* selectionPoints() const;
		
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
		virtual void draw(QPainter & p);
		
	
		/**
		 * A deep copy of the Areas
		 **/
		virtual Area* clone() const;	
		
		virtual void updateSelectionPoints();			
  	virtual int addCoord(const QPoint & p);
  	virtual void insertCoord(int pos, const QPoint & p);
  	virtual void removeCoord(int pos);
    virtual bool removeSelectionPoint(QRect * r);  	
  	virtual void moveCoord(int pos,const QPoint & p);
  	virtual Q3PointArray* coords() const;
		virtual void highlightSelectionPoint(int);
		
		virtual QRect selectionRect() const;

  	virtual QString attribute(const QString & name) const;
  	virtual void setAttribute(const QString & name, const QString & value);
  	virtual AttributeIterator firstAttribute() const;
  	virtual AttributeIterator lastAttribute() const;

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


inline void AreaSelection::invalidate() {
	_selectionCacheValid=false;
	_rectCacheValid=false;
}

#endif


