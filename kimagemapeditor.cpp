/***************************************************************************
                          imagemapeditor.cpp  -  description
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

#include "kimagemapeditor.h"

#include <iostream>
#include <assert.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QInputDialog>
#include <QLayout>
#include <QLinkedList>
#include <QListWidget>
#include <QMenu>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QToolTip>
#include <QUndoStack>
#include <QVBoxLayout>

// KDE Frameworks
#include <KPluginMetaData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/Job>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KUndoActions>
#include <KXMLGUIFactory>
#include <KXmlGuiWindow>

// local
#include "kimagemapeditor_debug.h"
#include "drawzone.h"
#include "kimedialogs.h"
#include "kimecommands.h"
#include "areacreator.h"
#include "arealistview.h"
#include "imageslistview.h"
#include "mapslistview.h"
#include "kimecommon.h"
#include "imagemapchoosedialog.h"
#include "kimagemapeditor_version.h"

K_PLUGIN_FACTORY_WITH_JSON(KImageMapEditorFactory, "kimagemapeditorpart.json", registerPlugin<KImageMapEditor>();)

KImageMapEditor::KImageMapEditor(QWidget *parentWidget, QObject *parent,
                                 const KPluginMetaData &metaData,
                                 const QVariantList & )
  : KParts::ReadWritePart(parent)
{
  setMetaData(metaData);

//  KDockMainWindow* mainWidget;

  mainWindow = dynamic_cast<KXmlGuiWindow*>(parent) ;
  QSplitter * splitter = nullptr;
  tabWidget = nullptr;

  if (mainWindow) {
//    qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor: We got a KDockMainWindow !";

//    K3DockWidget* parentDock = mainDock->getMainDockWidget();
    areaDock = new QDockWidget(i18n("Areas"),mainWindow);
    mapsDock = new QDockWidget(i18n("Maps"),mainWindow);
    imagesDock = new QDockWidget(i18n("Images"),mainWindow);

    // Needed to save their state
    areaDock->setObjectName("areaDock");
    mapsDock->setObjectName("mapsDock");
    imagesDock->setObjectName("imagesDock");

    mainWindow->addDockWidget( Qt::LeftDockWidgetArea, areaDock);
    mainWindow->addDockWidget( Qt::LeftDockWidgetArea, mapsDock);
    mainWindow->addDockWidget( Qt::LeftDockWidgetArea, imagesDock);

    areaListView = new AreaListView(areaDock);
    mapsListView = new MapsListView(mapsDock);
    imagesListView = new ImagesListView(imagesDock);

    areaDock->setWidget(areaListView);
    mapsDock->setWidget(mapsListView);
    imagesDock->setWidget(imagesListView);

  }
  else
  {
    areaDock = nullptr;
    mapsDock = nullptr;
    imagesDock = nullptr;
    splitter = new QSplitter(parentWidget);
    tabWidget = new QTabWidget(splitter);
    areaListView = new AreaListView(tabWidget);
    mapsListView = new MapsListView(tabWidget);
    imagesListView = new ImagesListView(tabWidget);

    tabWidget->addTab(areaListView,i18n("Areas"));
    tabWidget->addTab(mapsListView,i18n("Maps"));
    tabWidget->addTab(imagesListView,i18n("Images"));
  }


  connect( areaListView->listView, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
  connect( areaListView->listView,
           SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           this,
           SLOT(showTagEditor(QTreeWidgetItem*)));
  connect( areaListView->listView,
           SIGNAL(customContextMenuRequested(QPoint)),
           this,
           SLOT(slotShowPopupMenu(QPoint)));

  connect( mapsListView, SIGNAL(mapSelected(QString)),
           this, SLOT(setMap(QString)));

  connect( mapsListView, SIGNAL(mapRenamed(QString)),
           this, SLOT(setMapName(QString)));

  connect( mapsListView->listView(),
           SIGNAL(customContextMenuRequested(QPoint)),
           this,
           SLOT(slotShowMapPopupMenu(QPoint)));

  connect( imagesListView, &ImagesListView::imageSelected,
           this, QOverload<const QUrl &>::of(&KImageMapEditor::setPicture));

  connect( imagesListView,
           SIGNAL(customContextMenuRequested(QPoint)),
           this,
           SLOT(slotShowImagePopupMenu(QPoint)));

  if (splitter) {
    drawZone = new DrawZone(splitter,this);
    splitter->setStretchFactor(splitter->indexOf(tabWidget), 0);
    splitter->setStretchFactor(splitter->indexOf(drawZone), 1);
    setWidget(splitter);
  } else {
    QScrollArea *sa = new QScrollArea(mainWindow);
    drawZone = new DrawZone(nullptr,this);
    mainWindow->setCentralWidget(sa);
    sa->setWidget(drawZone);
    setWidget(mainWindow);
    //    sa->setWidgetResizable(true);
  }


  areas = new AreaList();
  currentSelected= new AreaSelection();
  _currentToolType=KImageMapEditor::Selection;
  copyArea = nullptr;
  defaultArea = nullptr;
  currentMapElement = nullptr;

  setupActions();
  setupStatusBar();

  setXMLFile("kimagemapeditorpartui.rc");

  setPicture(getBackgroundImage());

  init();
  readConfig();
}

KImageMapEditor::~KImageMapEditor() {
  writeConfig();

  delete areas;

  delete currentSelected;
  delete copyArea;
  delete defaultArea;

  // Delete our DockWidgets
  if (areaDock) {
    areaDock->hide();
    mapsDock->hide();
    imagesDock->hide();

    delete areaDock;
    delete mapsDock;
    delete imagesDock;
  }

}

QString KImageMapEditor::componentName() const
{
    // the part ui.rc file is in the program folder, not a separate one
    // TODO: change the component name to "kimagemapeditorpart" by removing this method and
    // adapting the folder where the file is placed.
    // Needs a way to also move any potential custom user ui.rc files
    // from kimagemapeditor/ to kimagemapeditorpart/
    return QStringLiteral("kimagemapeditor");
}

MapTag::MapTag() {
  modified = false;
  name.clear();
}

void KImageMapEditor::init()
{
  _htmlContent.clear();
  _imageUrl.clear();
  //  closeUrl();
  HtmlElement* el = new HtmlElement("<html>\n");
  _htmlContent.append(el);
  el = new HtmlElement("<head>\n");
  _htmlContent.append(el);
  el = new HtmlElement("</head>\n");
  _htmlContent.append(el);
  el = new HtmlElement("<body>\n");
  _htmlContent.append(el);

  addMap(i18n("unnamed"));

  el = new HtmlElement("</body>\n");
  _htmlContent.append(el);
  el = new HtmlElement("</html>\n");
  _htmlContent.append(el);

  setImageActionsEnabled(false);
}

void KImageMapEditor::setReadWrite(bool)
{

  // For now it does not matter if it is readwrite or readonly
  // it is always readwrite, because Quanta only supports ReadOnlyParts
  // at this moment and in that case it should be readwrite, too.
  ReadWritePart::setReadWrite(true);
  /*
    if (rw)
      ;
    else
    {
     actionCollection()->remove(arrowAction);
     actionCollection()->remove(circleAction);
     actionCollection()->remove(rectangleAction);
     actionCollection()->remove(polygonAction);
     actionCollection()->remove(freehandAction);
     actionCollection()->remove(addPointAction);
     actionCollection()->remove(removePointAction);

     actionCollection()->remove(cutAction);
     actionCollection()->remove(deleteAction);
     actionCollection()->remove(copyAction);
     actionCollection()->remove(pasteAction);

     actionCollection()->remove(mapNewAction);
     actionCollection()->remove(mapDeleteAction);
     actionCollection()->remove(mapNameAction);
     actionCollection()->remove(mapDefaultAreaAction);

     actionCollection()->remove(areaPropertiesAction);

     actionCollection()->remove(moveLeftAction);
     actionCollection()->remove(moveRightAction);
     actionCollection()->remove(moveUpAction);
     actionCollection()->remove(moveDownAction);

     actionCollection()->remove(increaseWidthAction);
     actionCollection()->remove(decreaseWidthAction);
     actionCollection()->remove(increaseHeightAction);
     actionCollection()->remove(decreaseHeightAction);

     actionCollection()->remove(toFrontAction);
     actionCollection()->remove(toBackAction);
     actionCollection()->remove(forwardOneAction);
     actionCollection()->remove(backOneAction);

     actionCollection()->remove(imageRemoveAction);
     actionCollection()->remove(imageAddAction);
     actionCollection()->remove(imageUsemapAction);

    }
  */

}

