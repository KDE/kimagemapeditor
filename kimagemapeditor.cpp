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

#include <iostream>
#include <assert.h>

// QT
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qsplitter.h>
#include <qfileinfo.h>
#include <qmultilineedit.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qdict.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qfontdatabase.h>

// KDE
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kaccel.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <klineeditdlg.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kxmlguifactory.h>
#include <kdockwidget.h>
#include <kio/job.h>
#include <kinputdialog.h>

// local
#include "kimagemapeditor.h"
#include "kimagemapeditor.moc"
#include "drawzone.h"
#include "kimedialogs.h"
#include "kimecommands.h"
#include "qextfileinfo.h"
#include "areacreator.h"
#include "arealistview.h"
#include "imageslistview.h"
#include "mapslistview.h"
#include "kimecommon.h"

#include <kparts/genericfactory.h>
#include <kinstance.h>

// Factory code for KDE 3
typedef KParts::GenericFactory<KImageMapEditor> KimeFactory;
K_EXPORT_COMPONENT_FACTORY( libkimagemapeditor , KimeFactory );

KImageMapEditor::KImageMapEditor(QWidget *parentWidget, const char *,
            QObject *parent, const char *name, const QStringList & )
  : KParts::ReadWritePart(parent,name)
{
  setInstance( KimeFactory::instance() );

//  KDockMainWindow* mainWidget;
  
  // Test if the MainWindow can handle DockWindows, if so create DockWidgets
  // instead of a Splitter 
  mainDock = dynamic_cast<KDockMainWindow*>(parent) ;
  QSplitter * splitter = 0L;

  if (mainDock) {
//    kdDebug() << "KImageMapEditor: We got a KDockMainWindow !" << endl;
    KDockWidget* parentDock = mainDock->getMainDockWidget();
    areaDock = mainDock->createDockWidget( "Areas", 0L, 0L, i18n("Areas"));
    mapsDock = mainDock->createDockWidget( "Maps", 0L, 0L, i18n("Maps"));
    imagesDock = mainDock->createDockWidget( "Images", 0L, 0L, i18n("Images"));
    
    areaListView = new AreaListView(areaDock,"AreaListView");
    mapsListView = new MapsListView(mapsDock, "MapsListView");
    imagesListView = new ImagesListView(imagesDock, "ImagesListView");

    areaDock->setWidget(areaListView);
    mapsDock->setWidget(mapsListView);
    imagesDock->setWidget(imagesListView);
    
    areaDock->manualDock( (KDockWidget*) parentDock, KDockWidget::DockLeft, 30);
    mapsDock->manualDock( (KDockWidget*) areaDock, KDockWidget::DockCenter);
    imagesDock->manualDock( (KDockWidget*) mapsDock, KDockWidget::DockCenter);
    
    connect( mainDock->manager(), SIGNAL(change()), this, SLOT(dockingStateChanged()));
  } 
  else 
  {  
    areaDock = 0L;
    mapsDock = 0L;
    imagesDock = 0L;
    splitter = new QSplitter(parentWidget);
    tabWidget = new QTabWidget(splitter);    
    areaListView = new AreaListView(tabWidget,"AreaListView");
    mapsListView = new MapsListView(tabWidget, "MapsListView");
    imagesListView = new ImagesListView(tabWidget, "ImagesListView");
  
    tabWidget->addTab(areaListView,i18n("Areas"));
    tabWidget->addTab(mapsListView,i18n("Maps"));    
    tabWidget->addTab(imagesListView,i18n("Images"));
  }    
  
  
  connect( areaListView->listView, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
  connect( areaListView->listView, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(showTagEditor(QListViewItem*)));
  connect( areaListView->listView, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint &,int)), this,
           SLOT(slotShowPopupMenu(QListViewItem*,const QPoint &)));
  
  connect( mapsListView, SIGNAL( mapSelected(const QString &)),
           this, SLOT( setMap(const QString &)));
  
  connect( mapsListView, SIGNAL( mapRenamed(const QString &)),
           this, SLOT( setMapName(const QString &)));

  connect( mapsListView->listView(), SIGNAL(rightButtonPressed(QListViewItem*,const QPoint &,int)), this,
           SLOT(slotShowMapPopupMenu(QListViewItem*,const QPoint &)));
             
  connect( imagesListView, SIGNAL( imageSelected(const KURL &)),
           this, SLOT( setPicture(const KURL &)));
  
  connect( imagesListView, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint &,int)), this,
           SLOT(slotShowImagePopupMenu(QListViewItem*,const QPoint &)));
  
  // Shows the text:
  // "Drop an image or html file"
/*  QString path = KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/dropimage.png" ) + "kimagemapeditor/dropimage.png";
  if ( ! QFileInfo(path).exists() ) {
      kdError() << "Couldn't find needed dropimage.png file in "
                   "the data directory of KImageMapEditor.\n"
                   "Perhaps you have forgotten to do a make install !" << endl;
      exit(1);                   
  }
*/  

  if (splitter) {
    drawZone = new DrawZone(splitter,this);
    splitter->setResizeMode(drawZone,QSplitter::Stretch);
    splitter->setResizeMode(tabWidget,QSplitter::KeepSize);
    setWidget(splitter);
  } else {
    drawZone = new DrawZone(parentWidget,this);
    setWidget(drawZone);
  }


  areas = new AreaList();
  currentSelected= new AreaSelection();
  _currentToolType=KImageMapEditor::Selection;
  copyArea=0L;
  defaultArea=0L;
  currentMapElement = 0L;
  
  setupActions();
  setupStatusBar();

  setXMLFile("kimagemapeditorpartui.rc");

  setPicture(getBackgroundImage());
  _htmlContent.setAutoDelete(true);
  
  init();
  readConfig();
}

