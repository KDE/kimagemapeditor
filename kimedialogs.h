/***************************************************************************
                          kimedialogs.h  -  description
                             -------------------
    begin                : Tue Apr 17 2001
    copyright            : (C) 2001 by Jan SchÃ¤fer
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

#ifndef KIMEDIALOGS_H
#define KIMEDIALOGS_H

#include <kdialog.h>

#include <kurl.h>
#include "kimagemapeditor.h"

#include "kdeversion.h"

#include <QLabel>
#include <Q3PtrList>

class QLineEdit;
class Q3MultiLineEdit;
class QSpinBox;


class CoordsEdit : public QWidget {
Q_OBJECT
	public :
		CoordsEdit(QWidget *parent, Area* a);
		virtual ~CoordsEdit();
		virtual void applyChanges();
	protected:
		Area *area;   // The working area
	protected slots:
		void slotTriggerUpdate();
	signals:
		void update();
};

class RectCoordsEdit : public CoordsEdit {
	public:
		RectCoordsEdit(QWidget *parent, Area* a);
		virtual void applyChanges();
	private:
		QSpinBox *topXSpin;
		QSpinBox *topYSpin;
		QSpinBox *widthSpin;
		QSpinBox *heightSpin;
};

class CircleCoordsEdit : public CoordsEdit {
	public:
		CircleCoordsEdit(QWidget *parent, Area* a);
		virtual void applyChanges();
	private:
		QSpinBox *centerXSpin;
		QSpinBox *centerYSpin;
		QSpinBox *radiusSpin;
};

class Q3Table;

class PolyCoordsEdit : public CoordsEdit {
Q_OBJECT	
	public:
		PolyCoordsEdit(QWidget *parent, Area* a);
		~PolyCoordsEdit();
		virtual void applyChanges();
	private:
		Q3Table *coordsTable;
	protected slots:
  	void slotAddPoint();
  	void slotRemovePoint();
  	void slotHighlightPoint(int);

};

class SelectionCoordsEdit : public CoordsEdit {
Q_OBJECT	
	public:
		SelectionCoordsEdit(QWidget *parent, Area* a);
		virtual void applyChanges();
	private:
		QSpinBox *topXSpin;
		QSpinBox *topYSpin;

};


class QCheckBox;
class Q3GridLayout;

class AreaDialog : public KDialog {
Q_OBJECT
	private:
		Area *area;
		Area *oldArea; // Only for drawing reasons
		Area *areaCopy; // A copy for restoring the original area if user press cancel
		QLineEdit *hrefEdit;
		QLineEdit *altEdit;
		QLineEdit *targetEdit;
		QLineEdit *titleEdit;
		
		QLineEdit *onClickEdit;
		QLineEdit *onDblClickEdit;
		QLineEdit *onMouseDownEdit;
		QLineEdit *onMouseUpEdit;
		QLineEdit *onMouseOverEdit;
		QLineEdit *onMouseMoveEdit;
		QLineEdit *onMouseOutEdit;
		
		CoordsEdit *coordsEdit;
		CoordsEdit* createCoordsEdit(QWidget *parent, Area *a);
		QCheckBox *defaultAreaChk;
		KImageMapEditor *_document;
		

	public:
		AreaDialog(KImageMapEditor* parent,Area * a);
		~AreaDialog();
	protected slots:
		virtual void slotOk();
		virtual void slotApply();
		virtual void slotCancel();
		void slotChooseHref();
		void slotUpdateArea();
		
    QLineEdit* createLineEdit(QWidget* parent, QGridLayout *layout, int y, const QString & value, const QString & name);
	  QWidget* createGeneralPage();
	  QWidget* createCoordsPage();
	  QWidget* createJavascriptPage();
	signals:
		void areaChanged(Area* a);
};

class QLineEdit;
class Q3ListBox;
class QLabel;


class ImageMapChooseDialog : public KDialog {
Q_OBJECT
	private:
		Q3Table *imageListTable;
		QLabel *imagePreview;		
		Q3ListBox *mapListBox;	
		QLineEdit *mapNameEdit;
		Q3PtrList<MapTag> *maps;
		Q3PtrList<ImageTag> *images;
		KUrl baseUrl;
    void initImageListTable(QWidget*);    
	public:
		ImageMapChooseDialog(QWidget* parent,Q3PtrList<MapTag> *_maps,Q3PtrList<ImageTag> *_images, const KUrl & _baseUrl);
		~ImageMapChooseDialog();
		KUrl pixUrl;
		MapTag* currentMap;
	protected slots:
		void slotImageChanged();
		void slotMapChanged(int i);
        void selectImageWithUsemap(const QString & usemap);
      
};

class KConfig;

class PreferencesDialog : public KDialog {
Q_OBJECT
	public:
		PreferencesDialog(QWidget *parent,KConfig*);
		~PreferencesDialog();
	protected slots:
	  virtual void slotDefault( void );
  	virtual void slotOk( void );
  	virtual void slotApply( void );
	private:
		QSpinBox *rowHeightSpinBox;
		QSpinBox *undoSpinBox;
		QSpinBox *redoSpinBox;
//		QCheckBox *colorizeAreaChk;
//		QCheckBox *showAltChk;
    QCheckBox *startWithCheck;
    KConfig *config;
};

class KHTMLPart;
class KTemporaryFile;

class HTMLPreviewDialog : public KDialog{
  public:
    HTMLPreviewDialog(QWidget *, const KUrl &, const QString &);
    ~HTMLPreviewDialog();
    virtual void show();
  private:
    KHTMLPart* htmlPart;
    KTemporaryFile* tempFile;
};

#endif