void KImageMapEditor::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    QAction *save = actionCollection()->action(KStandardAction::name(KStandardAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}


KConfig *KImageMapEditor::config()
{
    /* TODO KF5
    KSharedConfigPtr tmp = KimeFactory::componentData().config();
    return tmp.data();
    */
    return new KConfig();
}

void KImageMapEditor::readConfig(const KConfigGroup &config) {
  KConfigGroup data = config.parent().group( "Data" );
  recentFilesAction->loadEntries( data );
}

void KImageMapEditor::writeConfig(KConfigGroup& config) {
  config.writeEntry("highlightareas",highlightAreasAction->isChecked());
  config.writeEntry("showalt",showAltAction->isChecked());
  KConfigGroup data = config.parent().group( "Data" );
  recentFilesAction->saveEntries( data );
  saveLastURL(config);

}

void KImageMapEditor::readConfig() {
  readConfig(config()->group("General Options" ) );
  slotConfigChanged();
}

void KImageMapEditor::writeConfig() {
  KConfigGroup cg( config(), "General Options");
  writeConfig( cg );
  config()->sync();
}


void KImageMapEditor::saveProperties(KConfigGroup &config)
{
  saveLastURL(config);
}

void KImageMapEditor::readProperties(const KConfigGroup& config)
{
  openLastURL(config);
}

void KImageMapEditor::slotConfigChanged()
{
  KConfigGroup group = config()->group("Appearance");
  int newHeight=group.readEntry("maximum-preview-height",50);
  group = config()->group("General Options");
  _commandHistory->setUndoLimit(group.readEntry("undo-level",100));
#if 0
  _commandHistory->setRedoLimit(group.readEntry("redo-level",100));
#endif
  Area::highlightArea = group.readEntry("highlightareas",true);
  highlightAreasAction->setChecked(Area::highlightArea);
  Area::showAlt = group.readEntry("showalt",true);
  showAltAction->setChecked(Area::showAlt);

  // if the image preview size changed update all images
  if (maxAreaPreviewHeight!=newHeight) {
    maxAreaPreviewHeight=newHeight;
    areaListView->listView->setIconSize(QSize(newHeight,newHeight));
  }

  updateAllAreas();
  drawZone->repaint();
}

void KImageMapEditor::openLastURL(const KConfigGroup & config) {
  QUrl lastURL ( config.readPathEntry("lastopenurl", QString()) );
  QString lastMap = config.readEntry("lastactivemap");
  QString lastImage = config.readPathEntry("lastactiveimage", QString());


//  qCDebug(KIMAGEMAPEDITOR_LOG) << "loading from group : " << config.group();

//  qCDebug(KIMAGEMAPEDITOR_LOG) << "loading entry lastopenurl : " << lastURL.path();
//  KMessageBox::information(0L, config.group()+" "+lastURL.path());
  if (!lastURL.isEmpty()) {
    openUrl(lastURL);
    if (!lastMap.isEmpty())
      mapsListView->selectMap(lastMap);
    if (!lastImage.isEmpty())
      setPicture(QUrl::fromLocalFile(lastImage));
//    qCDebug(KIMAGEMAPEDITOR_LOG) << "opening HTML file with map " << lastMap << " and image " << lastImage;
//    if (! openHTMLFile(lastURL, lastMap, lastImage) )
//      closeUrl();
      //openUrl(lastURL);
      //    else
      //closeUrl();
  }
}

void KImageMapEditor::saveLastURL(KConfigGroup & config) {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "saveLastURL: " << url().path();
  config.writePathEntry("lastopenurl",url().path());
  config.writeEntry("lastactivemap",mapName());
  config.writePathEntry("lastactiveimage",_imageUrl.path());
//  qCDebug(KIMAGEMAPEDITOR_LOG) << "writing entry lastopenurl : " << url().path();
//  qCDebug(KIMAGEMAPEDITOR_LOG) << "writing entry lastactivemap : " << mapName();
//  qCDebug(KIMAGEMAPEDITOR_LOG) << "writing entry lastactiveimage : " << _imageUrl.path();
  //KMessageBox::information(0L, QString("Group: %1 Saving ... %2").arg(config.group()).arg(url().path()));
}

void KImageMapEditor::setupActions()
{
	// File Open
  QAction *temp =
    KStandardAction::open(this, SLOT(fileOpen()),
			  actionCollection());
  temp->setWhatsThis(i18n("<h3>Open File</h3>Click this to <em>open</em> a new picture or HTML file."));
  temp->setToolTip(i18n("Open new picture or HTML file"));

  // File Open Recent
  recentFilesAction = KStandardAction::openRecent(this, SLOT(openURL(QUrl)),
                                      actionCollection());
	// File Save
  temp =KStandardAction::save(this, SLOT(fileSave()), actionCollection());
	temp->setWhatsThis(i18n("<h3>Save File</h3>Click this to <em>save</em> the changes to the HTML file."));
	temp->setToolTip(i18n("Save HTML file"));


	// File Save As
  (void)KStandardAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());

	// File Close
  temp=KStandardAction::close(this, SLOT(fileClose()), actionCollection());
	temp->setWhatsThis(i18n("<h3>Close File</h3>Click this to <em>close</em> the currently open HTML file."));
	temp->setToolTip(i18n("Close HTML file"));

  // Edit Copy
  copyAction=KStandardAction::copy(this, SLOT(slotCopy()), actionCollection());
  copyAction->setWhatsThis(i18n("<h3>Copy</h3>"
                          "Click this to <em>copy</em> the selected area."));
  copyAction->setEnabled(false);

  // Edit Cut
  cutAction=KStandardAction::cut(this, SLOT(slotCut()), actionCollection());
  cutAction->setWhatsThis(i18n("<h3>Cut</h3>"
                          "Click this to <em>cut</em> the selected area."));
  cutAction->setEnabled(false);

  // Edit Paste
  pasteAction=KStandardAction::paste(this, SLOT(slotPaste()), actionCollection());
  pasteAction->setWhatsThis(i18n("<h3>Paste</h3>"
                          "Click this to <em>paste</em> the copied area."));
  pasteAction->setEnabled(false);


  // Edit Delete
  deleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
      i18n("&Delete"), this);
  actionCollection()->addAction("edit_delete", deleteAction );
  connect(deleteAction, SIGNAL(triggered(bool)), SLOT (slotDelete()));
  actionCollection()->setDefaultShortcut(deleteAction, QKeySequence(Qt::Key_Delete));
  deleteAction->setWhatsThis(i18n("<h3>Delete</h3>"
                          "Click this to <em>delete</em> the selected area."));
  deleteAction->setEnabled(false);

  // Edit Undo/Redo
  _commandHistory = new QUndoStack(this);
  KUndoActions::createUndoAction(_commandHistory, actionCollection());
  KUndoActions::createRedoAction(_commandHistory, actionCollection());

  // Edit Properties
    areaPropertiesAction  = new QAction(i18n("Pr&operties"), this);
    actionCollection()->addAction("edit_properties", areaPropertiesAction );
  connect(areaPropertiesAction, SIGNAL(triggered(bool)), SLOT(showTagEditor()));
  areaPropertiesAction->setEnabled(false);

  // View Zoom In
  zoomInAction=KStandardAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection());
  // View Zoom Out
  zoomOutAction=KStandardAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection());

  // View Zoom
  zoomAction  = new KSelectAction(i18n("Zoom"), this);
  actionCollection()->addAction("view_zoom", zoomAction );
  connect(zoomAction, &KSelectAction::indexTriggered, this, &KImageMapEditor::slotZoom);
  zoomAction->setWhatsThis(i18n("<h3>Zoom</h3>"
                          "Choose the desired zoom level."));
  zoomAction->setItems(QStringList()
    << i18n("25%")
    << i18n("50%")
    << i18n("100%")
    << i18n("150%")
    << i18n("200%")
    << i18n("250%")
    << i18n("300%")
    << i18n("500%")
    << i18n("750%")
    << i18n("1000%"));

  zoomAction->setCurrentItem(2);

  highlightAreasAction = actionCollection()->add<KToggleAction>("view_highlightareas");
  highlightAreasAction->setText(i18n("Highlight Areas"));

  connect(highlightAreasAction, SIGNAL(toggled(bool)),
	  this, SLOT(slotHighlightAreas(bool)));

  showAltAction =   actionCollection()->add<KToggleAction>("view_showalt");
  showAltAction->setText(i18n("Show Alt Tag"));
  connect(showAltAction, SIGNAL(toggled(bool)),this, SLOT (slotShowAltTag(bool)));

    mapNameAction  = new QAction(i18n("Map &Name..."), this);
    actionCollection()->addAction("map_name", mapNameAction );
  connect(mapNameAction, SIGNAL(triggered(bool)), SLOT(mapEditName()));

    mapNewAction  = new QAction(i18n("Ne&w Map..."), this);
    actionCollection()->addAction("map_new", mapNewAction );
  connect(mapNewAction, SIGNAL(triggered(bool)), SLOT(mapNew()));
  mapNewAction->setToolTip(i18n("Create a new map"));

    mapDeleteAction  = new QAction(i18n("D&elete Map"), this);
    actionCollection()->addAction("map_delete", mapDeleteAction );
  connect(mapDeleteAction, SIGNAL(triggered(bool)), SLOT(mapDelete()));
  mapDeleteAction->setToolTip(i18n("Delete the current active map"));

    mapDefaultAreaAction  = new QAction(i18n("Edit &Default Area..."), this);
    actionCollection()->addAction("map_defaultarea", mapDefaultAreaAction );
  connect(mapDefaultAreaAction, SIGNAL(triggered(bool)), SLOT(mapDefaultArea()));
  mapDefaultAreaAction->setToolTip(i18n("Edit the default area of the current active map"));

    temp  = new QAction(i18n("&Preview"), this);
    actionCollection()->addAction("map_preview", temp );
  connect(temp, SIGNAL(triggered(bool)), SLOT(mapPreview()));
  temp->setToolTip(i18n("Show a preview"));

  // IMAGE
  i18n("&Image");

  imageAddAction  = new QAction(i18n("Add Image..."), this);
  actionCollection()->addAction("image_add", imageAddAction );
  connect(imageAddAction, SIGNAL(triggered(bool)), SLOT(imageAdd()));
  imageAddAction->setToolTip(i18n("Add a new image"));

    imageRemoveAction  = new QAction(i18n("Remove Image"), this);
    actionCollection()->addAction("image_remove", imageRemoveAction );
  connect(imageRemoveAction, SIGNAL(triggered(bool)), SLOT(imageRemove()));
  imageRemoveAction->setToolTip(i18n("Remove the current visible image"));

    imageUsemapAction  = new QAction(i18n("Edit Usemap..."), this);
    actionCollection()->addAction("image_usemap", imageUsemapAction );
  connect(imageUsemapAction, SIGNAL(triggered(bool)), SLOT(imageUsemap()));
  imageUsemapAction->setToolTip(i18n("Edit the usemap tag of the current visible image"));

    temp  = new QAction(i18n("Show &HTML"), this);
    actionCollection()->addAction("map_showhtml", temp );
  connect(temp, SIGNAL(triggered(bool)), SLOT(mapShowHTML()));


  QActionGroup *drawingGroup = new QActionGroup(this);
  // Selection Tool
  arrowAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("arrow")), i18n("&Selection"), this);
  actionCollection()->setDefaultShortcut(arrowAction, QKeySequence("s"));
  actionCollection()->addAction("tool_arrow", arrowAction);
  connect(arrowAction, SIGNAL(triggered(bool)), SLOT (slotDrawArrow()));
  arrowAction->setWhatsThis(i18n("<h3>Selection</h3>"
                          "Click this to select areas."));
  drawingGroup->addAction(arrowAction);
  arrowAction->setChecked(true);

  // Circle
  circleAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("circle")), i18n("&Circle"), this);
  actionCollection()->setDefaultShortcut(circleAction, QKeySequence("c"));

  actionCollection()->addAction("tool_circle", circleAction);
  connect(circleAction, SIGNAL(triggered(bool)), this, SLOT(slotDrawCircle()));
  circleAction->setWhatsThis(i18n("<h3>Circle</h3>"
                          "Click this to start drawing a circle."));
  drawingGroup->addAction(circleAction);

  // Rectangle
    rectangleAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("rectangle")), i18n("&Rectangle"), this);
  actionCollection()->setDefaultShortcut(rectangleAction, QKeySequence("r"));
    actionCollection()->addAction("tool_rectangle", rectangleAction);
  connect(rectangleAction, SIGNAL(triggered(bool)), this, SLOT(slotDrawRectangle()));
  rectangleAction->setWhatsThis(i18n("<h3>Rectangle</h3>"
                          "Click this to start drawing a rectangle."));
  drawingGroup->addAction(rectangleAction);

  // Polygon
    polygonAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("polygon")), i18n("&Polygon"), this);
  actionCollection()->setDefaultShortcut(polygonAction, QKeySequence("p"));
    actionCollection()->addAction("tool_polygon", polygonAction);
  connect(polygonAction, SIGNAL(triggered(bool)), SLOT(slotDrawPolygon()));
  polygonAction->setWhatsThis(i18n("<h3>Polygon</h3>"
                          "Click this to start drawing a polygon."));
  drawingGroup->addAction(polygonAction);

  // Freehand
    freehandAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("freehand")), i18n("&Freehand Polygon"), this);
  actionCollection()->setDefaultShortcut(freehandAction, QKeySequence("f"));
    actionCollection()->addAction("tool_freehand", freehandAction);
  connect(freehandAction, SIGNAL(triggered(bool)), SLOT(slotDrawFreehand()));
  freehandAction->setWhatsThis(i18n("<h3>Freehandpolygon</h3>"
                          "Click this to start drawing a freehand polygon."));
  drawingGroup->addAction(freehandAction);

  // Add Point
    addPointAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("addpoint")), i18n("&Add Point"), this);
  actionCollection()->setDefaultShortcut(addPointAction, QKeySequence("a"));
    actionCollection()->addAction("tool_addpoint", addPointAction);
  connect(addPointAction, SIGNAL(triggered(bool)), SLOT(slotDrawAddPoint()));
  addPointAction->setWhatsThis(i18n("<h3>Add Point</h3>"
                          "Click this to add points to a polygon."));
  drawingGroup->addAction(addPointAction);

  // Remove Point
  removePointAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("removepoint")), i18n("&Remove Point"), this);
  actionCollection()->setDefaultShortcut(removePointAction, QKeySequence("e"));
  actionCollection()->addAction("tool_removepoint", removePointAction);
  connect(removePointAction, SIGNAL(triggered(bool)), 
          SLOT(slotDrawRemovePoint()));
  removePointAction->setWhatsThis(i18n("<h3>Remove Point</h3>"
                          "Click this to remove points from a polygon."));
  drawingGroup->addAction(removePointAction);

    QAction *action  = new QAction(i18n("Cancel Drawing"), this);
    actionCollection()->addAction("canceldrawing", action );
  connect(action, SIGNAL(triggered(bool)), SLOT(slotCancelDrawing()));
  actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::Key_Escape));

  moveLeftAction  = new QAction(i18n("Move Left"), this);
  actionCollection()->addAction("moveleft", moveLeftAction );
  connect(moveLeftAction, SIGNAL(triggered(bool)),
         SLOT(slotMoveLeft()));
  actionCollection()->setDefaultShortcut(moveLeftAction, QKeySequence(Qt::Key_Left));

    moveRightAction  = new QAction(i18n("Move Right"), this);
    actionCollection()->addAction("moveright", moveRightAction );
  connect(moveRightAction, SIGNAL(triggered(bool)), SLOT(slotMoveRight()));
  actionCollection()->setDefaultShortcut(moveRightAction, QKeySequence(Qt::Key_Right));

    moveUpAction  = new QAction(i18n("Move Up"), this);
    actionCollection()->addAction("moveup", moveUpAction );
  connect(moveUpAction, SIGNAL(triggered(bool)), SLOT(slotMoveUp()));
  actionCollection()->setDefaultShortcut(moveUpAction, QKeySequence(Qt::Key_Up));

    moveDownAction  = new QAction(i18n("Move Down"), this);
    actionCollection()->addAction("movedown", moveDownAction );
  connect(moveDownAction, SIGNAL(triggered(bool)), SLOT(slotMoveDown()));
  actionCollection()->setDefaultShortcut(moveDownAction, QKeySequence(Qt::Key_Down));

    increaseWidthAction  = new QAction(i18n("Increase Width"), this);
    actionCollection()->addAction("increasewidth", increaseWidthAction );
  connect(increaseWidthAction, SIGNAL(triggered(bool)), SLOT(slotIncreaseWidth()));
  actionCollection()->setDefaultShortcut(increaseWidthAction, QKeySequence(Qt::Key_Right + Qt::SHIFT));

    decreaseWidthAction  = new QAction(i18n("Decrease Width"), this);
    actionCollection()->addAction("decreasewidth", decreaseWidthAction );
  connect(decreaseWidthAction, SIGNAL(triggered(bool)), SLOT(slotDecreaseWidth()));
  actionCollection()->setDefaultShortcut(decreaseWidthAction, QKeySequence(Qt::Key_Left + Qt::SHIFT));

    increaseHeightAction  = new QAction(i18n("Increase Height"), this);
    actionCollection()->addAction("increaseheight", increaseHeightAction );
  connect(increaseHeightAction, SIGNAL(triggered(bool)), SLOT(slotIncreaseHeight()));
  actionCollection()->setDefaultShortcut(increaseHeightAction, QKeySequence(Qt::Key_Up + Qt::SHIFT));

    decreaseHeightAction  = new QAction(i18n("Decrease Height"), this);
    actionCollection()->addAction("decreaseheight", decreaseHeightAction );
  connect(decreaseHeightAction, SIGNAL(triggered(bool)), SLOT(slotDecreaseHeight()));
  actionCollection()->setDefaultShortcut(decreaseHeightAction, QKeySequence(Qt::Key_Down + Qt::SHIFT));

    toFrontAction  = new QAction(i18n("Bring to Front"), this);
    actionCollection()->addAction("tofront", toFrontAction );
  connect(toFrontAction, SIGNAL(triggered(bool)), SLOT(slotToFront()));

    toBackAction  = new QAction(i18n("Send to Back"), this);
    actionCollection()->addAction("toback", toBackAction );
  connect(toBackAction, SIGNAL(triggered(bool)), SLOT(slotToBack()));

    forwardOneAction  = new QAction(QIcon::fromTheme(QStringLiteral("raise")), i18n("Bring Forward One"), this);
    actionCollection()->addAction("forwardone", forwardOneAction );
  connect(forwardOneAction, SIGNAL(triggered(bool)), SLOT(slotForwardOne()));
    backOneAction  = new QAction(QIcon::fromTheme(QStringLiteral("lower")), i18n("Send Back One"), this);
    actionCollection()->addAction("backone", backOneAction );
  connect(backOneAction, SIGNAL(triggered(bool)), SLOT(slotBackOne()));

  areaListView->upBtn->addAction(forwardOneAction);
  areaListView->downBtn->addAction(backOneAction);

  connect( areaListView->upBtn, SIGNAL(pressed()), forwardOneAction, SLOT(trigger()));
  connect( areaListView->downBtn, SIGNAL(pressed()), backOneAction, SLOT(trigger()));

    action  = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure KImageMapEditor..."), this);
    actionCollection()->addAction("configure_kimagemapeditor", action );
  connect(action, SIGNAL(triggered(bool)), SLOT(slotShowPreferences()));

  qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor: 1";

  if (areaDock) {

    QAction* a =  areaDock->toggleViewAction();
    a->setText(i18n("Show Area List"));
    actionCollection()->addAction("configure_show_arealist",
				  a);

    a = mapsDock->toggleViewAction();
    a->setText(i18n("Show Map List"));
    actionCollection()->addAction("configure_show_maplist", a );

    a = imagesDock->toggleViewAction();
    a->setText(i18n("Show Image List"));
    actionCollection()->addAction("configure_show_imagelist", a );
  }

  qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor: 2";
  updateActionAccess();
  qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor: 3";
}

