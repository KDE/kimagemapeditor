/***************************************************************************
                          imagemapeditor.h  -  description
                             -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan SchÃÂ?fer
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

#ifndef KIMAGEMAPDIALOG_H
#define KIMAGEMAPDIALOG_H

#include <QDockWidget>
#include <QLinkedList>
#include <QObject>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QTextStream>

#include <kurl.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/factory.h>

#include <kdeversion.h>

#include "kimearea.h"

/**
  *@author Jan Schaefer
  */

// #define WITH_TABWIDGET


class QTreeWidget;
class QPushButton;
class DrawZone;
class QComboBox;
class QTreeWidgetItem;
class KToggleAction;
class KMainWindow;



/**
 * Stores an area tag and all its attributes
 */
typedef QHash<QString,QString> AreaTag;

/**
 * Stores an image tag and all its attributes
 * the origcode attribute hold the original htmlcode
 * of this tag
 */
typedef QHash<QString,QString> ImageTag;

/**
 * Only a small class to give a list of AreaTags a name
 */
class MapTag : public QLinkedList<AreaTag> {
public:
    MapTag();
    QString name;
    bool modified;
};


class HtmlElement {
public:
    HtmlElement(const QString & s) {
        htmlCode = s;
    };
    virtual ~HtmlElement() {}
    ;
    QString htmlCode;
};

class HtmlMapElement : public HtmlElement {
public:
    explicit HtmlMapElement(const QString & s) : HtmlElement(s) {
      mapTag = 0L;
    };

    virtual ~HtmlMapElement() {};

    MapTag* mapTag;
};

class HtmlImgElement : public HtmlElement {
public:
HtmlImgElement(const QString & s) : HtmlElement(s) {
      imgTag = 0L;
    };
    virtual ~HtmlImgElement() {}
    ;
    ImageTag* imgTag;
};

/**
 * Stores the hole HTML content in a List.
 */
typedef QList<HtmlElement*> HtmlContent;


class KSelectAction;
class KAction;
class KRecentFilesAction;
class KAction;
///class QListViewItem;
class K3CommandHistory;
class KApplication;
class QTabWidget;
class AreaListView;
class ImagesListView;
class MapsListView;
class KAboutData;

// needed by the statusbar
#define STATUS_CURSOR 1000
#define STATUS_SELECTION 1001

class KImageMapEditor : public KParts::ReadWritePart {
    Q_OBJECT
public :
    enum ToolType { Selection,
		    Rectangle,
		    Circle,
		    Polygon,
		    Freehand,
		    AddPoint,
		    RemovePoint };

    KImageMapEditor(QWidget *parentWidget,
                    QObject *parent, const QStringList & args = QStringList());
    virtual ~KImageMapEditor();

    static KAboutData *createAboutData();
    static KConfig *config();

    /**
    * Makes sure, that the actions cut, copy, delete and
    * show properties
    * can only be executed if sth. is selected.
    **/
    void updateActionAccess();

    DrawZone* getDrawZone() {
        return drawZone;
    };

    void addAreaAndEdit(Area*);
    void addArea(Area*);
    AreaListIterator areaList() const;
    KImageMapEditor::ToolType currentToolType() const;
    void deleteSelected();
    void deleteArea( Area * area);
    void deleteAllAreas();
    void deselectAll();
    void deselect(Area* s);
    void deselectWithoutUpdate(Area*);
    QString getHTMLImageMap() const;
    Area* onArea(const QPoint & p) const;
    QPixmap makeListViewPix(Area &) ;
    QString mapName() const;
    void select(Area*);
    void selectWithoutUpdate(Area*);
    void select(QTreeWidgetItem*);
    AreaSelection* selected() const;
    void setPicture(const QImage & pix);
    int showTagEditor(Area *);
    K3CommandHistory *commandHistory() const;

    KApplication* app() const;

    // Only refreshes the listView
    void updateSelection() const;

    void readConfig();
    void writeConfig();

    virtual void readProperties(const KConfigGroup &);
    virtual void saveProperties(KConfigGroup &);
    virtual bool closeUrl();
    bool queryClose();
    virtual void setReadWrite(bool);
    QString getHtmlCode();

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool);

    /**
     * Opens the given file.
     * If it's an HTML file openURL is called
     * If it's an Image, the image is added to the image list
     */
    void openFile(const KUrl &);

    /**
     * Opens the last URL the user worked with.
     * Sets also, the last map and the last image
     */
    void openLastURL(const KConfigGroup &);

    void readConfig(const KConfigGroup &);
    void writeConfig(KConfigGroup &);


protected:
    void init();
    bool openHTMLFile(const KUrl &, const QString & mapName = QString::null,
		      const QString & imagePath = QString::null);
    void saveImageMap(const KUrl &);

    /**
     * Returns a language dependent background picture, with
     * the text : Drop an image or html file
     */
    QImage getBackgroundImage();


    /**
     * Saves information to restore the last working state
     */
    void saveLastURL(KConfigGroup&);


