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

#include <QHash>
#include <QHashIterator>
#include <QLinkedList>
#include <QList>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QTreeWidgetItem>

#include <klocalizedstring.h>

class QPainter;
class QPolygon;
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


  SelectionPoint(QPoint,QCursor);
  virtual ~SelectionPoint();

  void setState(SelectionPoint::State s);
  State getState() const;

  QPoint getPoint() const;
  void setPoint(QPoint);
  void translate(int,int);

  QRect getRect() const;

  void draw(QPainter*, double);

  QCursor cursor();
  void setCursor(QCursor);

private:
  QPoint point;
  State state;
  QCursor _cursor;
  
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
	QTreeWidgetItem* _listViewItem;

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
	void setListViewItem(QTreeWidgetItem*);
	void deleteListViewItem();
	QTreeWidgetItem* listViewItem() const;
	void setName(const QString &);
	QString name() const;
	void setSelected(bool b);
	bool isSelected() const;
	bool finished() const;
	int countSelectionPoints() const;

};



inline QTreeWidgetItem* Area::listViewItem() const {
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
		~RectArea() override;

		Area* clone() const override;
		bool contains(const QPoint & p) const override;
		QString coordsToString() const override;
		void draw(QPainter*) override;
		void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p) override;
		bool setCoords(const QString & s) override;
		QString typeString() const override { return i18n("Rectangle"); }
		QBitmap getMask() const override;
			QString getHTMLCode() const override;
		void updateSelectionPoints() override;
};


/**
 *	Represents a Circle Area
 **/
class CircleArea : public Area
{
	public:
		CircleArea();
		~CircleArea() override;

		Area* clone() const override;
		bool contains(const QPoint & p) const override;
		QString coordsToString() const override;
		void draw(QPainter*) override;
		void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p) override;
		bool setCoords(const QString & s) override;
		void setRect(const QRect & r) override;
		QString typeString() const override { return i18n("Circle"); }
		QBitmap getMask() const override;
		QString getHTMLCode() const override;
		void updateSelectionPoints() override;

};

/**
 *	Represents a Rectangle Area
 **/
class PolyArea :public Area
{
	public:
		PolyArea();
		~PolyArea() override;

		Area* clone() const override;
		bool contains(const QPoint & p) const override;
		QString coordsToString() const override;
		void draw(QPainter*) override;
		void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p) override;
		void simplifyCoords() override;
  	int addCoord(const QPoint & p) override;
		bool setCoords(const QString & s) override;
		QRect selectionRect() const override;
		void setFinished(bool b, bool removeLast) override;
		QString typeString() const override { return i18n("Polygon"); }
		QBitmap getMask() const override;
		QString getHTMLCode() const override;
		void updateSelectionPoints() override;

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
  ~DefaultArea() override;

  Area* clone() const override;
  // the default area isn't drawn
  void draw(QPainter*) override;
  QString typeString() const override { 
    return i18n("Default"); 
  }
  QString getHTMLCode() const override;

};


typedef QList<Area*> AreaList;
typedef QListIterator<Area*> AreaListIterator;

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
  ~AreaSelection() override;

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
   * Overridden Methods of the Area class
   */
  bool contains(const QPoint & p) const override;

  /**
   *
   **/
  SelectionPoint* onSelectionPoint(const QPoint & p, double zoom) const override;

  /**
   * Only if one Area is selected the moveSelectionPoint method
   * of that Area will be called
   **/
  void moveSelectionPoint(SelectionPoint* selectionPoint, const QPoint & p) override;

  const SelectionPointList & selectionPoints() const override;

  /**
   * All Areas will be moved by dx and dy
   **/
  void moveBy(int dx, int dy) override;

  /**
   * Calls for every selected Area the setArea with the
   * corresponding Area in the copy Selection.
   * IMPORTANT : works only if the copy Area is an AreaSelection
   * and have the same number of Areas
   **/
  void setArea(const Area & copy) override;
  virtual void setAreaSelection(const AreaSelection & copy);

  /**
   * If only one Area is selected the setRect method of that Area
   * will be called
   **/
  void setRect(const QRect & r) override;
  QRect rect() const override;


  QString typeString() const override;
  ShapeType type() const override;

  // The selection is only a container
  // so it is never drawn
  void draw(QPainter*) override;


  /**
   * A deep copy of the Areas
   **/
  Area* clone() const override;

  void resetSelectionPointState() override;
  void setSelectionPointStates(SelectionPoint::State st) override;
  void updateSelectionPointStates();

  void updateSelectionPoints() override;
  int addCoord(const QPoint & p) override;
  void insertCoord(int pos, const QPoint & p) override;
  void removeCoord(int pos) override;
  bool removeSelectionPoint(SelectionPoint *) override;
  void moveCoord(int pos,const QPoint & p) override;
  QPolygon coords() const override;
  void highlightSelectionPoint(int) override;

  QRect selectionRect() const override;

  QString attribute(const QString & name) const override;
  void setAttribute(const QString & name, const QString & value) override;
  AttributeIterator attributeIterator() const override;

  void setMoving(bool b) override;
  bool isMoving() const override;


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