void KImageMapEditor::setupStatusBar()
{

//  We can't do this with a KPart !
//	widget()->statusBar()->insertItem(i18n(" Cursor")+" : x: 0 ,y: 0",STATUS_CURSOR);
//	widget()->statusBar()->insertItem(i18n(" Selection")+" : - ",STATUS_SELECTION);
  emit setStatusBarText( i18n(" Selection: -  Cursor: x: 0, y: 0 "));
}

void KImageMapEditor::slotShowPreferences()
{
  PreferencesDialog *dialog = new PreferencesDialog(widget(),config());
  connect(dialog, SIGNAL(preferencesChanged()), this, SLOT(slotConfigChanged()));
  dialog->exec();
  delete dialog;
}


void KImageMapEditor::showPopupMenu(const QPoint & pos, const QString & name)
{
  QMenu* pop = static_cast<QMenu *>(factory()->container(name, this));

  if (!pop) {
      qCWarning(KIMAGEMAPEDITOR_LOG) << QString("KImageMapEditorPart: Missing XML definition for %1\n").arg(name);
      return;
  }

  pop->popup(pos);
}

void KImageMapEditor::slotShowMainPopupMenu(const QPoint & pos)
{
  showPopupMenu(pos,"popup_main");
}

void KImageMapEditor::slotShowMapPopupMenu(const QPoint & pos)
{
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotShowMapPopupMenu";
  QTreeWidgetItem* item = mapsListView->listView()->itemAt(pos);

  if (isReadWrite()) {
    mapDeleteAction->setEnabled(item);
    mapNameAction->setEnabled(item);
    mapDefaultAreaAction->setEnabled(item);
  }

  if (item)
     mapsListView->selectMap(item);

  showPopupMenu(mapsListView->listView()->viewport()->mapToGlobal(pos),"popup_map");
}

void KImageMapEditor::slotShowImagePopupMenu(const QPoint & pos)
{
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotShowImagePopupMenu";
  QTreeWidgetItem* item = imagesListView->itemAt(pos);

  imageRemoveAction->setEnabled(item);
  imageUsemapAction->setEnabled(item);

  if (item)
     imagesListView->setCurrentItem(item);

  showPopupMenu(imagesListView->viewport()->mapToGlobal(pos),"popup_image");
}

void KImageMapEditor::slotShowPopupMenu(const QPoint & p)
{
  QTreeWidgetItem* item = areaListView->listView->itemAt(p);

  if (!item)
    return;

  if (!item->isSelected())
  {
    deselectAll();
    select(item);
  }

  slotShowMainPopupMenu(areaListView->listView->viewport()->mapToGlobal(p));
}

void KImageMapEditor::updateStatusBar()
{
  emit setStatusBarText(selectionStatusText+"  "+cursorStatusText);
}

void KImageMapEditor::slotChangeStatusCoords(int x,int y)
{
//	statusBar()->changeItem(QString(" Cursor : x: %1 ,y: %2 ").arg(x).arg(y),STATUS_CURSOR);
  cursorStatusText = i18n(" Cursor: x: %1, y: %2 ", x, y);
  updateStatusBar();
}