KImageMapEditor::~KImageMapEditor() {
  writeConfig();

  #if KDE_VERSION < 300
    delete accel;
  #endif 
  delete areas;
  
  delete currentSelected;
  if (copyArea)
     delete copyArea;
  if (defaultArea)
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

MapTag::MapTag() {
  modified = false;
  name = "";
}

void KImageMapEditor::init()
{
  _htmlContent.clear();
  _imageUrl = "";
  m_url = "";
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

KAboutData* KImageMapEditor::createAboutData()
{
    KAboutData* aboutData =
              new KAboutData( "kimagemapeditor", I18N_NOOP("KImageMapEditor"),
              "1.0", I18N_NOOP( "An HTML imagemap editor" ),
              KAboutData::License_GPL,
              "(c) 2001-2003 Jan Sch&auml;fer <janschaefer@users.sourceforge.net>");
    return aboutData;
}


void KImageMapEditor::setReadWrite(bool)
{
  
  // For now it doesn't matter if its readwrite or readonly
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
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
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
    return KimeFactory::instance()->config();
}

void KImageMapEditor::readConfig(KConfig* config) {
  recentFilesAction->loadEntries(config,"Data");
}

void KImageMapEditor::writeConfig(KConfig* config) {
  config->writeEntry("highlightareas",highlightAreasAction->isChecked());
  config->writeEntry("showalt",showAltAction->isChecked());
	recentFilesAction->saveEntries(config,"Data");
  saveLastURL(config);
  
}

void KImageMapEditor::readConfig() {
  config()->setGroup("General Options");
  readConfig(config());
  slotConfigChanged();
}

void KImageMapEditor::writeConfig() {
  config()->setGroup("General Options");
  writeConfig(config());
  config()->sync();
}


void KImageMapEditor::saveProperties(KConfig *config)
{
  saveLastURL(config);
}

void KImageMapEditor::readProperties(KConfig * config)
{
  openLastURL(config);
}

void KImageMapEditor::slotConfigChanged()
{
  config()->setGroup("Appearance");
  int newHeight=config()->readNumEntry("maximum-preview-height",50);
  config()->setGroup("General Options");
  _commandHistory->setUndoLimit(config()->readNumEntry("undo-level",20));
  _commandHistory->setRedoLimit(config()->readNumEntry("redo-level",20));
  Area::highlightArea = config()->readBoolEntry("highlightareas",true);
  highlightAreasAction->setChecked(Area::highlightArea);
  Area::showAlt = config()->readBoolEntry("showalt",true);
  showAltAction->setChecked(Area::showAlt);

  // if the image preview size changed update all images
  if (maxAreaPreviewHeight!=newHeight) {
    maxAreaPreviewHeight=newHeight;
  }

  updateAllAreas();
  drawZone->viewport()->repaint();
}

void KImageMapEditor::openLastURL(KConfig* config) {
  KURL lastURL = config->readEntry("lastopenurl");
  QString lastMap = config->readEntry("lastactivemap");
  QString lastImage = config->readEntry("lastactiveimage");
  
  
//  kdDebug() << "loading from group : " << config->group() << endl;
  
//  kdDebug() << "loading entry lastopenurl : " << lastURL.path() << endl;
//  KMessageBox::information(0L, config->group()+" "+lastURL.path());
  
  if (!lastURL.isEmpty()) {
//    kdDebug() << "opening HTML file with map " << lastMap << " and image " << lastImage << endl;
    openHTMLFile(lastURL, lastMap, lastImage);
    m_url = lastURL;
    
  }
}

void KImageMapEditor::saveLastURL(KConfig* config) {
  config->writeEntry("lastopenurl",url().path());
  config->writeEntry("lastactivemap",mapName());
  config->writeEntry("lastactiveimage",_imageUrl.path());
//  kdDebug() << "writing entry lastopenurl : " << url().path() << endl;
//  kdDebug() << "writing entry lastactivemap : " << mapName() << endl;
//  kdDebug() << "writing entry lastactiveimage : " << _imageUrl.path() << endl;
  //KMessageBox::information(0L, QString("Group: %1 Saving ... %2").arg(config->group()).arg(url().path()));
}

void KImageMapEditor::setupActions()
{
	// File Open
  KAction *temp=KStdAction::open(this, SLOT(fileOpen()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "openimage", SmallIcon("fileopen") );
	temp->setWhatsThis(i18n("<h3>Open File</h3>Click this to <em>open</em> a new picture or html file."));
	temp->setToolTip(i18n("Open new picture or html file"));

	// File Open Recent
  recentFilesAction = KStdAction::openRecent(this, SLOT(openURL(const KURL&)),
                                      actionCollection());
	// File Save
  temp =KStdAction::save(this, SLOT(fileSave()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "saveimage", SmallIcon("filesave") );
	temp->setWhatsThis(i18n("<h3>Save File</h3>Click this to <em>save</em> the changes to the html file."));
	temp->setToolTip(i18n("Save html file"));


	// File Save As
  (void)KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());

	// File Close
  temp=KStdAction::close(this, SLOT(fileClose()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "closeimage", SmallIcon("fileclose") );
	temp->setWhatsThis(i18n("<h3>Close File</h3>Click this to <em>close</em> the currently open html file."));
	temp->setToolTip(i18n("Close html file"));

  // Edit Copy
  copyAction=KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "editcopyimage", SmallIcon("editcopy") );
  copyAction->setWhatsThis(i18n("<h3>Copy</h3>"
                          "Click this to <em>copy</em> the selected area."));
  copyAction->setEnabled(false);													

  // Edit Cut
  cutAction=KStdAction::cut(this, SLOT(slotCut()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "editcutimage", SmallIcon("editcut") );
  cutAction->setWhatsThis(i18n("<h3>Cut</h3>"
                          "Click this to <em>cut</em> the selected area."));
  cutAction->setEnabled(false);													

  // Edit Paste													
  pasteAction=KStdAction::paste(this, SLOT(slotPaste()), actionCollection());
  QMimeSourceFactory::defaultFactory()->setPixmap( "editpasteimage", SmallIcon("editpaste") );
  pasteAction->setWhatsThis(i18n("<h3>Paste</h3>"
                          "Click this to <em>paste</em> the copied area."));
  pasteAction->setEnabled(false);


  // Edit Delete
  deleteAction=new KAction(i18n("&Delete"), "editdelete",
              Key_Delete,this,SLOT (slotDelete()),actionCollection(), "edit_delete");
  QMimeSourceFactory::defaultFactory()->setPixmap( "editdeleteimage", SmallIcon("editdelete") );
  deleteAction->setWhatsThis(i18n("<h3>Delete</h3>"
                          "Click this to <em>delete</em> the selected area."));
  deleteAction->setEnabled(false);													

  // Edit Undo/Redo
  _commandHistory = new KCommandHistory( actionCollection(), true);

  // Edit Properties
  areaPropertiesAction= new KAction(i18n("Pr&operties"),0,this,SLOT(showTagEditor()),
    actionCollection(), "edit_properties");
  areaPropertiesAction->setEnabled(false);		

  // View Zoom In													
  zoomInAction=KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection());
  // View Zoom Out													
  zoomOutAction=KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection());

  // View Zoom
  zoomAction=new KSelectAction(i18n("Zoom"), 0,this,SLOT (slotZoom()),
      actionCollection(), "view_zoom");
  zoomAction->setWhatsThis(i18n("<h3>Zoom</h3>"
                          "Choose the desired zoom level."));
  zoomAction->setItems(QStringList()
    << i18n("25 %")
    << i18n("50 %")
    << i18n("100 %")
    << i18n("150 %")
    << i18n("200 %")
    << i18n("250 %")
    << i18n("300 %")
    << i18n("500 %")
    << i18n("750 %")
    << i18n("1000 %"));

  zoomAction->setCurrentItem(2);

  highlightAreasAction = new KToggleAction(i18n("Highlight Areas"),0, this, SLOT (slotHightlightAreas()),
      actionCollection(), "view_highlightareas");

  showAltAction = new KToggleAction(i18n("Show Alt Tag"),0, this, SLOT (slotShowAltTag()),
      actionCollection(), "view_showalt");


  mapNameAction= new KAction(i18n("Map &Name"),0,this,SLOT(mapEditName()),
    actionCollection(), "map_name");

  mapNewAction = new KAction(i18n("Ne&w Map"),0,this,SLOT(mapNew()),
    actionCollection(), "map_new");
  mapNewAction->setToolTip(i18n("Create a new map"));

  mapDeleteAction = new KAction(i18n("D&elete Map"),0,this,SLOT(mapDelete()),
    actionCollection(), "map_delete");
  mapDeleteAction->setToolTip(i18n("Delete the current active map"));

  mapDefaultAreaAction = new KAction(i18n("Edit &Default Area"),0,this,SLOT(mapDefaultArea()),
    actionCollection(), "map_defaultarea");
  mapDefaultAreaAction->setToolTip(i18n("Edit the default area of the current active map"));

  temp = new KAction(i18n("&Preview"),0,this,SLOT(mapPreview()),
    actionCollection(), "map_preview");
  temp->setToolTip(i18n("Show a preview"));

  // IMAGE
  i18n("&Image");

  imageAddAction = new KAction(i18n("Add Image"),0,this,SLOT(imageAdd()),
    actionCollection(), "image_add");
  imageAddAction->setToolTip(i18n("Add a new image"));    

  imageRemoveAction = new KAction(i18n("Remove Image"),0,this,SLOT(imageRemove()),
    actionCollection(), "image_remove");
  imageRemoveAction->setToolTip(i18n("Remove the current visible image"));    
  
  imageUsemapAction = new KAction(i18n("Edit Usemap"),0,this,SLOT(imageUsemap()),
    actionCollection(), "image_usemap");
  imageUsemapAction->setToolTip(i18n("Edit the usemap tag of the current visible image"));    
  
  temp= new KAction(i18n("Show &HTML"),0,this,SLOT(mapShowHTML()),
    actionCollection(), "map_showhtml");


  // Selection Tool
  arrowAction=new KRadioAction(i18n("&Selection"), "arrow",
            0,this,SLOT (slotDrawArrow()),
      actionCollection(), "tool_arrow");
  QMimeSourceFactory::defaultFactory()->setPixmap( "arrowimage", SmallIcon("arrow") );
  arrowAction->setWhatsThis(i18n("<h3>Selection</h3>"
                          "Click this to select areas."));
  arrowAction->setExclusiveGroup("drawing");
  arrowAction->setChecked(true);

  // Circle
  circleAction=new KRadioAction(i18n("&Circle"), "circle",
            0,this,SLOT (slotDrawCircle()),
      actionCollection(), "tool_circle");
  QMimeSourceFactory::defaultFactory()->setPixmap( "circleimage", SmallIcon("drawcircle") );
  circleAction->setWhatsThis(i18n("<h3>Circle</h3>"
                          "Click this to start drawing a circle."));
  circleAction->setExclusiveGroup("drawing");

  // Rectangle
  rectangleAction=new KRadioAction(i18n("&Rectangle"), "rectangle",
            0,this,SLOT (slotDrawRectangle()),
      actionCollection(), "tool_rectangle");
  QMimeSourceFactory::defaultFactory()->setPixmap( "rectangleimage", SmallIcon("drawrectangle") );
  rectangleAction->setWhatsThis(i18n("<h3>Rectangle</h3>"
                          "Click this to start drawing a rectangle."));
  rectangleAction->setExclusiveGroup("drawing");

  // Polygon
  polygonAction=new KRadioAction(i18n("&Polygon"), "polygon",
            0,this,SLOT (slotDrawPolygon()),
      actionCollection(), "tool_polygon");
  QMimeSourceFactory::defaultFactory()->setPixmap( "polygonimage", SmallIcon("drawpolygon") );
  polygonAction->setWhatsThis(i18n("<h3>Polygon</h3>"
                          "Click this to start drawing a polygon."));
  polygonAction->setExclusiveGroup("drawing");

  // Freehand
  freehandAction=new KRadioAction(i18n("&Freehandpolygon"), "freehand",
            0,this,SLOT (slotDrawFreehand()),
      actionCollection(), "tool_freehand");
  QMimeSourceFactory::defaultFactory()->setPixmap( "freehandimage", SmallIcon("freehand") );
  freehandAction->setWhatsThis(i18n("<h3>Freehandpolygon</h3>"
                          "Click this to start drawing a freehand polygon."));
  freehandAction->setExclusiveGroup("drawing");

  // Add Point
  addPointAction=new KRadioAction(i18n("&Add Point"), "addpoint",
            0,this,SLOT (slotDrawAddPoint()),
      actionCollection(), "tool_addpoint");
  QMimeSourceFactory::defaultFactory()->setPixmap( "addpointimage", SmallIcon("addpoint") );
  addPointAction->setWhatsThis(i18n("<h3>Add Point</h3>"
                          "Click this to add points to a polygon."));
  addPointAction->setExclusiveGroup("drawing");

  // Remove Point
  removePointAction=new KRadioAction(i18n("&Remove Point"), "removepoint",
            0,this,SLOT (slotDrawRemovePoint()),
      actionCollection(), "tool_removepoint");
  QMimeSourceFactory::defaultFactory()->setPixmap( "removepointimage", SmallIcon("removepoint") );
  removePointAction->setWhatsThis(i18n("<h3>Remove Point</h3>"
                          "Click this to remove points from a polygon."));
  removePointAction->setExclusiveGroup("drawing");

  #if KDE_VERSION < 300
    KAction *cancelAction = 
  #endif    
      new KAction(i18n("Cancel Drawing"), Key_Escape, this, SLOT( slotCancelDrawing() ),
                              actionCollection(), "canceldrawing" );

  moveLeftAction = new KAction(i18n("Move Left"), Key_Left, this, SLOT( slotMoveLeft() ),
                              actionCollection() , "moveleft" );

  moveRightAction = new KAction(i18n("Move Right"), Key_Right, this, SLOT( slotMoveRight() ),
                              actionCollection() , "moveright" );

  moveUpAction = new KAction(i18n("Move Up"), Key_Up, this, SLOT( slotMoveUp() ),
                              actionCollection() , "moveup" );

  moveDownAction = new KAction(i18n("Move Down"), Key_Down, this, SLOT( slotMoveDown() ),
                              actionCollection() , "movedown" );

  increaseWidthAction = new KAction(i18n("Increase Width"), Key_Right + SHIFT, this, SLOT( slotIncreaseWidth() ),
                              actionCollection() , "increasewidth" );

  decreaseWidthAction = new KAction(i18n("Decrease Width"), Key_Left + SHIFT, this, SLOT( slotDecreaseWidth() ),
                              actionCollection() , "decreasewidth" );

  increaseHeightAction = new KAction(i18n("Increase Height"), Key_Up + SHIFT, this, SLOT( slotIncreaseHeight() ),
                              actionCollection() , "increaseheight" );

  decreaseHeightAction = new KAction(i18n("Decrease Height"), Key_Down + SHIFT, this, SLOT( slotDecreaseHeight() ),
                              actionCollection() , "decreaseheight" );
  #if KDE_VERSION < 300                              
    accel = new KAccel(widget());
    cancelAction->plugAccel(accel, true);
    moveLeftAction->plugAccel(accel, true);
    moveRightAction->plugAccel(accel, true);
    moveUpAction->plugAccel(accel, true);
    moveDownAction->plugAccel(accel, true);
    increaseWidthAction->plugAccel(accel, true);
    decreaseWidthAction->plugAccel(accel, true);
    increaseHeightAction->plugAccel(accel, true);
    decreaseHeightAction->plugAccel(accel, true);
  #endif

  toFrontAction = new KAction(i18n("Bring to front"), 0 , this, SLOT( slotToFront() ),
                              actionCollection() , "tofront" );

  toBackAction  = new KAction(i18n("To Back"), 0 , this, SLOT( slotToBack() ),
                              actionCollection() , "toback" );

  forwardOneAction  = new KAction(i18n("Forward One"), "raise" ,0, this, SLOT( slotForwardOne() ),
                              actionCollection() , "forwardone" );
  backOneAction = new KAction(i18n("Back One"), "lower" ,0, this, SLOT( slotBackOne() ),
                              actionCollection() , "backone" );

  forwardOneAction->plug(areaListView->upBtn);
  backOneAction->plug(areaListView->downBtn);			

  connect( areaListView->upBtn, SIGNAL(pressed()), forwardOneAction, SLOT(activate()));												
  connect( areaListView->downBtn, SIGNAL(pressed()), backOneAction, SLOT(activate()));

  new KAction( i18n("Configure KImageMapEditor..."), "configure", 0,
                        this, SLOT(slotShowPreferences()),
                        actionCollection(), "configure_kimagemapeditor" );

  if (areaDock) {                        
    configureShowAreaListAction = new KToggleAction( i18n("Show Area List"), 0L, 0,
                        this, SLOT(configureShowAreaList()),
                        actionCollection(), "configure_show_arealist" );
                          
    configureShowMapListAction = new KToggleAction( i18n("Show Map List"), 0L, 0,
                        this, SLOT(configureShowMapList()),
                        actionCollection(), "configure_show_maplist" );

    configureShowImageListAction = new KToggleAction( i18n("Show Image List"), 0L, 0,
                        this, SLOT(configureShowImageList()),
                        actionCollection(), "configure_show_imagelist" );
  }                                                                        
                        
  updateActionAccess();
}

void KImageMapEditor::setupStatusBar()
{

//  We can't do this with a KPart !
//	widget()->statusBar()->insertItem(i18n(" Cursor")+" : x: 0 ,y: 0",STATUS_CURSOR);
//	widget()->statusBar()->insertItem(i18n(" Selection")+" : - ",STATUS_SELECTION);
  emit setStatusBarText( i18n(" Cursor")+" : x: 0 ,y: 0" + "  " + i18n(" Selection")+" : - ");
}

void KImageMapEditor::slotShowPreferences()
{
  PreferencesDialog *dialog = new PreferencesDialog(widget(),config());
  connect(dialog, SIGNAL(applyClicked()), this, SLOT(slotConfigChanged()));
  dialog->exec();
  delete dialog;
}


void KImageMapEditor::showPopupMenu(const QPoint & pos, const QString & name)
{
  QPopupMenu* pop = static_cast<QPopupMenu *>(factory()->container(name, this));

  if (!pop) {
      kdWarning() << QString("KImageMapEditorPart: Missing XML definition for %1\n").arg(name) << endl;
      return;
  }

  pop->popup(pos);
}

void KImageMapEditor::slotShowMainPopupMenu(const QPoint & pos)
{
  showPopupMenu(pos,"popup_main");
}

void KImageMapEditor::slotShowMapPopupMenu(QListViewItem* item,const QPoint & pos)
{
  if (isReadWrite()) {
    mapDeleteAction->setEnabled(item);
    mapNameAction->setEnabled(item);
    mapDefaultAreaAction->setEnabled(item);
  }

  if (item)
     mapsListView->selectMap(item);     
          
  showPopupMenu(pos,"popup_map");
}

void KImageMapEditor::slotShowImagePopupMenu(QListViewItem* item,const QPoint & pos)
{
  imageRemoveAction->setEnabled(item);
  imageUsemapAction->setEnabled(item);

  if (item)
     imagesListView->setSelected(item,true);     
          
  showPopupMenu(pos,"popup_image");
}

void KImageMapEditor::slotShowPopupMenu(QListViewItem* item,const QPoint & p)
{
  if (!item)
    return;

  if (!item->isSelected())
  {
    deselectAll();
    select(item);
  }

  slotShowMainPopupMenu(p);
}

void KImageMapEditor::updateStatusBar()
{
  emit setStatusBarText(selectionStatusText+"  "+cursorStatusText);
}

void KImageMapEditor::slotChangeStatusCoords(int x,int y) 
{
//	statusBar()->changeItem(QString(" Cursor : x: %1 ,y: %2 ").arg(x).arg(y),STATUS_CURSOR);
  cursorStatusText = i18n(" Cursor : x: %1 ,y: %2 ").arg(x).arg(y);
  updateStatusBar();
}

void KImageMapEditor::slotUpdateSelectionCoords() {
  if (selected()->count()>0) {
    QRect r=selected()->rect();
//		statusBar()->changeItem(
    selectionStatusText = i18n(" Selection : x: %1 ,y: %2, w: %3, h: %4 ").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height());

//		  ,STATUS_SELECTION);
    kapp->processEvents();
  } else
    selectionStatusText = i18n(" Selection : - ");
    //statusBar()->changeItem(" Selection : - ",STATUS_SELECTION);

  updateStatusBar();    
}

void KImageMapEditor::slotUpdateSelectionCoords( const QRect & r )
{
  selectionStatusText = i18n(" Selection : x: %1 ,y: %2, w: %3, h: %4 ").arg(r.left()).arg(r.top()).arg(r.width()).arg(r.height());
  updateStatusBar();
  kapp->processEvents();
}

KApplication* KImageMapEditor::app() const
{
  return kapp;
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
//  QString path = ""; //KGlobal::dirs()->findResourceDir( "data", "kimagemapeditor/"+filename ) + "kimagemapeditor/"+filename;
//  kdDebug() << "getBackgroundPic : loaded image : " << path << endl;
  
//  if ( ! QFileInfo(path).exists() ) {
    int width = 400;
    int height = 400;
    int border = 20;
    int fontSize = 58;
    
    QPixmap pix(width,height);
    pix.fill(QColor(74,76,74));
    QPainter p(&pix);
    
    QFont font = QFontDatabase().font("Luxi Sans","Bold",fontSize);
    p.setFont( font );
    p.setRasterOp(Qt::CopyROP);
    p.setPen(QPen(QColor(112,114,112),1));
    
    // The translated string must be divided into
    // parts with about the same size that fit to the image
    QString str = i18n("Drop an image or html file");
    QStringList strList = QStringList::split(" ",str);
    
    // Get the string parts
    QString tmp = "";
    QStringList outputStrList;
    QFontMetrics fm = p.fontMetrics();
    
    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it ) {
      QString tmp2 = tmp + *it;
      
        if (fm.boundingRect(tmp2).width() > width-border) {      
           outputStrList.append(tmp);
           tmp = *it + " ";
        }
        else
          tmp = tmp2 + " ";
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

    _backgroundImage = pix.convertToImage();
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
    
    
    path = KGlobal::dirs()->saveLocation( "data", "kimagemapeditor/" ) +filename;
    kdDebug() << "getBackgroundPic : save new image to : " << path << endl;
    pix.save(path,"PNG",100);
  } 

  if ( ! QFileInfo(path).exists() ) {
      kdError() << "Couldn't find needed " << filename << " file in "
                   "the data directory of KImageMapEditor.\n"
                   "Perhaps you have forgotten to do a make install !" << endl;
      exit(1);                   
  }
*/  
}


