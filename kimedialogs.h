/***************************************************************************
                          kimedialogs.h  -  description
                             -------------------
    begin                : Tue Apr 17 2001
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

#ifndef KIMEDIALOGS_H
#define KIMEDIALOGS_H

#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLinkedList>
#include <QWebEngineView>

#include "kimagemapeditor.h"

class QLineEdit;
class QSpinBox;


class CoordsEdit : public QWidget {
Q_OBJECT
	public :
		CoordsEdit(QWidget *parent, Area* a);
		~CoordsEdit() override;
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
		void applyChanges() override;
	private:
		QSpinBox *topXSpin;
		QSpinBox *topYSpin;
		QSpinBox *widthSpin;
		QSpinBox *heightSpin;
};

class CircleCoordsEdit : public CoordsEdit {
	public:
		CircleCoordsEdit(QWidget *parent, Area* a);
		void applyChanges() override;
	private:
		QSpinBox *centerXSpin;
		QSpinBox *centerYSpin;
		QSpinBox *radiusSpin;
};

class QTableWidget;

class PolyCoordsEdit : public CoordsEdit {
Q_OBJECT	
	public:
		PolyCoordsEdit(QWidget *parent, Area* a);
		~PolyCoordsEdit() override;
		void applyChanges() override;
	private:
		QTableWidget *coordsTable;
		void updatePoints();
	protected slots:
  	void slotAddPoint();
  	void slotRemovePoint();
  	void slotHighlightPoint(int);
	
};

class SelectionCoordsEdit : public CoordsEdit {
Q_OBJECT	
	public:
		SelectionCoordsEdit(QWidget *parent, Area* a);
		void applyChanges() override;
	private:
		QSpinBox *topXSpin;
		QSpinBox *topYSpin;

};


class QCheckBox;

class AreaDialog : public QDialog {
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
		~AreaDialog() override;
	protected slots:
		virtual void slotOk();
		virtual void slotApply();
		virtual void slotCancel();
		void slotChooseHref();
		void slotUpdateArea();
		
		QLineEdit* createLineEdit(QFormLayout *layout, const QString &value, const QString &name);
		QWidget* createGeneralPage();
		QWidget* createCoordsPage();
		QWidget* createJavascriptPage();
	signals:
		void areaChanged(Area* a);
};




class KConfig;

class PreferencesDialog : public QDialog {
Q_OBJECT
	public:
		PreferencesDialog(QWidget *parent,KConfig*);
		~PreferencesDialog() override;
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
 signals:
    void preferencesChanged();
};

class KHTMLPart;
class QTemporaryFile;

class HTMLPreviewDialog : public QDialog{
  public:
    HTMLPreviewDialog(QWidget *, const QString &);
    ~HTMLPreviewDialog() override;
  private:
    QWebEngineView* htmlPart;
    QTemporaryFile* tempFile;
};

#endif