void KImageMapEditor::slotUpdateSelectionCoords() {
  if (selected()->count()>0) {
    QRect r=selected()->rect();
//		statusBar()->changeItem(
    selectionStatusText = i18n(" Selection: x: %1, y: %2, w: %3, h: %4 ", r.left(), r.top(), r.width(), r.height());

//		  ,STATUS_SELECTION);
    qApp->processEvents();
  } else
    selectionStatusText = i18n(" Selection: - ");
    //statusBar()->changeItem(" Selection : - ",STATUS_SELECTION);

  updateStatusBar();
}

void KImageMapEditor::slotUpdateSelectionCoords( const QRect & r )
{
  selectionStatusText = i18n(" Selection: x: %1, y: %2, w: %3, h: %4 ", r.left(), r.top(), r.width(), r.height());
  updateStatusBar();
  qApp->processEvents();
}

void KImageMapEditor::drawToCenter(QPainter* p, const QString & str, int y, int width) {
  int xmid = width / 2;

  QFontMetrics fm = p->fontMetrics();
  QRect strBounds = fm.boundingRect(str);

  p->drawText(xmid-(strBounds.width()/2),y,str);
}


QImage KImageMapEditor::getBackgroundImage() {

  // Lazy initialisation
  if ( _backgroundImage.isNull() ) {


//  QString filename = QString("dropimage_")+KGlobal::locale()->language()+".png";
//  QString path = QString(); //KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/"+filename ) + "kimagemapeditor/"+filename;
//  qCDebug(KIMAGEMAPEDITOR_LOG) << "getBackgroundPic : loaded image : " << path;

//  if ( ! QFileInfo(path).exists() ) {
    int width = 400;
    int height = 400;
    int border = 20;
    int fontSize = 58;

    QPixmap pix(width,height);
    pix.fill(QColor(74,76,74));
    QPainter p(&pix);

    //    QFont font = QFontDatabase().font("Luxi Sans","Bold",fontSize);
    QFont font;
    font.setBold(true);
    font.setPixelSize(fontSize);
    p.setFont( font );
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setPen(QPen(QColor(112,114,112),1));

    // The translated string must be divided into
    // parts with about the same size that fit to the image
    QString str = i18n("Drop an image or HTML file");
    const QStringList strList = str.split(' ');

    // Get the string parts
    QString tmp;
    QStringList outputStrList;
    QFontMetrics fm = p.fontMetrics();

    for ( QStringList::ConstIterator it = strList.begin(); it != strList.end(); ++it ) {
      QString tmp2 = tmp + *it;

        if (fm.boundingRect(tmp2).width() > width-border) {
           outputStrList.append(tmp);
           tmp = *it + ' ';
        }
        else
          tmp = tmp2 + ' ';
    }

    // Last one was forgotten so add it.
    outputStrList.append(tmp);

    // Try to adjust the text vertically centered
    int step = myround(float(height) / (outputStrList.size()+1));
    int y = step;

    for ( QStringList::Iterator it = outputStrList.begin(); it != outputStrList.end(); ++it ) {
        drawToCenter(&p, *it, y, pix.width());
        y += step;
    }

    p.end();

    _backgroundImage = pix.toImage();
  }


  return _backgroundImage;

/*
        QFontDatabase fdb;
    QStringList families = fdb.families();
    for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f ) {
        QString family = *f;
        qDebug( family );
        QStringList styles = fdb.styles( family );
        for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s ) {
            QString style = *s;
            QString dstyle = "\t" + style + " (";
            QValueList<int> smoothies = fdb.smoothSizes( family, style );
            for ( QValueList<int>::Iterator points = smoothies.begin();
                  points != smoothies.end(); ++points ) {
                dstyle += QString::number( *points ) + " ";
            }
            dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
            qDebug( dstyle );
        }
    }


    path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + "kimagemapeditor/" ) +filename;
    qCDebug(KIMAGEMAPEDITOR_LOG) << "getBackgroundPic : save new image to : " << path;
    pix.save(path,"PNG",100);
  }

  if ( ! QFileInfo(path).exists() ) {
      qCCritical(KIMAGEMAPEDITOR_LOG) << "Couldn't find needed " << filename << " file in "
                   "the data directory of KImageMapEditor.\n"
                   "Perhaps you have forgotten to do a make install !";
      exit(1);
  }
*/
}


void KImageMapEditor::addArea(Area* area) {
  if (!area) return;

  // Perhaps we've got a selection of areas
  // so test it and add all areas of the selection
  // nested selections are possible but doesn't exist
  AreaSelection *selection = nullptr;
  if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaListIterator it = selection->getAreaListIterator();
    while (it.hasNext()) {
      Area* a = it.next();
      areas->prepend(a);
      a->setListViewItem(new QTreeWidgetItem(
          areaListView->listView,
          QStringList(a->attribute("href"))));
      a->listViewItem()->setIcon(1,QIcon(makeListViewPix(*a)));
    }
  }
  else
  {
    areas->prepend(area);
    area->setListViewItem(new QTreeWidgetItem(
      areaListView->listView,
      QStringList(area->attribute("href"))));
    area->listViewItem()->setIcon(1,QIcon(makeListViewPix(*area)));
  }

  setModified(true);

}

void KImageMapEditor::addAreaAndEdit(Area* s)
{
  areas->prepend(s);
  s->setListViewItem(new QTreeWidgetItem(
    areaListView->listView,
    QStringList(s->attribute("href"))));
  s->listViewItem()->setIcon(1,QIcon(makeListViewPix(*s)));
  deselectAll();
  select(s);
  if (!showTagEditor(selected())) {
    // If the user has pressed cancel
    // he undos the creation
    commandHistory()->undo();
  }
}

void KImageMapEditor::deleteArea( Area * area )
{
  if (!area) return;

  // only for repaint reasons
  QRect redrawRect = area->selectionRect();

  // Perhaps we've got a selection of areas
  // so test it and delete the whole selection
  // nested selections are possible but doesn't exist
  AreaSelection *selection = nullptr;
    if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaListIterator it = selection->getAreaListIterator();
    while (it.hasNext()) {
      Area* a = it.next();
      currentSelected->remove(a);
      areas->removeAll( a );
      a->deleteListViewItem();
    }
  }
  else
  {
    deselect( area );
    areas->removeAll( area );
    area->deleteListViewItem();
  }

  drawZone->repaintRect(redrawRect);


  // Only to disable cut and copy actions
  if (areas->count()==0)
    deselectAll();

  setModified(true);
}

void KImageMapEditor::deleteSelected() {

  AreaListIterator it = currentSelected->getAreaListIterator();
  while (it.hasNext()) {
    Area *a = it.next();
    currentSelected->remove( a );
    areas->removeAll( a );
    delete a->listViewItem();
  }


  drawZone->repaintArea( *currentSelected );
  // Only to disable cut and copy actions
  if (areas->count()==0)
    deselectAll();

  setModified(true);
}

void KImageMapEditor::deleteAllAreas()
{
  Area* a;
  foreach (a,*areas) {
    deselect( a );
    areas->removeAll( a );
    a->deleteListViewItem();
    if (!areas->isEmpty())
      a = areas->first(); // because the current is deleted
  }

  drawZone->repaint();

}

void KImageMapEditor::updateAllAreas()
{
//  qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::updateAllAreas";
  Area* a;
  foreach(a,*areas) {
    a->listViewItem()->setIcon(1,QIcon(makeListViewPix(*a)));
  }
  drawZone->repaint();
}

void KImageMapEditor::updateSelection() const {
  //FIXME: areaListView->listView->triggerUpdate();
}

AreaSelection* KImageMapEditor::selected() const {
  return currentSelected;
}

void KImageMapEditor::select(Area* a)
{
  if (!a) return;

  currentSelected->add(a);
  updateActionAccess();
  slotUpdateSelectionCoords();
//	drawZone->repaintArea( *a);

}

void KImageMapEditor::selectWithoutUpdate(Area* a)
{
  if (!a) return;
  currentSelected->add(a);
}

void KImageMapEditor::slotSelectionChanged()
{
  AreaListIterator it = areaList();
  AreaList list = currentSelected->getAreaList();

  while (it.hasNext()) {
    Area* a = it.next();
    if ( a->listViewItem()->isSelected() != (list.contains(a)) )
    {
      a->listViewItem()->isSelected()
        ? select( a )
        :	deselect( a );

      drawZone->repaintArea( *a);
    }
  }

}

void KImageMapEditor::select( QTreeWidgetItem* item)
{

  AreaListIterator it = areaList();
  while (it.hasNext()) {
    Area* a = it.next();
    if (a->listViewItem() == item )
    {
      select( a );
      drawZone->repaintArea( *a);
    }
  }


}

AreaListIterator KImageMapEditor::areaList() const {
  AreaListIterator it(*areas);
  return it;
}


void KImageMapEditor::slotAreaChanged(Area *area)
{
  if (!area)
    return;

  setModified(true);

  AreaSelection *selection = nullptr;
  if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaListIterator it = selection->getAreaListIterator();
    while (it.hasNext()) {
      Area* a = it.next();
      if (a->listViewItem()) {
        a->listViewItem()->setText(0,a->attribute("href"));
        a->listViewItem()->setIcon(1,QIcon(makeListViewPix(*a)));
      }
    }

  }
  else
  if (area->listViewItem()) {
    area->listViewItem()->setText(0,area->attribute("href"));
    area->listViewItem()->setIcon(1,QIcon(makeListViewPix(*area)));
  }

  drawZone->repaintArea(*area);

}

void KImageMapEditor::deselect(Area* a)
{
  if (a) {
    currentSelected->remove(a);
//		drawZone->repaintArea(*a);
    updateActionAccess();
    slotUpdateSelectionCoords();
  }
}

void KImageMapEditor::deselectWithoutUpdate(Area* a)
{
  if (a) {
    currentSelected->remove(a);
  }
}