private:
    // Stores the hole html file in a List
    // The entries are either a MapTag an ImageTag or a QString
    HtmlContent _htmlContent;

    // the url of the working image;
    KUrl _imageUrl;
    QString _mapName;
    QImage _backgroundImage;

    bool backupFileCreated;

    KImageMapEditor::ToolType _currentToolType;
    AreaList *areas;
    AreaSelection *currentSelected;
    AreaSelection *copyArea;
    Area *defaultArea;
    DrawZone* drawZone;
    QTabWidget* tabWidget;
    AreaListView *areaListView;
    ImagesListView* imagesListView;
    MapsListView* mapsListView;
    HtmlMapElement* currentMapElement;

    //
    // Actions
    //
    KSelectAction* zoomAction;
    KAction *arrowAction;
    KAction *circleAction;
    KAction *rectangleAction;
    KAction *polygonAction;
    KAction *freehandAction;
    KAction *addPointAction;
    KAction *removePointAction;

    KAction *cutAction;
    KAction *deleteAction;
    KAction *copyAction;
    KAction *pasteAction;
    KAction *zoomInAction;
    KAction *zoomOutAction;

    KAction *mapNewAction;
    KAction *mapDeleteAction;
    KAction *mapNameAction;
    KAction *mapDefaultAreaAction;

    KAction *imageAddAction;
    KAction *imageRemoveAction;
    KAction *imageUsemapAction;

    KToggleAction *highlightAreasAction;
    KToggleAction *showAltAction;

    KAction *areaPropertiesAction;

    KAction *moveLeftAction;
    KAction *moveRightAction;
    KAction *moveUpAction;
    KAction *moveDownAction;

    KAction *increaseWidthAction;
    KAction *decreaseWidthAction;
    KAction *increaseHeightAction;
    KAction *decreaseHeightAction;

    KAction *toFrontAction;
    KAction *toBackAction;
    KAction *forwardOneAction;
    KAction *backOneAction;

    KToggleAction* configureShowAreaListAction;
    KToggleAction* configureShowMapListAction;
    KToggleAction* configureShowImageListAction;


  	KRecentFilesAction* recentFilesAction;

    KMainWindow *mainWindow;
    QDockWidget* areaDock;
    QDockWidget* mapsDock;
    QDockWidget* imagesDock;

    K3CommandHistory *_commandHistory;
    int maxAreaPreviewHeight;

    QString cursorStatusText;
    QString selectionStatusText;

    void setupActions();
    void setupStatusBar();
    void updateStatusBar();
    /* refreshes all Areas, only used by preferences dialog
     * updates only the preview pictures*/
    void updateAllAreas();
    void updateUpDownBtn();

    QHash<QString,QString> getTagAttributes(QTextStream & s,QString &);

    void setMap(HtmlMapElement*);
    void setMap(MapTag*);
    void addMap(const QString &);

    // Returns the entire html file as a String
    HtmlElement* findHtmlElement(const QString &);
    HtmlImgElement* findHtmlImgElement(ImageTag*);
    HtmlMapElement* findHtmlMapElement(const QString &);
    void deleteAllMaps();
    void addImage(const KUrl &);
    void setImageActionsEnabled(bool);
    void setMapActionsEnabled(bool);

    void saveAreasToMapTag(MapTag*);
    void showPopupMenu(const QPoint &, const QString &);
    void drawToCenter(QPainter* p, const QString & str, int y, int width);

public slots:
    virtual bool openURL(const KUrl & url);
    void slotChangeStatusCoords(int x,int y);
    void slotUpdateSelectionCoords();
    void slotUpdateSelectionCoords( const QRect &);
    void slotAreaChanged(Area *);
    void slotShowMainPopupMenu(const QPoint &);
    void slotShowMapPopupMenu(const QPoint &);
    void slotShowImagePopupMenu(const QPoint &);
    void slotConfigChanged();
    void setPicture(const KUrl & url);
    void setMap(const QString &);
    void setMapName(const QString & s);


protected slots:
    // overridden from KReadWritePart
    virtual bool openFile();

    virtual bool saveFile() {
        saveImageMap( url() );
//        setModified(false);
        return true;
    }

    void fileOpen();
    void fileSaveAs();
    void fileSave();
    void fileClose();

    void slotShowPopupMenu(const QPoint &);
    void slotShowPreferences();
    void slotHighlightAreas(bool b);
    void slotShowAltTag(bool b);
    void slotSelectionChanged();

    int showTagEditor(QTreeWidgetItem *item);
    int showTagEditor();

    void slotZoom();
    void slotZoomIn();
    void slotZoomOut();

    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotDelete();

    void slotDrawArrow();
    void slotDrawCircle();
    void slotDrawRectangle();
    void slotDrawPolygon();
    void slotDrawFreehand();
    void slotDrawAddPoint();
    void slotDrawRemovePoint();

    void mapDefaultArea();
    void mapNew();
    void mapDelete();
    void mapEditName();
    void mapShowHTML();
    void mapPreview();

    void slotBackOne();
    void slotForwardOne();
    void slotToBack();
    void slotToFront();

    void slotMoveUp();
    void slotMoveDown();
    void slotMoveLeft();
    void slotMoveRight();

    void slotIncreaseHeight();
    void slotDecreaseHeight();
    void slotIncreaseWidth();
    void slotDecreaseWidth();

    void slotCancelDrawing();

    //	void slotPreferences();
    void imageAdd();
    void imageRemove();
    void imageUsemap();

};


inline KImageMapEditor::ToolType KImageMapEditor::currentToolType() const {
    return _currentToolType;
}

inline QString KImageMapEditor::mapName() const {
    return _mapName;
}

inline K3CommandHistory* KImageMapEditor::commandHistory() const {
    return _commandHistory;
}


#endif