void KImageMapEditor::addArea(Area* area) {
  if (!area) return;

  // Perhaps we've got a selection of areas
  // so test it and add all areas of the selection
  // nested selections are possible but doesn't exist
  AreaSelection *selection=0L;
  if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaList list = selection->getAreaList();

    for (Area* a = list.first(); a != 0L; a = list.next() )
    {
      areas->prepend(a);
      a->setListViewItem(new QListViewItem(areaListView->listView,a->attribute("href")));
      a->listViewItem()->setPixmap(1,makeListViewPix(*a));
    }
  }
  else
  {
    areas->prepend(area);
    area->setListViewItem(new QListViewItem(areaListView->listView,area->attribute("href")));
    area->listViewItem()->setPixmap(1,makeListViewPix(*area));
  }

  setModified(true);

}

void KImageMapEditor::addAreaAndEdit(Area* s)
{
  areas->prepend(s);
  s->setListViewItem(new QListViewItem(areaListView->listView,s->attribute("href")));
  s->listViewItem()->setPixmap(1,makeListViewPix(*s));
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
  AreaSelection *selection=0L;
    if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaList list = selection->getAreaList();

    for (Area* a = list.first(); a != 0L; a = list.next() )
    {
      currentSelected->remove(a);
      areas->remove( a );
      a->deleteListViewItem();
    }
  }
  else
  {
    deselect( area );
    areas->remove( area );
    area->deleteListViewItem();
  }

  drawZone->repaintRect(redrawRect);


  // Only to disable cut and copy actions
  if (areas->count()==0)
    deselectAll();

  setModified(true);
}