/**
* Makes sure, that the actions cut, copy, delete and
* show properties
* can only be executed if sth. is selected.
**/
void KImageMapEditor::updateActionAccess()
{
  if (!isReadWrite())
     return;

  if ( 0 < selected()->count())
  {
    qCDebug(KIMAGEMAPEDITOR_LOG) << "actions enabled";
    areaPropertiesAction->setEnabled(true);
    deleteAction->setEnabled(true);
    copyAction->setEnabled(true);
    cutAction->setEnabled(true);
    moveLeftAction->setEnabled(true);
    moveRightAction->setEnabled(true);
    moveUpAction->setEnabled(true);
    moveDownAction->setEnabled(true);
    toFrontAction->setEnabled(true);
    toBackAction->setEnabled(true);

    if ( (selected()->count() == 1) )
    {
      if (selected()->type()==Area::Polygon)
      {
        increaseWidthAction->setEnabled(false);
        decreaseWidthAction->setEnabled(false);
        increaseHeightAction->setEnabled(false);
        decreaseHeightAction->setEnabled(false);
        addPointAction->setEnabled(true);
        removePointAction->setEnabled(true);
      }
      else
      {
        increaseWidthAction->setEnabled(true);
        decreaseWidthAction->setEnabled(true);
        increaseHeightAction->setEnabled(true);
        decreaseHeightAction->setEnabled(true);
        addPointAction->setEnabled(false);
        removePointAction->setEnabled(false);
      }

    }
    else
    {
      increaseWidthAction->setEnabled(false);
      decreaseWidthAction->setEnabled(false);
      increaseHeightAction->setEnabled(false);
      decreaseHeightAction->setEnabled(false);
      addPointAction->setEnabled(false);
      removePointAction->setEnabled(false);
    }

  }
  else
  {
    qCDebug(KIMAGEMAPEDITOR_LOG) << "Actions disabled";
    areaPropertiesAction->setEnabled(false);
    deleteAction->setEnabled(false);
    copyAction->setEnabled(false);
    cutAction->setEnabled(false);
    moveLeftAction->setEnabled(false);
    moveRightAction->setEnabled(false);
    moveUpAction->setEnabled(false);
    moveDownAction->setEnabled(false);
    increaseWidthAction->setEnabled(false);
    decreaseWidthAction->setEnabled(false);
    increaseHeightAction->setEnabled(false);
    decreaseHeightAction->setEnabled(false);
    toFrontAction->setEnabled(false);
    toBackAction->setEnabled(false);
    addPointAction->setEnabled(false);
    removePointAction->setEnabled(false);

  }

  updateUpDownBtn();
}

void KImageMapEditor::updateUpDownBtn()
{
  if (!isReadWrite())
     return;

  AreaList list = currentSelected->getAreaList();

  if (list.isEmpty() || (areas->count() < 2)) {
    forwardOneAction->setEnabled(false);
    areaListView->upBtn->setEnabled(false);
    backOneAction->setEnabled(false);
    areaListView->downBtn->setEnabled(false);
    return;
  }
  // if the first Area is in the selection can't move up
  if (list.contains( areas->first() )) {
    forwardOneAction->setEnabled(false);
    areaListView->upBtn->setEnabled(false);
  } else {
    forwardOneAction->setEnabled(true);
    areaListView->upBtn->setEnabled(true);
  }

  drawZone->repaintArea(*currentSelected);

  // if the last Area is in the selection can't move down
  if (list.contains( areas->last() )) {
    backOneAction->setEnabled(false);
    areaListView->downBtn->setEnabled(false);
  }
  else {
    backOneAction->setEnabled(true);
    areaListView->downBtn->setEnabled(true);
  }

}

void KImageMapEditor::deselectAll()
{
  QRect redrawRect= currentSelected->selectionRect();
  currentSelected->reset();
  drawZone->repaintRect(redrawRect);
  updateActionAccess();
}

Area* KImageMapEditor::onArea(const QPoint & p) const {
  Area* s;
  foreach(s,*areas) {
    if (s->contains(p))
      return s;
  }
  return nullptr;
}


int KImageMapEditor::showTagEditor(Area *a) {
  if (!a) return 0;
  drawZone->repaintArea(*a);

  AreaDialog *dialog= new AreaDialog(this,a);
  connect (dialog, SIGNAL(areaChanged(Area*)), this, SLOT(slotAreaChanged(Area*)));

  int result = dialog->exec();

  return result;


}

int KImageMapEditor::showTagEditor(QTreeWidgetItem *item) {
  if (!item)
    return 0;

  Area* a;
  foreach(a,*areas) {
    if (a->listViewItem()==item) {
      return showTagEditor(a);
    }
  }
  return 0;
}

int KImageMapEditor::showTagEditor() {
  return showTagEditor(selected());
}


QString KImageMapEditor::getHTMLImageMap() const {
  QString retStr;
  retStr+="<map "+QString("name=\"")+_mapName+"\">\n";

  Area* a;
  foreach(a,*areas) {
    retStr+="  "+a->getHTMLCode()+'\n';
  }

  if (defaultArea && defaultArea->finished())
    retStr+="  "+defaultArea->getHTMLCode()+'\n';

  retStr+="</map>";
  return retStr;
}

QPixmap KImageMapEditor::makeListViewPix(Area & a)
{
  QPixmap pix=a.cutOut(drawZone->picture());

  double shrinkFactor=1;

  // picture fits into max row height ?
  if (maxAreaPreviewHeight < pix.height())
    shrinkFactor = ( (double) maxAreaPreviewHeight / pix.height() );

  QPixmap pix2((int)(pix.width()*shrinkFactor), (int)(pix.height()*shrinkFactor));

  // Give all pixels a defined color
  pix2.fill(Qt::white);

  QPainter p(&pix2);

  p.scale(shrinkFactor,shrinkFactor);
  p.drawPixmap(0,0,pix);

  return pix2;
}

void KImageMapEditor::setMapName(const QString & s) {
    mapsListView->changeMapName(_mapName, s);
    _mapName=s;
    currentMapElement->mapTag->name = s;
}


void KImageMapEditor::setPicture(const QUrl & url) {
  _imageUrl=url;
  if (QFileInfo::exists(url.path())) {
     QImage img(url.path());

     if (!img.isNull()) {
         setPicture(img);
         imageRemoveAction->setEnabled(true);
         imageUsemapAction->setEnabled(true);
     }
     else
         qCCritical(KIMAGEMAPEDITOR_LOG) << QString("The image %1 could not be opened.").arg(url.path());
  }
  else
     qCCritical(KIMAGEMAPEDITOR_LOG) << QString("The image %1 does not exist.").arg(url.path());
}

void KImageMapEditor::setPicture(const QImage & pix) {
    drawZone->setPicture(pix);
    updateAllAreas();
}


void KImageMapEditor::slotDrawArrow() {
  _currentToolType=KImageMapEditor::Selection;

}

void KImageMapEditor::slotDrawCircle() {
  _currentToolType=KImageMapEditor::Circle;
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotDrawCircle";

}

void KImageMapEditor::slotDrawRectangle() {
  _currentToolType=KImageMapEditor::Rectangle;
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotDrawRectangle";

}

void KImageMapEditor::slotDrawPolygon() {
  _currentToolType=KImageMapEditor::Polygon;
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotDrawPolygon";
}

void KImageMapEditor::slotDrawFreehand() {
  _currentToolType=KImageMapEditor::Freehand;
}

void KImageMapEditor::slotDrawAddPoint() {
  _currentToolType=KImageMapEditor::AddPoint;
}

void KImageMapEditor::slotDrawRemovePoint() {
  _currentToolType=KImageMapEditor::RemovePoint;
}


void KImageMapEditor::slotZoom() {

  int i=zoomAction->currentItem();
  switch (i) {
    case 0 : drawZone->setZoom(0.25);break;
    case 1 : drawZone->setZoom(0.5);break;
    case 2 : drawZone->setZoom(1);break;
    case 3 : drawZone->setZoom(1.5);break;
    case 4 : drawZone->setZoom(2.0);break;
    case 5 : drawZone->setZoom(2.5);break;
    case 6 : drawZone->setZoom(3);break;
    case 7 : drawZone->setZoom(5);break;
    case 8 : drawZone->setZoom(7.5);break;
    case 9 : drawZone->setZoom(10);break;
  }
  if (i<10)
    zoomInAction->setEnabled(true);
  else
    zoomInAction->setEnabled(false);

  if (i>0)
    zoomOutAction->setEnabled(true);
  else
    zoomOutAction->setEnabled(false);
}

void KImageMapEditor::slotZoomIn() {
  if (zoomAction->currentItem()==(int)(zoomAction->items().count()-1))
    return;

  zoomAction->setCurrentItem(zoomAction->currentItem()+1);
  slotZoom();
}

void KImageMapEditor::slotZoomOut() {
  if (zoomAction->currentItem()==0)
    return;

  zoomAction->setCurrentItem(zoomAction->currentItem()-1);
  slotZoom();
}

void KImageMapEditor::mapDefaultArea()
{
  if (defaultArea)
    showTagEditor(defaultArea);
  else {
    defaultArea= new DefaultArea();
    showTagEditor(defaultArea);
  }

}

void KImageMapEditor::mapEditName()
{
  bool ok=false;
  QString input = QInputDialog::getText(widget(),
    i18n("Enter Map Name"), i18n("Enter the name of the map:"),
    QLineEdit::Normal, _mapName, &ok);
  if (ok && !input.isEmpty()) {
    if (input != _mapName) {
        if (mapsListView->nameAlreadyExists(input))
            KMessageBox::sorry(this->widget(), i18n("The name <em>%1</em> already exists.", input));
        else {
            setMapName(input);
        }
    }
  }
}

void KImageMapEditor::mapShowHTML()
{
  QDialog *dialog = new QDialog(widget());
  dialog->setModal(true);
  dialog->setWindowTitle(i18n("HTML Code of Map"));
  QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

  QTextEdit *edit = new QTextEdit;

  edit->setPlainText(getHtmlCode());
  edit->setReadOnly(true);
  edit->setLineWrapMode(QTextEdit::NoWrap);
  mainLayout->addWidget(edit);
//  dialog->resize(dialog->calculateSize(edit->maxLineWidth(),edit->numLines()*));
//	dialog->adjustSize();

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
  mainLayout->addWidget(buttonBox);

  dialog->resize(600,400);
  dialog->exec();
  delete dialog;
}

void KImageMapEditor::openFile(const QUrl & url) {
  if ( ! url.isEmpty()) {
    QMimeDatabase db;
    QMimeType openedFileType = db.mimeTypeForUrl(url);
    if (openedFileType.name().left(6) == "image/") {
        addImage(url);
    } else {
        openURL(url);
    }
  }
}

bool KImageMapEditor::openURL(const QUrl & url) {
    // If a local file does not exist
    // we start with an empty file, so
    // that we can return true here.
    // For non local files, we cannot check
    // the existence
    if (url.isLocalFile() &&
        ! QFile::exists(url.path()))
        return true;
    return KParts::ReadWritePart::openUrl(url);
}

void KImageMapEditor::fileOpen() {

  QString fileName = QFileDialog::getOpenFileName(widget(), i18n("Choose File to Open"), QString(),
                     i18n("Web File (*.png *.jpg *.jpeg *.gif *.htm *.html);;Images (*.png *.jpg *.jpeg *.gif *.bmp *.xbm *.xpm *.pnm *.mng);;"
                          "HTML Files (*.htm *.html);;All Files (*)"));

  openFile(QUrl::fromUserInput( fileName ));
}



void KImageMapEditor::fileClose()
{
  if (! closeUrl())
     return;


    setPicture(getBackgroundImage());
    recentFilesAction->setCurrentItem(-1);
    setModified(false);
}

void KImageMapEditor::fileSave()
{
  // if we aren't read-write, return immediately
  if ( ! isReadWrite() )
      return;

  if (url().isEmpty()) {
    fileSaveAs();
  }
  else {
    saveFile();
    setModified(false);
  }


}

void KImageMapEditor::fileSaveAs() {

  QUrl url = QFileDialog::getSaveFileUrl(widget(), QString(), QUrl(), i18n("HTML File (*.htm *.html);;Text File (*.txt);;All Files (*)" ));
  if (url.isEmpty() || !url.isValid()) {
    return;
  }


  saveAs(url);
  recentFilesAction->addUrl(url);

}


bool KImageMapEditor::openFile()
{
  QUrl u = url();
  QFileInfo fileInfo(u.path());

  if ( !fileInfo.exists() )
  {
      KMessageBox::information(widget(),
        i18n("<qt>The file <b>%1</b> does not exist.</qt>", fileInfo.fileName()),
        i18n("File Does Not Exist"));
      return false;
  }

  openHTMLFile(u);

  drawZone->repaint();
  recentFilesAction->addUrl(u);
  setModified(false);
  backupFileCreated = false;
  return true;
}

/**
 * This method supposes that the given QTextStream s has just read
 * the &lt; of a tag. It now reads all attributes of the tag until a &gt;
 * The tagname itself is also read and stored as a <em>tagname</em>
 * attribute. After parsing the whole tag it returns a QDict<QString>
 * with all attributes and their values. It stores the whole read text in the
 * parameter readText.
 */
QHash<QString,QString> KImageMapEditor::getTagAttributes(QTextStream & s, QString & readText)
{
  QHash<QString,QString> dict;
  // the "<" is already read
  QChar w;
  QString attr,value;

  readText.clear();

  // get the tagname
  while (!s.atEnd() && w!=' ') {
    s >> w;
    readText.append(w);
    if (w.isSpace() || w=='>') {
      dict.insert("tagname",value);
      break;
    }
    value+=w;
  }


  // do we have a comment ?
  // read the comment and return
  if (value.right(3)=="-->")
    return dict;

  if (value.startsWith(QLatin1String("!--"))) {
    while (!s.atEnd()) {
      s >> w;
      readText.append(w);

      if (w=='-') {
        s >> w;
        readText.append(w);
        if (w=='-') {
          s >> w;
          readText.append(w);
          if (w=='>')
            return dict;
        }
      }
    }
  }

  bool attrRead=true;	// currently reading an attribute ?
  bool equalSign=false; // an equalsign was read?
  bool valueRead=false; // currently reading a value ?
  QChar quotation='\0'; // currently reading a value with quotation marks ?
  bool php=false; // currently reading a php script
  attr.clear();
  value.clear();

  //get the other attributes
  while (!s.atEnd() && w!='>')
  {
    s >> w;
    readText.append(w);

    // End of PHP Script ?
    if (php && (w=='?') )
    {
      s >> w;
      readText.append(w);

      if (valueRead)
          value+=w;

      if (w=='>')
      {
        php = false;
        s >> w;
        readText.append(w);
      }
    }

    // Wrong syntax or PHP-Script !
    if (!php && (w=='<'))
    {
      if (valueRead)
        value+=w;
      s >> w;
      readText.append(w);
      if (valueRead)
        value+=w;

      if (w=='?')
      {
        php = true;
      }
    } else
    // finished ?
    if (w=='>') {
      if (valueRead) {
        dict.insert(attr,value);
      }
      return dict;
    } else
    // currently reading an attribute ?
    if (attrRead) {
      // if there is a whitespace the attributename has finished
      // possibly there isn't any value e.g. noshade
      if (w.isSpace())
        attrRead=false;
      else
      // an equal sign signals that the value follows
      if (w=='=') {
        attrRead=false;
        equalSign=true;
      } else
        attr+=w;
    } else
    // an equal sign was read ? delete every whitespace
    if (equalSign) {
      if (!w.isSpace()) {
        equalSign=false;
        valueRead=true;
        if (w=='"' || w=='\'')
          quotation=w;
      }
    } else
    // currently reading the value
    if (valueRead) {
      // if php, read without regarding anything
      if (php)
        value+=w;
      // if value within quotation marks is read
      // only stop when another quotationmark is found
      else
      if (quotation != '\0') {
        if (quotation!=w) {
          value+=w;
        } else {
          quotation='\0';
          valueRead=false;
          dict.insert(attr,value);
          attr.clear();
          value.clear();
        }
      } else
      // a whitespace indicates that the value has finished
      if (w.isSpace()) {
        valueRead=false;
        dict.insert(attr,value);
        attr.clear();
        value.clear();
      }
    } else {
      if (!w.isSpace()) {
        attrRead=true;
        attr+=w;
      }
    }
  }

  return dict;

}


bool KImageMapEditor::openHTMLFile(const QUrl & url)
{
  QFile f(url.path());
  if ( !f.exists () )
      return false;
  f.open(QIODevice::ReadOnly);
  QTextStream s(&f);
  QChar w;
  QHash<QString,QString> *attr = nullptr;
  QList<ImageTag*> images;
  MapTag *map = nullptr;
  QList<MapTag*> maps;

  _htmlContent.clear();
  currentMapElement = nullptr;

  QString temp;
  QString origcode;

  bool readMap=false;

  while (!s.atEnd()) {

    s >> w;
    if (w=='<')
    {
      if (!readMap && !origcode.isEmpty()) {
        _htmlContent.append( new HtmlElement(origcode));
        origcode.clear();
      }

      origcode.append("<");
      attr=new QHash<QString,QString>(getTagAttributes(s,temp));
      origcode.append(temp);

      if (attr->contains("tagname")) {
        QString tagName = attr->value("tagname").toLower();
        if (tagName =="img") {
          HtmlImgElement *el = new HtmlImgElement(origcode);
          el->imgTag = static_cast<ImageTag*>(attr);
          images.append(el->imgTag);
          _htmlContent.append(el);

          origcode.clear();
        } else
        if (tagName == "map") {
          map = new MapTag();
          map->name = attr->value("name");
	  qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::openHTMLFile: found map with name:" << map->name;
	  
          readMap=true;
        } else
        if (tagName=="/map") {
          readMap=false;
          maps.append(map);
          HtmlMapElement *el = new HtmlMapElement(origcode);
          el->mapTag = map;
          _htmlContent.append(el);

          origcode.clear();
        } else
        if (readMap) {
          if (tagName=="area") {
             map->prepend(*attr);
          }
        } else {
          _htmlContent.append(new HtmlElement(origcode));
          origcode.clear();
        }

      }
    } // w != "<"
    else {
      origcode.append(w);
    }
  }

  if (!origcode.isEmpty()) {
    _htmlContent.append(new HtmlElement(origcode));
  }

  f.close();

  QUrl imageUrl;

  map = nullptr;

    // If we have more than on map or more than one image
    // Let the user choose, otherwise take the only ones
    if (maps.count() > 1) {
      map = maps.first();
    }

    if (images.count() > 1) {
      ImageTag* imgTag = images.first();
      if (imgTag) {
        if (imgTag->contains("src")) {
            if (url.path().isEmpty() | !url.path().endsWith('/')) {
                imageUrl = QUrl(url.path() + '/').resolved(QUrl(imgTag->value("src")));
            }
            else {
                imageUrl = url.resolved(QUrl(imgTag->value("src")));
            }
        }
      }
    }

    // If there is more than one map and more than one image
    // use the map that has an image with an according usemap tag
    if (maps.count() > 1 && images.count() > 1) {
      bool found = false;
      MapTag *mapTag;
      foreach(mapTag, maps) {
        ImageTag *imageTag;
        foreach(imageTag, images) {
            if (imageTag->contains("usemap")) {
                QString usemap = imageTag->value("usemap");
                // Remove the #
                QString usemapName = usemap.right(usemap.length()-1);
                if (usemapName == mapTag->name) {
		  if (imageTag->contains("src")) {
                      if (url.path().isEmpty() | !url.path().endsWith('/')) {
                          imageUrl = QUrl(url.path() + '/').resolved(QUrl(imageTag->value("src")));
                      }
                      else {
                          imageUrl = url.resolved(QUrl(imageTag->value("src")));
                      }
		      found = true;
		  }
                }
            }
	    if (found)
	      break;
        }
	if (found)
	  break;
      }
      if (found) {
	map = mapTag;
      }
    }


    // If there are more than one map or there wasn't
    // found a fitting image and there is something to choose
    // let the user choose
    /*    if (maps.count() >1 || (imageUrl.isEmpty() && images.count() > 1))
    {
      ImageMapChooseDialog dialog(widget(),maps,images,url);
      qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::openHTMLFile: before dialog->exec()";
      dialog.exec();
      qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::openHTMLFile: after dialog->exec()";
      map = dialog.currentMap;
      imageUrl = dialog.pixUrl;
      }*/
  

  imagesListView->clear();
  imagesListView->setBaseUrl(url);
  imagesListView->addImages(images);

  mapsListView->clear();
  mapsListView->addMaps(maps);


  setMapActionsEnabled(false);

  if (map) {
    mapsListView->selectMap(map->name);
  } else {
#ifdef WITH_TABWIDGET
    if (tabWidget)
       tabWidget->showPage(mapsListView);
#endif
  }


  if (!imageUrl.isEmpty()) {
    setPicture(imageUrl);
  } else {
    setPicture(getBackgroundImage());
#ifdef WITH_TABWIDGET
    if (tabWidget)
       tabWidget->showPage(imagesListView);
#endif
  }


  emit setWindowCaption(url.fileName());
  setModified(false);
  return true;
}

/**
 * Finds the first html element which contains the given text.
 * Returns the first matching element.
 * Returns 0L if no element was found.
 */
HtmlElement* KImageMapEditor::findHtmlElement(const QString & containingText) {
  HtmlElement *el;
  foreach (el,_htmlContent) {
    if (el->htmlCode.contains(containingText,Qt::CaseInsensitive)) {
      return el;
    }
  }
  return nullptr;
}

/**
 * Finds the first html element which contains the given ImageTag.
 * Returns the first matching element.
 * Returns 0L if no element was found.
 */