void KImageMapEditor::deleteSelected() {

  Area *a;
  AreaList list=currentSelected->getAreaList();

  for ( a=list.first(); a != 0; a=list.next() )	{
    currentSelected->remove( a );
    areas->remove( a );  	
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
  for (Area* a=areas->first();a!=0L;)
  {
    deselect( a );
    areas->remove( a );  	
    a->deleteListViewItem();
    a=areas->first(); // because the current is deleted
  }

  drawZone->viewport()->repaint();

}

void KImageMapEditor::updateAllAreas()
{
//  kdDebug() << "KImageMapEditor::updateAllAreas" << endl;
  for (Area* a=areas->first();a!=0L;a=areas->next()) {
    a->listViewItem()->setPixmap(1,makeListViewPix(*a));
  }
  drawZone->viewport()->repaint();
}

void KImageMapEditor::updateSelection() const {
  areaListView->listView->triggerUpdate();
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

  for ( ; it.current() != 0L; ++it)
  {
    if ( it.current()->listViewItem()->isSelected() != (list.containsRef(it.current()) > 0) )
    {
      it.current()->listViewItem()->isSelected()
        ? select( it.current() )
        :	deselect( it.current() );

      drawZone->repaintArea( *it.current());
    }
  }

}

void KImageMapEditor::select( QListViewItem* item)
{

  AreaListIterator it = areaList();

  for ( ; it.current() != 0L; ++it)
  {
    if (it.current()->listViewItem() == item )
    {
      select( it.current() );
      drawZone->repaintArea( *it.current());
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

  AreaSelection *selection=0L;
  if ( (selection = dynamic_cast <AreaSelection*> ( area ) ) )
  {
    AreaListIterator it = selection->getAreaListIterator();

    for ( ; it.current() != 0L; ++it )
    {
      if (it.current()->listViewItem()) {
        it.current()->listViewItem()->setText(0,it.current()->attribute("href"));
        it.current()->listViewItem()->setPixmap(1,makeListViewPix(*it.current()));
      }
    }

  }
  else
  if (area->listViewItem()) {
    area->listViewItem()->setText(0,area->attribute("href"));
    area->listViewItem()->setPixmap(1,makeListViewPix(*area));
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

  if (list.isEmpty() || (areas->count() < 2))
  {
    forwardOneAction->setEnabled(false);													
    areaListView->upBtn->setEnabled(false);
    backOneAction->setEnabled(false);
    areaListView->downBtn->setEnabled(false);
    return; 	
  }
  // if the first Area is in the selection can't move up	
  if (list.find( areas->getFirst() ) == -1)
  {
    forwardOneAction->setEnabled(true);
    areaListView->upBtn->setEnabled(true);
  }
  else {
    forwardOneAction->setEnabled(false);													
    areaListView->upBtn->setEnabled(false);
  }

  drawZone->repaintArea(*currentSelected);

  // if the last Area is in the selection can't move down
  if (list.find( areas->getLast() ) == -1)
  {
    backOneAction->setEnabled(true);
    areaListView->downBtn->setEnabled(true);
  }
  else {
    backOneAction->setEnabled(false);
    areaListView->downBtn->setEnabled(false);
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
  for (Area* s=areas->first();s!=0L;s=areas->next()) {
    if (s->contains(p))
      return s;
  }
  return 0L;
}


int KImageMapEditor::showTagEditor(Area *a) {
  if (!a) return 0;
  drawZone->repaintArea(*a);

  AreaDialog *dialog= new AreaDialog(this,a);
  connect (dialog, SIGNAL(areaChanged(Area*)), this, SLOT(slotAreaChanged(Area*)));

  int result = dialog->exec();

  return result;


}

int KImageMapEditor::showTagEditor(QListViewItem *item) {
  if (!item) return 0;
  for (Area* a=areas->first();a!=0L;a=areas->next()) {
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

  for (Area* a=areas->first();a!=0L;a=areas->next()) {
    retStr+="  "+a->getHTMLCode()+"\n";
  }

  if (defaultArea && defaultArea->finished())
    retStr+="  "+defaultArea->getHTMLCode()+"\n";

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


void KImageMapEditor::setPicture(const KURL & url) {
  _imageUrl=url;
  if (QFileInfo(url.path()).exists()) {
     QImage img(url.path());

     if (!img.isNull()) {
         setPicture(img);
         imageRemoveAction->setEnabled(true);
         imageUsemapAction->setEnabled(true);
     }
     else
         kdError() << QString("The image %1 couldn't be opened !").arg(url.path()) << endl;
  }
  else
     kdError() << QString("The image %1 does not exist !").arg(url.path()) << endl;
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

}

void KImageMapEditor::slotDrawRectangle() {
  _currentToolType=KImageMapEditor::Rectangle;

}

void KImageMapEditor::slotDrawPolygon() {
  _currentToolType=KImageMapEditor::Polygon;
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
  QString input = KLineEditDlg::getText(i18n("Enter the name of the map"),_mapName,&ok,widget());
  if (ok) {
    if (input != _mapName) {
        if (mapsListView->nameAlreadyExists(input))
            KMessageBox::sorry(this->widget(), i18n("Sorry, the name <em>%1</em> already exists !").arg(input));
        else {
            setMapName(input);
        }
    }
  }
}

void KImageMapEditor::mapShowHTML()
{
  KDialogBase *dialog= new KDialogBase(widget(),"",true,i18n("HTML code of the map"),KDialogBase::Ok);
  QMultiLineEdit *edit = new QMultiLineEdit(dialog);

  edit->setText(getHtmlCode());
  edit->setReadOnly(true);
  edit->setWordWrap(QTextEdit::NoWrap);
  dialog->setMainWidget(edit);
//  dialog->resize(dialog->calculateSize(edit->maxLineWidth(),edit->numLines()*));
//	dialog->adjustSize();
  dialog->resize(600,400);
  dialog->exec();
}

void KImageMapEditor::openFile(const KURL & url) {
  if ( ! url.isEmpty()) {
    QString ext=QFileInfo(url.path()).extension().lower();
  
    if (ext=="png" || ext=="jpg" || ext=="jpeg" || ext=="gif" ||
        ext=="bmp" || ext=="xbm" || ext=="xpm" || ext=="mng" || ext=="pnm") 
        addImage(url);
    else        
        openURL(url);
  }
}

void KImageMapEditor::fileOpen() {

  QString fileName = KFileDialog::getOpenFileName(QString::null,
          "*.png *.jpg *.jpeg *.gif *.htm *.html|Web file\n"
          "*.png *.jpg *.jpeg *.gif *.bmp *.xbm *.xpm *.pnm *.mng|Image\n"
          "*.htm *.html|HTML-File\n"
          "*.png|PNG-Image\n*.jpg *.jpeg|JPEG-Image\n*.gif|GIF-Image\n*|All files"
          ,widget(),i18n("Choose a file to open"));
          
  openFile(fileName);          
}



void KImageMapEditor::fileClose()
{
  if (! closeURL())
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
  
  KURL url = KFileDialog::getSaveURL(0L,"*.htm *.html|"+i18n("HTML file")+"\n*.txt|"+
  						i18n("Text file")+"\n*|"+i18n("All Files"),widget());
  if (url.isEmpty() || !url.isValid()) {
    return;
  }


  QFileInfo fileInfo(url.path());

  if ( fileInfo.exists() )
  {
  	if (KMessageBox::warningYesNo(widget(),
      i18n("<qt>The file <em>%1</em> already exists!<br>Do you want to overwrite it?</qt>").arg(fileInfo.fileName()),
      i18n("Really overwrite the file ?"))==KMessageBox::No)
      return;
  
    if(!fileInfo.isWritable()) {
      KMessageBox::sorry(widget(), i18n("<qt>You do not have write permission for the file <em>%1/em>!</qt>").arg(fileInfo.fileName()));
      return;
    }
  }
  
  
  saveAs(url);
  recentFilesAction->addURL(url);
  
}


bool KImageMapEditor::openFile()
{
  QFileInfo fileInfo(url().path());

  if ( !fileInfo.exists() )
  {
      KMessageBox::information(widget(),
        i18n("<qt>The file <b>%1</b> does not exists!</qt>").arg(fileInfo.fileName()),
        i18n("File does not exists!"));
      return false;
  }

  openHTMLFile(url());

  drawZone->viewport()->repaint();
  recentFilesAction->addURL(url());
  setModified(false);
  backupFileCreated = false;
  return true;
}

/**
 * This methods supposes that the given QTextStream s has just read
 * the &lt; of a tag. It now reads all attributes of the tag until a &gt;
 * The tagname itself is also read and stored as a <em>tagname</em>
 * attribute. After parsing the whole tag it returns a QDict<QString>
 * with all attributes and their values. It stores the whole read text in the
 * parameter readText.
 */
QDict<QString> KImageMapEditor::getTagAttributes(QTextStream & s, QString & readText)
{
  QDict<QString> dict(17,false);
  // the "<" is already read
  QChar w;
  QString attr,value;

  readText = "";
  
  // get the tagname
  while (!s.atEnd() && w!=" ") {
    s >> w;
    readText.append(w);
    if (w==" " || w==">") {
      dict.insert("tagname",new QString(value));
      break;
    }
    value+=w;
  }

  
  // do we have a comment ?
  // read the comment and return
  if (value.right(3)=="-->")
    return dict;

  if (value.startsWith("!--")) {
    while (!s.atEnd()) {
      s >> w;
      readText.append(w);
      
      if (w=="-") {
        s >> w;
        readText.append(w);
        if (w=="-") {
          s >> w;
          readText.append(w);
          if (w==">")
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
  attr="";
  value="";

  //get the other attributes
  while (!s.atEnd() && w!=">")
  {
    s >> w;
    readText.append(w);

    // End of PHP Script ?
    if (php && (w=="?") )
    {
      s >> w;
      readText.append(w);
      
      if (valueRead)
          value+=w;

      if (w==">")
      {
        php = false;
        s >> w;
        readText.append(w);
      }
    }

    // Wrong syntax or PHP-Skript !
    if (!php && (w=="<"))
    {
      if (valueRead)
        value+=w;
      s >> w;
      readText.append(w);
      if (valueRead)
        value+=w;

      if (w=="?")
      {
        php = true;
      }
    } else
    // finished ?
    if (w==">") {
      if (valueRead)
        dict.insert(attr,new QString(value));
        return dict;
    } else
    // currently reading an attribute ?
    if (attrRead) {
      // if there is a whitespace the attributename has finished
      // possibly there isn't any value e.g. noshade
      if (w==" ")
        attrRead=false;
      else
      // an equal sign signals that the value follows
      if (w=="=") {
        attrRead=false;
        equalSign=true;
      } else
        attr+=w;
    } else
    // an equal sign was read ? delete every whitespace
    if (equalSign) {
      if (w!=" ") {
        equalSign=false;
        valueRead=true;
        if (w=="\"" || w=="'")
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
          dict.insert(attr,new QString(value));
          attr="";value="";

        }
      } else
      // a whitespace indicates that the value has finished
      if (w==" ") {
        valueRead=false;
        dict.insert(attr,new QString(value));
        attr="";value="";
      }
    } else {
      if (w!=" ") {
        attrRead=true;
        attr+=w;
      }
    }
  }

  return dict;

}


void KImageMapEditor::openHTMLFile(const KURL & url, const QString & mapName, const QString & imagePath)
{
  QFile f(url.path());
  f.open(IO_ReadOnly);
  QTextStream s(&f);
  QString str;
  QChar w;
  QDict<QString> *attr=0L;
  QPtrList<ImageTag> *images= new QPtrList<ImageTag>;
  MapTag *map=0L;
  QPtrList<MapTag> *maps = new QPtrList<MapTag>;

  _htmlContent.clear();
  currentMapElement = 0L;

  QString temp;
  QString origcode;

  bool readMap=false;

  while (!s.atEnd()) {

    s >> w;
    if (w=="<")
    {
      if (!readMap && origcode != "") {
        _htmlContent.append( new HtmlElement(origcode));
        origcode = "";
      }

      origcode.append("<");
      attr=new QDict<QString>(getTagAttributes(s,temp));
      origcode.append(temp);

      if (attr->find("tagname")) {

        if (attr->find("tagname")->lower()=="img") {
          HtmlImgElement *el = new HtmlImgElement(origcode);
          el->imgTag = static_cast<ImageTag*>(attr);
          images->append(el->imgTag);
          _htmlContent.append(el);

          origcode = "";
        } else
        if (attr->find("tagname")->lower()=="map") {
          map = new MapTag();
          map->name=(*attr->find("name"));
          readMap=true;
        } else
        if (attr->find("tagname")->lower()=="/map") {
          readMap=false;
          maps->append(map);
          HtmlMapElement *el = new HtmlMapElement(origcode);
          el->mapTag = map;
          _htmlContent.append(el);

          origcode = "";
        } else
        if (readMap) {
          if (attr->find("tagname")->lower()=="area") {
             map->prepend(attr);
          }
        } else {
          _htmlContent.append(new HtmlElement(origcode));
          origcode = "";
        }

      }
    } // w != "<"
    else {
      origcode.append(w);
    }
  }

  if (origcode != "") {
    _htmlContent.append(new HtmlElement(origcode));
  }

  f.close();

  KURL imageUrl;
  map = 0L;
     
  
    
  // If there is a preselection of map and image 
  // don't let the user choose something  
  if (imagePath.isNull() || mapName.isNull()) {
    // If we have more than on map or more than one image
    // Let the user choose, otherwise take the only ones
    if (maps->count() == 1) {
      map = maps->first();
    }
    
    if (images->count() == 1) {
      if (images->first()) {
        ImageTag* imgTag = images->first();
        QString *src = imgTag->find("src");
        if (src)
          imageUrl = KURL(url,*src);
      }
    }

    // If there is only one map and more than one image
    // try to find out the image with the according usemap tag
    if (maps->count() == 1 && images->count() > 1) {
        ImageTag* imageTag;
        for ( imageTag = images->first(); imageTag; imageTag = images->next() ) 
        {
            QString *usemap = imageTag->find("usemap");
            if (usemap) {
                // Remove the #
                QString usemapName = usemap->right(usemap->length()-1);
                if (usemapName == map->name) {
                    QString *src = imageTag->find("src");
                    if (src)
                      imageUrl = KURL(url,*src);
                }
            }
        }
    }
    
    
    // If there are more than one map or there wasn't
    // found a fitting image and there is something to choose 
    // let the user choose
    if (maps->count() >1 || (imageUrl.isEmpty() && images->count() > 1))
    {
      ImageMapChooseDialog dialog(widget(),maps,images,url);
      dialog.exec();
      map=dialog.currentMap;
      imageUrl=dialog.pixUrl;
    } 
  } 
  else 
    imageUrl = imagePath;

  imagesListView->clear();
  imagesListView->setBaseUrl(url);
  imagesListView->addImages(images);

  mapsListView->clear();
  mapsListView->addMaps(maps);

  
  setMapActionsEnabled(false);
  
  if (map) {  
    mapsListView->selectMap(map->name);  
  }
  else if ( ! mapName.isNull()) {
    mapsListView->selectMap(mapName);  
  }
  

  // Only update the image if an image was choosen
  if (!imageUrl.isEmpty()) {
    setPicture(imageUrl);
  } 
  

  emit setWindowCaption(url.fileName());      
  setModified(false);
}

/**
 * Finds the first html element which contains the given text.
 * Returns the first matching element.
 * Returns 0L if no element was found.
 */
HtmlElement* KImageMapEditor::findHtmlElement(const QString & containingText) {
  for (HtmlElement * el = _htmlContent.first(); el; el = _htmlContent.next() ) {
    if (el->htmlCode.contains(containingText,false)) {
      return el;
    }
  }
  return 0L;
}

/**
 * Finds the first html element which contains the given ImageTag.
 * Returns the first matching element.
 * Returns 0L if no element was found.
 */
HtmlImgElement* KImageMapEditor::findHtmlImgElement(ImageTag* tag) {
  for (HtmlElement * el = _htmlContent.first(); el; el = _htmlContent.next() ) {
    HtmlImgElement* imgEl = dynamic_cast<HtmlImgElement*>(el);
    
    if (imgEl && imgEl->imgTag == tag)
       return imgEl;
  }
  return 0L;
}

void KImageMapEditor::addMap(const QString & name = QString::null) {
  HtmlMapElement* el = new HtmlMapElement("\n<map></map>");
  MapTag* map = new MapTag();
  map->name = name;
  el->mapTag = map;
  
  // Try to find the body tag
  HtmlElement* bodyTag = findHtmlElement("<body");
  
  // if we found one add the new map right after the body tag
  if (bodyTag) {
     uint index = _htmlContent.find(bodyTag);
     
     // Add a newline before the map 
     _htmlContent.insert(index+1, new HtmlElement("\n"));

     _htmlContent.insert(index+2, el);   
  } // if there is no body tag we add the map to the end of the file
  else {
     // Add a newline before the map 
     _htmlContent.append(new HtmlElement("\n"));

     _htmlContent.append(el);
     kdDebug() << "KImageMapEditor::addMap : No <body found ! Appending new map to the end." << endl;
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
  for (HtmlElement * el = _htmlContent.first(); el; el = _htmlContent.next() ) {
    if (dynamic_cast<HtmlMapElement*>(el)) {
      HtmlMapElement *tagEl = static_cast<HtmlMapElement*>(el);
      if (tagEl->mapTag->name == mapName) {
         return tagEl;
      }
    }
  }

  kdWarning() << "KImageMapEditor::findHtmlMapElement: couldn't find map '" << mapName << "'" << endl;
  return 0L;
}

/**
 * Calls setMap with the HtmlMapElement with the given map name
 */
void KImageMapEditor::setMap(const QString & mapName) {
    HtmlMapElement* el = findHtmlMapElement(mapName);
    if (!el) {
      kdWarning() << "KImageMapEditor::setMap : Couldn't set map '" << mapName << "', because it wasn't found !" << endl;
      return;
    }
    
    setMap(el);

}

void KImageMapEditor::setMap(MapTag* map) {
  for (HtmlElement * el = _htmlContent.first(); el; el = _htmlContent.next() ) {
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
  for (Area* a=areas->first();a!=0L;a=areas->next()) {
    QDict<QString> *dict = new QDict<QString>(17,false);
    QString *shapeStr = 0L;
    
    switch (a->type()) {
      case Area::Rectangle : shapeStr = new QString("rect");break;
      case Area::Circle : shapeStr = new QString("circle");break;
      case Area::Polygon : shapeStr = new QString("poly");break;
      default : continue;
    }
    
    dict->insert("shape",shapeStr);
    
    for (AttributeIterator it = a->firstAttribute();it!=a->lastAttribute();++it)  {
      dict->insert(it.key(),new QString(it.data()));
    }
    
    dict->insert("coords",new QString(a->coordsToString()));
    
    map->append(dict);
    
  }

  if (defaultArea && defaultArea->finished()) {
    QDict<QString> *dict = new QDict<QString>(17,false);
    dict->insert("shape",new QString("default"));
    
    for (AttributeIterator it = defaultArea->firstAttribute();it!=defaultArea->lastAttribute();++it)  {
      dict->insert(it.key(),new QString(it.data()));
    }
    
    map->append(dict);
  }
    
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
    defaultArea = 0L;
//    kdDebug() << "KImageMapEditor::setMap : Setting new map : " << map->name << endl;
    _mapName = map->name;
    for (AreaTag *tag=map->first();tag!=0L;tag=map->next())
    {
        QString shape="rect";
        if (tag->find("shape"))
          shape=*tag->find("shape");

        Area::ShapeType type=Area::Rectangle;
        if (shape=="circle")
          type=Area::Circle;
        else if (shape=="poly")
          type=Area::Polygon;
        else if (shape=="default")
          type=Area::Default;

        Area* a=AreaCreator::create(type);

        if (tag->find("href"))
          a->setAttribute("href",*tag->find("href"));

        if (tag->find("alt"))
          a->setAttribute("alt",*tag->find("alt"));

        if (tag->find("target"))
          a->setAttribute("target",*tag->find("target"));

        if (tag->find("title"))
          a->setAttribute("title",*tag->find("title"));

        if (tag->find("onclick"))
          a->setAttribute("onclick",*tag->find("onclick"));

        if (tag->find("onmousedown"))
          a->setAttribute("onmousedown",*tag->find("onmousedown"));

        if (tag->find("onmouseup"))
          a->setAttribute("onmouseup",*tag->find("onmouseup"));

        if (tag->find("onmouseover"))
          a->setAttribute("onmouseover",*tag->find("onmouseover"));

        if (tag->find("onmousemove"))
          a->setAttribute("onmousemove",*tag->find("onmousemove"));

        if (tag->find("onmouseout"))
          a->setAttribute("onmouseout",*tag->find("onmouseout"));



        if (type==Area::Default) {
          defaultArea=a;
          defaultArea->setFinished(true);
          continue;
        }

        if (tag->find("coords"))
          a->setCoords(*tag->find("coords"));

        a->setMoving(false);
        addArea(a);
    }

    updateAllAreas();
    
    setMapActionsEnabled(true);
}

/**
 * Sets wether actions that depend on an selected map
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

  QString result = "";  
    
  HtmlElement *el;
    for ( el = _htmlContent.first(); el; el = _htmlContent.next() ) {
        result += el->htmlCode;
        //kdDebug() << "KImageMapEditor::getHtmlCode : Writing : " << el->htmlCode << endl;

    }
  return result;
}

void KImageMapEditor::saveImageMap(const KURL & url)
{
  QFileInfo fileInfo(url.path());
  
  if (!QFileInfo(url.directory()).isWritable()) {
    KMessageBox::error(widget(), 
      i18n("<qt>The file <i>%1</i> could not be saved, because you don't have the needed write permissions !</qt>").arg(url.path()));
    return;      
  }
  
  if (!backupFileCreated) {
    QString backupFile = url.path()+"~";
    KIO::file_copy(url, backupFile, -1, true, false, false);
    backupFileCreated = true;
  }
  
  setModified(false);

  if (mapName().isEmpty()) {
    mapEditName();
  }
  QFile file(url.path());
  file.open(IO_WriteOnly);

  QTextStream t(&file);

  if (_htmlContent.isEmpty()) {
    t << "<html>\n"
      << "<head>\n"
      << "  <title></title>\n"
      << "</head>\n"
      << "<body>\n"
      << "  " << getHTMLImageMap()
      << "\n"
      << "  <img src=\"" << QExtFileInfo::toRelative(_imageUrl,url.directory()).path() << "\""
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
  if (copyArea)
    delete copyArea;

  copyArea= static_cast< AreaSelection* > (currentSelected->clone());
  pasteAction->setEnabled(true);
  KCommand *command= new CutCommand(this,*currentSelected);
  commandHistory()->addCommand( command ,true);	
}


void KImageMapEditor::slotDelete()
{
  if ( 0 == currentSelected->count() )
    return;

  KCommand *command= new DeleteCommand(this,*currentSelected);
  commandHistory()->addCommand( command ,true);	
}

void KImageMapEditor::slotCopy()
{
  if (copyArea)
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
  commandHistory()->addCommand( new PasteCommand(this,*a),true);	
  delete a;
//	addAreaAndEdit(a);
}



void KImageMapEditor::slotBackOne()
{
  if (currentSelected->isEmpty())
    return;

  AreaList list = currentSelected->getAreaList();


  Area *a = 0L;
  // move every selected Area one step lower
  for (int i=areas->count()-2; i > -1; i--)
  {
    if (list.find( areas->at(i) ) > -1 )
    {
      a = areas->at(i);
      areas->remove(a);
      areas->insert((uint)i+1,a);
      a->listViewItem()->moveItem( areas->at(i)->listViewItem() );
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

  Area *a = 0L;
  // move every selected Area one step higher
  for (int i=1; i < (int)areas->count(); i++)
  {
    if (list.find( areas->at(i) ) > -1 )
    {
      a = areas->at(i);
      areas->remove(a);
      areas->insert((uint)i-1,a);
      areas->at(i)->listViewItem()->moveItem( a->listViewItem() );	
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

  commandHistory()->addCommand(
    new MoveCommand( this, selected(), r.topLeft() ) ,true );
  selected()->setMoving(false);	
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveDown()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);	
  selected()->moveBy(0,1);

  commandHistory()->addCommand(
    new MoveCommand( this, selected(), r.topLeft() ) ,true );
  selected()->setMoving(false);	
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveLeft()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);	
  selected()->moveBy(-1,0);

  commandHistory()->addCommand(
    new MoveCommand( this, selected(), r.topLeft() ) ,true );
  selected()->setMoving(false);
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotMoveRight()
{
  QRect r=selected()->rect();
  selected()->setMoving(true);	
  selected()->moveBy(1,0);

  commandHistory()->addCommand(
    new MoveCommand( this, selected(), r.topLeft() ) ,true );
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
  r.moveBy(0,-1);

  selected()->setRect(r);

  commandHistory()->addCommand(
    new ResizeCommand( this, selected(), oldArea ) ,true );
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotDecreaseHeight()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setHeight( r.height()-1 );
  r.moveBy(0,1);

  selected()->setRect(r);

  commandHistory()->addCommand(
    new ResizeCommand( this, selected(), oldArea ) ,true );
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotIncreaseWidth()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setWidth( r.width()+1 );

  selected()->setRect(r);

  commandHistory()->addCommand(
    new ResizeCommand( this, selected(), oldArea ) ,true );
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotDecreaseWidth()
{
  Area *oldArea=selected()->clone();

  QRect r = selected()->rect();
  r.setWidth( r.width()-1 );

  selected()->setRect(r);

  commandHistory()->addCommand(
    new ResizeCommand( this, selected(), oldArea ) ,true );
  slotAreaChanged(selected());
  slotUpdateSelectionCoords();
}

void KImageMapEditor::slotHightlightAreas()
{
  bool b = highlightAreasAction->isChecked();

//  highlightAreasAction->setChecked(b);
  Area::highlightArea = b;
  updateAllAreas();
  drawZone->viewport()->repaint();
}

void KImageMapEditor::slotShowAltTag()
{
  bool b = showAltAction->isChecked();
//  showAltAction->setChecked(b);
  Area::showAlt = b;
  drawZone->viewport()->repaint();
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

  int result = KMessageBox::warningYesNo(widget(),
    i18n("<qt>Do you really want to delete the map <i>%1</i> ?"
         " <br>There is no way to undo this !</qt>").arg(selectedMap),
    i18n("Really delete map ?"));

  if (result == KMessageBox::No)
     return;



  mapsListView->removeMap(selectedMap);
  HtmlMapElement* mapEl = findHtmlMapElement(selectedMap);
  _htmlContent.remove(mapEl);
  if (mapsListView->count() == 0) {

      currentMapElement = 0L;
      deleteAllAreas();
      setMapActionsEnabled(false);
  }
  else {
      // The old one was deleted, so the new one got selected
      setMap(mapsListView->selectedMap());
  }
}

void KImageMapEditor::mapPreview() {
  HTMLPreviewDialog dialog(widget(), url(), getHtmlCode());
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
 * Doesn't call the closeURL method, because
 * we need the URL for the session management
 */
bool KImageMapEditor::queryClose() {
  if ( ! isModified() )
     return true;
     
  switch ( KMessageBox::warningYesNoCancel( widget(),
   i18n("<qt>The file <i>%1</i> has been modified.<br>Do you want to save it ?</qt>").arg(url().fileName())) ) {
           case KMessageBox::Yes :
             saveFile();
             return TRUE;
           case KMessageBox::No :
             return TRUE;
           default: 
             return FALSE;     
  }             
}

bool KImageMapEditor::closeURL()
{
  bool result = KParts::ReadWritePart::closeURL();
  if (!result)
     return false;

  _htmlContent.clear();
  deleteAllMaps();
  imagesListView->clear();

  if (copyArea)
     delete copyArea;
  copyArea=0L;

  if (defaultArea)
     delete defaultArea;
  defaultArea=0L;

  currentMapElement = 0L;

  init();
  emit setWindowCaption("");

  return true;

}

void KImageMapEditor::addImage(const KURL & imgUrl) {
    if (imgUrl.isEmpty())
        return;

    QString relativePath = QExtFileInfo::toRelative(imgUrl, url().directory()).path();

    QString imgHtml = QString("<img src=\"")+relativePath+QString("\">");
    ImageTag *imgTag = new ImageTag();
    imgTag->insert("tagname",new QString("img"));
    imgTag->insert("src", new QString(relativePath));

    HtmlImgElement *imgEl = new HtmlImgElement(imgHtml);
    imgEl->imgTag = imgTag;

    HtmlElement *bodyEl = findHtmlElement("<body");
    if (bodyEl) {
        int bodyIndex = _htmlContent.find(bodyEl);
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
    KURL imgUrl = KFileDialog::getImageOpenURL();
    addImage(imgUrl);
}

void KImageMapEditor::imageRemove() {
    ImageTag* imgTag = imagesListView->selectedImage();
    HtmlImgElement* imgEl = findHtmlImgElement(imgTag);
    imagesListView->removeImage(imgTag);
    _htmlContent.remove(imgEl);

    if (imagesListView->childCount() == 0) {
        setPicture(getBackgroundImage());
        setImageActionsEnabled(false);
    }
    else {
        ImageTag* selected = imagesListView->selectedImage();
        if (selected) {
            QString *url = selected->find("src");
            if (url) {
                setPicture(KURL(*url));
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

  QString usemap = "";

  if (imageTag->find("usemap"))
      usemap=*imageTag->find("usemap");

  QStringList maps = mapsListView->getMaps();     
  int index = maps.findIndex(usemap);
  if (index == -1) {
    maps.prepend("");
    index = 0;
  }    
  
  QString input = KInputDialog::getItem(i18n("Enter usmap"),
    i18n("Enter the usemap value"),
    maps,index,true,&ok,widget());
  if (ok) {
     imageTag->replace("usemap", new QString(input));
     imagesListView->updateImage(imageTag);
     setModified(true);

     // Update the htmlCode of the HtmlElement
     HtmlImgElement* imgEl = findHtmlImgElement(imageTag);

     imgEl->htmlCode = "<";
     QString *tagName = imgEl->imgTag->find("tagname");
     imgEl->htmlCode += QString(*tagName);

     QDictIterator<QString> it( *imgEl->imgTag );
     for( ; it.current(); ++it ) {
        if (it.currentKey() != "tagname") {
           imgEl->htmlCode += " " + it.currentKey() + "=\"";
           imgEl->htmlCode += *it.current();
           imgEl->htmlCode += "\"";
        }
     }

     imgEl->htmlCode += ">";

  }
}

void KImageMapEditor::configureShowAreaList() {
  if (configureShowAreaListAction->isChecked())
    mainDock->makeDockVisible(areaDock);
  else
    mainDock->makeDockInvisible(areaDock);
}

void KImageMapEditor::configureShowMapList() {
  if (configureShowMapListAction->isChecked())
    mainDock->makeDockVisible(mapsDock);
  else
    mainDock->makeDockInvisible(mapsDock);
}

void KImageMapEditor::configureShowImageList() {
  if (configureShowImageListAction->isChecked())
    mainDock->makeDockVisible(imagesDock);
  else
    mainDock->makeDockInvisible(imagesDock);
}

void KImageMapEditor::dockingStateChanged() {
  if (areaDock) {
    configureShowImageListAction->setChecked( imagesDock->isVisible() );                        
    configureShowAreaListAction->setChecked( areaDock->isVisible() );                        
    configureShowMapListAction->setChecked( mapsDock->isVisible() );                        
  }
}