HtmlImgElement* KImageMapEditor::findHtmlImgElement(ImageTag* tag) {
  HtmlElement* el;
  foreach(el,_htmlContent) {
    HtmlImgElement* imgEl = dynamic_cast<HtmlImgElement*>(el);

    if (imgEl && imgEl->imgTag == tag)
       return imgEl;
  }
  return nullptr;
}

void KImageMapEditor::addMap(const QString & name = QString()) {
  HtmlMapElement* el = new HtmlMapElement("\n<map></map>");
  MapTag* map = new MapTag();
  map->name = name;
  el->mapTag = map;

  // Try to find the body tag
  HtmlElement* bodyTag = findHtmlElement("<body");

  // if we found one add the new map right after the body tag
  if (bodyTag) {
     uint index = _htmlContent.indexOf(bodyTag);

     // Add a newline before the map
     _htmlContent.insert(index+1, new HtmlElement("\n"));

     _htmlContent.insert(index+2, el);
  } // if there is no body tag we add the map to the end of the file
  else {
     // Add a newline before the map
     _htmlContent.append(new HtmlElement("\n"));

     _htmlContent.append(el);
     qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::addMap : No <body found ! Appending new map to the end.";
  }

  mapsListView->addMap(name);
  mapsListView->selectMap(name);
}

/**
 * Finds the HtmlMapElement in the HtmlContent, that corresponds
 * to the given map name.<br>
 * Returns 0L if there exists no map with the given name
 */
HtmlMapElement* KImageMapEditor::findHtmlMapElement(const QString & mapName) {
  foreach(HtmlElement * el,_htmlContent) {
    if (dynamic_cast<HtmlMapElement*>(el)) {
      HtmlMapElement *tagEl = static_cast<HtmlMapElement*>(el);
      if (tagEl->mapTag->name == mapName) {
         return tagEl;
      }
    }
  }

  qCWarning(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::findHtmlMapElement: couldn't find map '" << mapName << "'";
  return nullptr;
}

/**
 * Calls setMap with the HtmlMapElement with the given map name
 */
void KImageMapEditor::setMap(const QString & mapName) {
    HtmlMapElement* el = findHtmlMapElement(mapName);
    if (!el) {
      qCWarning(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::setMap : Couldn't set map '" << mapName << "', because it wasn't found !";
      return;
    }

    setMap(el);

}

void KImageMapEditor::setMap(MapTag* map) {
  HtmlElement * el;
  foreach(el,_htmlContent) {
    HtmlMapElement *tagEl = dynamic_cast<HtmlMapElement*>(el);
    if (tagEl) {
      if (tagEl->mapTag == map) {
         setMap(tagEl);
         break;
      }
    }
  }

}

void KImageMapEditor::saveAreasToMapTag(MapTag* map) {
  map->clear();
  Area* a;
  foreach(a,*areas) {
    QString shapeStr;

    switch (a->type()) {
      case Area::Rectangle : shapeStr = "rect";break;
      case Area::Circle : shapeStr = "circle";break;
      case Area::Polygon : shapeStr = "poly";break;
      default : continue;
    }

    QHash<QString,QString> dict;
    dict.insert("shape",shapeStr);

    AttributeIterator it = a->attributeIterator();
    while (it.hasNext())
    {
      it.next();
      dict.insert(it.key(),it.value());
    }

    dict.insert("coords",a->coordsToString());

    map->append(dict);

  }

  if (defaultArea && defaultArea->finished()) {
    QHash<QString,QString> dict;
    dict.insert("shape","default");

    AttributeIterator it = defaultArea->attributeIterator();
    while (it.hasNext())
    {
      it.next();
      dict.insert(it.key(),it.value());
    }

    map->append(dict);
  }

}

static void setAttribute(Area* a, const AreaTag & tag, const QString & s) {
  if (tag.contains(s))
    a->setAttribute(s,tag.value(s));
}

void KImageMapEditor::setMap(HtmlMapElement* mapElement) {
  if (currentMapElement) {
    currentMapElement->mapTag->modified=true;
    currentMapElement->htmlCode = getHTMLImageMap();
    saveAreasToMapTag(currentMapElement->mapTag);
  }

  currentMapElement = mapElement;
  MapTag* map = currentMapElement->mapTag;

  // Remove old areas only if a new map is loaded
  deleteAllAreas();
  delete defaultArea;
  defaultArea = nullptr;
//    qCDebug(KIMAGEMAPEDITOR_LOG) << "KImageMapEditor::setMap : Setting new map : " << map->name;
    _mapName = map->name;
    AreaTag tag;

    QLinkedListIterator<AreaTag> it(*map);
    while (it.hasNext()) {
        tag = it.next();
        QString shape="rect";
        if (tag.contains("shape"))
          shape=tag.value("shape");

        Area::ShapeType type=Area::Rectangle;
        if (shape=="circle")
          type=Area::Circle;
        else if (shape=="poly")
          type=Area::Polygon;
        else if (shape=="default")
          type=Area::Default;

        Area* a=AreaCreator::create(type);

        setAttribute(a,tag,"href");
        setAttribute(a,tag,"alt");
        setAttribute(a,tag,"target");
        setAttribute(a,tag,"title");
        setAttribute(a,tag,"onclick");
        setAttribute(a,tag,"ondblclick");
        setAttribute(a,tag,"onmousedown");
        setAttribute(a,tag,"onmouseup"); 
        setAttribute(a,tag,"onmouseover");
        setAttribute(a,tag,"onmousemove");
        setAttribute(a,tag,"onmouseout");

        if (type==Area::Default) {
          defaultArea=a;
          defaultArea->setFinished(true);
          continue;
        }

        if (tag.contains("coords"))
          a->setCoords(tag.value("coords"));

        a->setMoving(false);
        addArea(a);
    }

    updateAllAreas();

    setMapActionsEnabled(true);
}

/**
 * Sets whether actions that depend on an selected map
 * are enabled
 */
void KImageMapEditor::setMapActionsEnabled(bool b) {
   mapDeleteAction->setEnabled(b);
   mapDefaultAreaAction->setEnabled(b);
   mapNameAction->setEnabled(b);

   arrowAction->setChecked(true);
   slotDrawArrow();

   arrowAction->setEnabled(b);
   circleAction->setEnabled(b);
   rectangleAction->setEnabled(b);
   polygonAction->setEnabled(b);
   freehandAction->setEnabled(b);
   addPointAction->setEnabled(b);
   removePointAction->setEnabled(b);

}

QString KImageMapEditor::getHtmlCode() {
  if (currentMapElement) {
    currentMapElement->htmlCode = getHTMLImageMap();
  }

  QString result;

  HtmlElement *el;
  foreach(el,_htmlContent) {
        result += el->htmlCode;
  }
  return result;
}


/**
 create a relative short url based in baseURL

 taken from qextfileinfo.cpp:

 From WebMaker - KDE HTML Editor
 Copyright (C) 1998, 1999 Alexei Dets <dets@services.ru>

 Rewritten for Quanta Plus: (C) 2002 Andras Mantia <amantia@freemail.hu>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
*/
static QUrl toRelative(const QUrl& urlToConvert,const QUrl& baseURL)
{
  QUrl resultURL = urlToConvert;
  if (urlToConvert.scheme() == baseURL.scheme())
  {
    QString path = urlToConvert.path();
    QString basePath = baseURL.path().endsWith('/') ? baseURL.path() : baseURL.path() + '/';
    if (path.startsWith(QLatin1String("/")) && basePath != "/")
    {
      path.remove(0, 1);
      basePath.remove(0, 1);
      if ( basePath.right(1) != "/" ) basePath.append("/");

      int pos=0;
      int pos1=0;
      for (;;)
      {
        pos=path.indexOf("/");
        pos1=basePath.indexOf("/");
        if ( pos<0 || pos1<0 ) break;
        if ( path.left(pos+1 ) == basePath.left(pos1+1) )
        {
          path.remove(0, pos+1);
          basePath.remove(0, pos1+1);
        }
        else
          break;
      };

      if ( basePath == "/" ) basePath="";
      int level = basePath.count("/");
      for (int i=0; i<level; i++)
      {
        path="../"+path;
      };
    }

    resultURL.setPath(QDir::cleanPath(path));
  }

  if (urlToConvert.path().endsWith('/')) resultURL.setPath(resultURL.path() + '/');
  return resultURL;
}


void KImageMapEditor::saveImageMap(const QUrl & url)
{
  if (!QFileInfo(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path()).isWritable()) {
    KMessageBox::error(widget(),
      i18n("<qt>The file <i>%1</i> could not be saved, because you do not have the required write permissions.</qt>", url.path()));
    return;
  }

  if (!backupFileCreated) {
    QString backupFile = url.path()+'~';
    KIO::file_copy(url, QUrl::fromUserInput(backupFile ), -1, KIO::Overwrite | KIO::HideProgressInfo);
    backupFileCreated = true;
  }

  setModified(false);

  if (mapName().isEmpty()) {
    mapEditName();
  }
  QFile file(url.path());
  file.open(QIODevice::WriteOnly);

  QTextStream t(&file);

  if (_htmlContent.isEmpty()) {
    t << "<html>\n"
      << "<head>\n"
      << "  <title></title>\n"
      << "</head>\n"
      << "<body>\n"
      << "  " << getHTMLImageMap()
      << "\n"
      << "  <img src=\"" << toRelative(_imageUrl,QUrl( url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path() )).path() << "\""
      << " usemap=\"#" << _mapName << "\""
      << " width=\"" << drawZone->picture().width() << "\""
      << " height=\"" << drawZone->picture().height() << "\">\n"
      << "</body>\n"
      << "</html>";
  } else
  {
    t << getHtmlCode();
  }

  file.close();

}


void KImageMapEditor::slotCut()
{
  if ( 0 == currentSelected->count() )
    return;
  delete copyArea;

  copyArea= static_cast< AreaSelection* > (currentSelected->clone());
  pasteAction->setEnabled(true);
  QUndoCommand *command= new CutCommand(this,*currentSelected);
  commandHistory()->push(command);
}


void KImageMapEditor::slotDelete()
{
  if ( 0 == currentSelected->count() )
    return;

  QUndoCommand *command= new DeleteCommand(this,*currentSelected);
  commandHistory()->push(command);
}

void KImageMapEditor::slotCopy()
{
  delete copyArea;

  copyArea = static_cast< AreaSelection* > (currentSelected->clone());
  pasteAction->setEnabled(true);
}

void KImageMapEditor::slotPaste()
{
  if (!copyArea)
    return;

  copyArea->moveBy(5,5);
  if (copyArea->rect().x()>= drawZone->getImageRect().width() ||
      copyArea->rect().y()>= drawZone->getImageRect().height())
      copyArea->moveTo(0,0);

  if (copyArea->rect().width()>drawZone->getImageRect().width() ||
      copyArea->rect().height()>drawZone->getImageRect().height())
      return;

  AreaSelection *a=static_cast< AreaSelection* > (copyArea->clone());
  commandHistory()->push(new PasteCommand(this,*a));
  delete a;
//	addAreaAndEdit(a);
}



void KImageMapEditor::slotBackOne()
{
  if (currentSelected->isEmpty())
    return;

  AreaList list = currentSelected->getAreaList();


  Area *a = nullptr;
  // move every selected Area one step lower
  for (int i=areas->count()-2; i > -1; i--)
  {
    if (list.contains( areas->at(i) ))
    {
      uint j = (uint)i+1;
      a = areas->at(i);
      areas->removeAll(a);
      areas->insert(j,a);
      QTreeWidgetItem* root = areaListView->listView->invisibleRootItem();
      root->insertChild(j,root->takeChild(i));
    }
  }
  // to update the up and down buttons
  updateUpDownBtn();

}

void KImageMapEditor::slotForwardOne()
{
  if (currentSelected->isEmpty())
    return;

  AreaList list = currentSelected->getAreaList();

  Area *a = nullptr;
  // move every selected Area one step higher
  for (int i=1; i < (int)areas->count(); i++)
  {
    if (list.contains( areas->at(i) ))
    {
      uint j = (uint) i-1;
      a = areas->at(i);
      areas->removeAll(a);
      areas->insert(j,a);
      QTreeWidgetItem* root = areaListView->listView->invisibleRootItem();
      root->insertChild(j,root->takeChild(i));
    }
  }
  // to update the up and down buttons
  updateUpDownBtn();
}

void KImageMapEditor::slotToBack()
{
  if (currentSelected->isEmpty())
    return;

  while (backOneAction->isEnabled())
    slotBackOne();
}

void KImageMapEditor::slotToFront()
{
  if (currentSelected->isEmpty())
    return;

  while (forwardOneAction->isEnabled())
    slotForwardOne();
}


void KImageMapEditor::slotMoveUp()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);
  selected()->moveBy(0,-1);

  commandHistory()->push(
    new MoveCommand( this, selected(), r.topLeft() ));
  selected()->setMoving(false);
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveDown()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);
  selected()->moveBy(0,1);

  commandHistory()->push(
    new MoveCommand( this, selected(), r.topLeft() ));
  selected()->setMoving(false);
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveLeft()
{
  qCDebug(KIMAGEMAPEDITOR_LOG) << "slotMoveLeft";
  QRect r=selected()->rect();
  selected()->setMoving(true);
  selected()->moveBy(-1,0);

  commandHistory()->push(
    new MoveCommand( this, selected(), r.topLeft() ));
  selected()->setMoving(false);
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveRight()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);
  selected()->moveBy(1,0);

  commandHistory()->push(
    new MoveCommand( this, selected(), r.topLeft() ));
  selected()->setMoving(false);
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotCancelDrawing()
{
  drawZone->cancelDrawing();
}

void KImageMapEditor::slotIncreaseHeight()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setHeight( r.height()+1 );
  r.translate(0,-1);

  selected()->setRect(r);

  commandHistory()->push(
    new ResizeCommand( this, selected(), oldArea ));
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotDecreaseHeight()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setHeight( r.height()-1 );
  r.translate(0,1);

  selected()->setRect(r);

  commandHistory()->push(
    new ResizeCommand( this, selected(), oldArea ));
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotIncreaseWidth()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setWidth( r.width()+1 );

  selected()->setRect(r);

  commandHistory()->push(
    new ResizeCommand( this, selected(), oldArea ));
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotDecreaseWidth()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setWidth( r.width()-1 );

  selected()->setRect(r);

  commandHistory()->push(
    new ResizeCommand( this, selected(), oldArea ));
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotHighlightAreas(bool b)
{
  Area::highlightArea = b;
  updateAllAreas();
  drawZone->repaint();
}

void KImageMapEditor::slotShowAltTag(bool b)
{
  Area::showAlt = b;
  drawZone->repaint();
}

void KImageMapEditor::mapNew()
{
    QString mapName = mapsListView->getUnusedMapName();
    addMap(mapName);
    mapEditName();
}

void KImageMapEditor::mapDelete()
{
  if (mapsListView->count() == 0)
     return;

  QString selectedMap = mapsListView->selectedMap();

  int result = KMessageBox::warningContinueCancel(widget(),
    i18n("<qt>Are you sure you want to delete the map <i>%1</i>?"
         " <br /><b>There is no way to undo this.</b></qt>", selectedMap),
    i18n("Delete Map?"),KGuiItem(i18n("&Delete"),"edit-delete"));

  if (result == KMessageBox::Cancel)
     return;



  mapsListView->removeMap(selectedMap);
  HtmlMapElement* mapEl = findHtmlMapElement(selectedMap);
  _htmlContent.removeAll(mapEl);
  if (mapsListView->count() == 0) {

      currentMapElement = nullptr;
      deleteAllAreas();
      setMapActionsEnabled(false);
  }
  else {
      // The old one was deleted, so the new one got selected
      setMap(mapsListView->selectedMap());
  }
}

void KImageMapEditor::mapPreview() {
  HTMLPreviewDialog dialog(widget(), getHtmlCode());
  dialog.exec();
}

void KImageMapEditor::deleteAllMaps()
{
  deleteAllAreas();
  mapsListView->clear();
  if (isReadWrite()) {
    mapDeleteAction->setEnabled(false);
    mapDefaultAreaAction->setEnabled(false);
    mapNameAction->setEnabled(false);
  }
}

/**
 * Doesn't call the closeUrl method, because
 * we need the URL for the session management
 */
bool KImageMapEditor::queryClose() {
  if ( ! isModified() )
     return true;

  switch ( KMessageBox::warningYesNoCancel(
              widget(),
	      i18n("<qt>The file <i>%1</i> has been modified.<br />Do you want to save it?</qt>",
	      url().fileName()),
	      QString(),
	      KStandardGuiItem::save(),
	      KStandardGuiItem::discard()) )
    {
    case KMessageBox::Yes :
      saveFile();
      return true;
    case KMessageBox::No :
      return true;
    default:
      return false;
  }
}

bool KImageMapEditor::closeUrl()
{
  bool result = KParts::ReadWritePart::closeUrl();
  if (!result)
     return false;

  _htmlContent.clear();
  deleteAllMaps();
  imagesListView->clear();

  delete copyArea;
  copyArea = nullptr;

  delete defaultArea;
  defaultArea = nullptr;

  currentMapElement = nullptr;

  init();
  emit setWindowCaption("");

  return true;

}

void KImageMapEditor::addImage(const QUrl & imgUrl) {
    if (imgUrl.isEmpty())
        return;

    QString relativePath ( toRelative(imgUrl, QUrl( url().adjusted(QUrl::RemoveFilename).path() )).path() );

    QString imgHtml = QString("<img src=\"")+relativePath+QString("\">");
    ImageTag* imgTag = new ImageTag();
    imgTag->insert("tagname","img");
    imgTag->insert("src", relativePath);

    HtmlImgElement* imgEl = new HtmlImgElement(imgHtml);
    imgEl->imgTag = imgTag;

    HtmlElement* bodyEl = findHtmlElement("<body");
    if (bodyEl) {
        int bodyIndex = _htmlContent.indexOf(bodyEl);
        _htmlContent.insert(bodyIndex+1, new HtmlElement("\n"));
        _htmlContent.insert(bodyIndex+2, imgEl);
    }
    else {
        _htmlContent.append(new HtmlElement("\n"));
        _htmlContent.append(imgEl);
    }

    imagesListView->addImage(imgTag);
    imagesListView->selectImage(imgTag);
    setImageActionsEnabled(true);

    setModified(true);
}

/**
 * Sets whether the image actions that depend on an
 * selected image are enabled
 */
void KImageMapEditor::setImageActionsEnabled(bool b) {
  imageRemoveAction->setEnabled(b);
  imageUsemapAction->setEnabled(b);
}


void KImageMapEditor::imageAdd() {
    QUrl imgUrl = QFileDialog::getOpenFileUrl(widget(), i18n("Select image"),
                  QUrl(), i18n("Images (*.png *.jpg *.jpeg *.gif *.bmp *.xbm *.xpm *.pnm *.mng);;All Files (*)"));
    addImage(imgUrl);
}

void KImageMapEditor::imageRemove() {
    ImageTag* imgTag = imagesListView->selectedImage();
    HtmlImgElement* imgEl = findHtmlImgElement(imgTag);
    imagesListView->removeImage(imgTag);
    _htmlContent.removeAt(_htmlContent.indexOf(imgEl));

    if (imagesListView->topLevelItemCount() == 0) {
        setPicture(getBackgroundImage());
        setImageActionsEnabled(false);
    }
    else {
      ImageTag* selected = imagesListView->selectedImage();
      if (selected) {
	if (selected->contains("src")) {
	  setPicture(QUrl(selected->value("src")));
	}
      }
    }

    setModified(true);
}

void KImageMapEditor::imageUsemap() {

  bool ok=false;
  ImageTag* imageTag = imagesListView->selectedImage();
  if ( ! imageTag)
     return;

  QString usemap;

  if (imageTag->contains("usemap"))
      usemap = imageTag->value("usemap");

  QStringList maps = mapsListView->getMaps();
  int index = maps.indexOf(usemap);
  if (index == -1) {
    maps.prepend("");
    index = 0;
  }

  QString input =
    QInputDialog::getItem(widget(), i18n("Enter Usemap"),
			  i18n("Enter the usemap value:"),
			  maps, index, true, &ok);
  if (ok) {
     imageTag->insert("usemap", input);
     imagesListView->updateImage(imageTag);
     setModified(true);

     // Update the htmlCode of the HtmlElement
     HtmlImgElement* imgEl = findHtmlImgElement(imageTag);

     imgEl->htmlCode = QLatin1String("<");
     QString tagName = imgEl->imgTag->value("tagname");
     imgEl->htmlCode += QString(tagName);
     QHashIterator<QString, QString> it( *imgEl->imgTag );
     while (it.hasNext()) {
       it.next();
       if (it.key() != "tagname") {
           imgEl->htmlCode += " " + it.key() + "=\"";
           if (it.key() == "usemap")
               imgEl->htmlCode += '#';
           imgEl->htmlCode += it.value();
           imgEl->htmlCode += '"';
       }
     }

     imgEl->htmlCode += '>';

  }
}

#include "kimagemapeditor.moc"
