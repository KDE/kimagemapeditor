/***************************************************************************
                          kimedialogs.cpp  -  description
                            -------------------
    begin                : Tue Apr 17 2001
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

// QT
#include <qcheckbox.h>
#include <q3multilineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qimage.h>
#include <QPixmap>
#include <QGridLayout>
#include <QLinkedList>
#include <QFrame>
#include <QVBoxLayout>

// KDE
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <ktemporaryfile.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>
#include <kglobal.h>
#include <kvbox.h>

// LOCAL
#include "kimedialogs.h"

CoordsEdit::CoordsEdit(QWidget *parent, Area* a)
  : QWidget(parent)
{
  area=a;
}

void CoordsEdit::applyChanges() {
  return;
}

void CoordsEdit::slotTriggerUpdate() {
  applyChanges();
  emit update();
}

CoordsEdit::~CoordsEdit()
{
}

RectCoordsEdit::RectCoordsEdit(QWidget *parent, Area* a)
  : CoordsEdit(parent,a)
{
  QGridLayout *layout= new QGridLayout(this); //,5,2,5,5);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(INT_MAX);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  layout->addWidget(topXSpin,0,1);
  connect( topXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Top &X:"),this);
  lbl->setBuddy(topXSpin);
  layout->addWidget(lbl,0,0);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(INT_MAX);
  topYSpin->setMinimum(0);
  topYSpin->setValue(a->rect().top());
  layout->addWidget(topYSpin,1,1);
  connect( topYSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("Top &Y:"),this);
  lbl->setBuddy(topYSpin);
  layout->addWidget(lbl,1,0);

  widthSpin = new QSpinBox(this);
  widthSpin->setMaximum(INT_MAX);
  widthSpin->setMinimum(0);
  widthSpin->setValue(a->rect().width());
  layout->addWidget(widthSpin,2,1);
  connect( widthSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("&Width:"),this);
  lbl->setBuddy(widthSpin);
  layout->addWidget(lbl,2,0);

  heightSpin = new QSpinBox(this);
  heightSpin->setMaximum(INT_MAX);
  heightSpin->setMinimum(0);
  heightSpin->setValue(a->rect().height());
  layout->addWidget(heightSpin,3,1);
  connect( heightSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("Hei&ght:"),this);
  lbl->setBuddy(heightSpin);
  layout->addWidget(lbl,3,0);

  layout->setRowStretch(4,10);
}

void RectCoordsEdit::applyChanges() {
  QRect r;
  r.setLeft(topXSpin->text().toInt());
  r.setTop(topYSpin->text().toInt());
  r.setWidth(widthSpin->text().toInt());
  r.setHeight(heightSpin->text().toInt());
  area->setRect(r);
}

CircleCoordsEdit::CircleCoordsEdit(QWidget *parent, Area* a)
  : CoordsEdit(parent,a)
{
  QGridLayout *layout= new QGridLayout(this);

  centerXSpin = new QSpinBox(this);
  centerXSpin->setMaximum(INT_MAX);
  centerXSpin->setMinimum(0);
  centerXSpin->setValue(a->rect().center().x());
  layout->addWidget(centerXSpin,0,1);
  connect( centerXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Center &X:"),this);
  lbl->setBuddy(centerXSpin);
  layout->addWidget(lbl,0,0);

  centerYSpin = new QSpinBox(this);
  centerYSpin->setMaximum(INT_MAX);
  centerYSpin->setMinimum(0);
  centerYSpin->setValue(a->rect().center().y());
  layout->addWidget(centerYSpin,1,1);
  connect( centerYSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));


  lbl= new QLabel(i18n("Center &Y:"),this);
  lbl->setBuddy(centerYSpin);
  layout->addWidget(lbl,1,0);

  radiusSpin = new QSpinBox(this);
  radiusSpin->setMaximum(INT_MAX);
  radiusSpin->setMinimum(0);
  radiusSpin->setValue(a->rect().width()/2);
  layout->addWidget(radiusSpin,2,1);
  connect( radiusSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));


  lbl= new QLabel(i18n("&Radius:"),this);
  lbl->setBuddy(radiusSpin);
  layout->addWidget(lbl,2,0);

  layout->setRowStretch(3,10);

}

void CircleCoordsEdit::applyChanges() {
  QRect r;
  r.setWidth(radiusSpin->text().toInt()*2);
  r.setHeight(radiusSpin->text().toInt()*2);
  r.moveCenter(QPoint(centerXSpin->text().toInt(),
                      centerYSpin->text().toInt()));
  area->setRect(r);
}

PolyCoordsEdit::PolyCoordsEdit(QWidget *parent, Area* a)
  : CoordsEdit(parent,a)
{
  if (!a) return;
  QVBoxLayout *layout= new QVBoxLayout(this);
  coordsTable= new QTableWidget(0,2,this);
  coordsTable->verticalHeader()->hide();
  // PORT: coordsTable->setLeftMargin(0);
  coordsTable->setSelectionMode( QTableWidget::SingleSelection );
  connect( coordsTable, SIGNAL(currentChanged(int,int)), this, SLOT(slotHighlightPoint(int)));

  updatePoints();
//	coordsTable->setMinimumHeight(50);
//	coordsTable->setMaximumHeight(400);
//	coordsTable->resizeContents(100,100);
  coordsTable->resize(coordsTable->width(),100);
  layout->addWidget(coordsTable);
  layout->setStretchFactor(coordsTable,-1);
  KHBox *hbox= new KHBox(this);
  QPushButton *addBtn=new QPushButton(i18n("Add"),hbox);
  connect( addBtn, SIGNAL(pressed()), this, SLOT(slotAddPoint()));
  QPushButton *removeBtn=new QPushButton(i18n("Remove"),hbox);
  connect( removeBtn, SIGNAL(pressed()), this, SLOT(slotRemovePoint()));

  layout->addWidget(hbox);
  slotHighlightPoint(1);
}

PolyCoordsEdit::~PolyCoordsEdit() {
}

void PolyCoordsEdit::slotHighlightPoint(int row) {
  if (!area) return;
  area->highlightSelectionPoint(row);
  emit update();
}


void PolyCoordsEdit::updatePoints() {
  coordsTable->clear();

  int count=area->coords().size();

  coordsTable->setHorizontalHeaderLabels(QStringList() << "X" << "Y");
  coordsTable->setRowCount(count);

  for (int i=0;i<count;i++) {
    coordsTable->setItem(i,0, new QTableWidgetItem(QString::number(area->coords().point(i).x()) ));
    coordsTable->setItem(i,1, new QTableWidgetItem(QString::number(area->coords().point(i).y()) ));
  }

  emit update();
}

void PolyCoordsEdit::slotAddPoint() {
  int newPos= coordsTable->currentRow();
  if (newPos < 0 || newPos >= area->coords().size()) 
    newPos = area->coords().size();

  QPoint currentPoint=area->coords().point(newPos);
  area->insertCoord(newPos,currentPoint);
  updatePoints();

}

void PolyCoordsEdit::slotRemovePoint() {
  int currentPos= coordsTable->currentRow();
  if (currentPos < 0 || currentPos >= area->coords().size())
    return;
  area->removeCoord(currentPos);
  updatePoints();
}

void PolyCoordsEdit::applyChanges() {
  int count=coordsTable->rowCount();

  for (int i=0;i<count;i++) {
    QPoint newPoint( coordsTable->item(i,0)->text().toInt(),
                    coordsTable->item(i,1)->text().toInt());

    area->moveCoord(i,newPoint);
  }
}

SelectionCoordsEdit::SelectionCoordsEdit(QWidget *parent, Area* a)
  : CoordsEdit(parent,a)
{
  QGridLayout *layout= new QGridLayout(this);//,2,2);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(INT_MAX);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  layout->addWidget(topXSpin,0,1);
  connect( topXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Top &X"),this);
  lbl->setBuddy(topXSpin);
  layout->addWidget(lbl,0,0);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(INT_MAX);
  topYSpin->setMinimum(0);
  topYSpin->setValue(a->rect().top());
  layout->addWidget(topYSpin,1,1);
  connect( topYSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("Top &Y"),this);
  lbl->setBuddy(topYSpin);
  layout->addWidget(lbl,1,0);
}

void SelectionCoordsEdit::applyChanges() {
  area->moveTo(topXSpin->text().toInt(), topYSpin->text().toInt());
}



QLineEdit* AreaDialog::createLineEdit(QWidget* parent, QGridLayout *layout, int y, const QString & value, const QString & name)
{
  QLineEdit* edit=new QLineEdit(value,parent);
  layout->addWidget(edit,y,2);
  QLabel* lbl=new QLabel(name,parent);
  lbl->setBuddy(edit);
  layout->addWidget(lbl,y,1);

  return edit;
}

QWidget* AreaDialog::createGeneralPage()
{
  QFrame* page = new QFrame(this);
  QGridLayout* layout = new QGridLayout(page);//,5,2,5,5);


  KHBox *hbox= new KHBox(page);
  hrefEdit = new QLineEdit(area->attribute("href"),hbox);
  QPushButton *btn = new QPushButton("",hbox);
  btn->setIcon(SmallIcon("document-open"));
  connect( btn, SIGNAL(pressed()), this, SLOT(slotChooseHref()));
  hbox->setMinimumHeight(hbox->height());

  layout->addWidget(hbox,0,2);
  QLabel *lbl=new QLabel(i18n( "&HREF:" ),page);
  lbl->setBuddy(hrefEdit);
  layout->addWidget(lbl,0,1);

  altEdit = createLineEdit(page,layout,1,
			   area->attribute("alt"),
			   i18n("Alt. &Text:"));
  targetEdit = createLineEdit(page,layout,2,
			      area->attribute("target"),
			      i18n("Tar&get:"));
  titleEdit = createLineEdit(page,layout,3,
			     area->attribute("title"),
			     i18n("Tit&le:"));

  if (area->type()==Area::Default)
  {
    defaultAreaChk = new QCheckBox(i18n("Enable default map"),page);
    if (area->finished())
      defaultAreaChk->setChecked(true);
    layout->addWidget(defaultAreaChk,4,2);
  }


  layout->setRowStretch(4,10);

  return page;
}

QWidget* AreaDialog::createCoordsPage()
{
  QFrame* page = new QFrame(this);
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setMargin(5);

  coordsEdit = createCoordsEdit(page,area);
  layout->addWidget(coordsEdit);
  connect( coordsEdit, SIGNAL(update()), this, SLOT(slotUpdateArea()));

  return page;
}

QWidget* AreaDialog::createJavascriptPage()
{
  QFrame* page = new QFrame(this);
  QGridLayout* layout = new QGridLayout(page);//,8,2,5,5);

  onClickEdit = createLineEdit(page,layout,0,area->attribute("onClick"),i18n("OnClick:"));
  onDblClickEdit = createLineEdit(page,layout,1,area->attribute("onDblClick"),i18n("OnDblClick:"));
  onMouseDownEdit = createLineEdit(page,layout,2,area->attribute("onMouseDown"),i18n("OnMouseDown:"));
  onMouseUpEdit = createLineEdit(page,layout,3,area->attribute("onMouseUp"),i18n("OnMouseUp:"));
  onMouseOverEdit = createLineEdit(page,layout,4,area->attribute("onMouseOver"),i18n("OnMouseOver:"));
  onMouseMoveEdit = createLineEdit(page,layout,5,area->attribute("onMouseMove"),i18n("OnMouseMove:"));
  onMouseOutEdit = createLineEdit(page,layout,6,area->attribute("onMouseOut"),i18n("OnMouseOut:"));

  layout->setRowStretch(7,10);


  return page;
}

AreaDialog::AreaDialog(KImageMapEditor* parent,Area * a)
  : KDialog(parent->widget())
// : KDialogBase(Tabbed,i18n("Area Tag Editor"),Ok|Apply|Cancel,Ok,parent,"")
//	: KDialogBase(parent,"",true,"Area Tag Editor",Ok|Apply|Cancel,Ok,true)
{
  setCaption(i18n("Area Tag Editor"));
  setButtons(Ok|Apply|Cancel);
  setDefaultButton(Ok);
  //  setFaceType( KPageDialog::Tabbed );
  setObjectName( "Area Tag Editor" );
  setModal(true);
  
  _document=parent;

  if (!a) {
      slotCancel();
      return;
  }


  area=a;
  QString shape="Default";
  areaCopy= a->clone();
  oldArea= new Area();
  oldArea->setRect( a->rect() );

  switch (a->type()) {
    case Area::Rectangle : shape=i18n("Rectangle");break;
    case Area::Circle : shape=i18n("Circle");break;
    case Area::Polygon : shape=i18n("Polygon");break;
    case Area::Selection : shape=i18n("Selection");break;
    default : break;
  }


  // To get a margin around everything

  QWidget* w = mainWidget();

  QVBoxLayout *layout = new QVBoxLayout(w);

  layout->setMargin(5);

  QLabel *lbl = new QLabel("<b>"+shape+"</b>",w);
  lbl->setTextFormat(Qt::RichText);
  layout->addWidget(lbl);

  QFrame *line = new QFrame(w);
  line->setFrameStyle(QFrame::HLine  | QFrame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line);

  QTabWidget *tab = new QTabWidget(w);

  layout->addWidget(tab);

  tab->addTab(createGeneralPage(),i18n("&General"));

  if (a->type()==Area::Default)
  {
      shape=i18n("Default");
  }
  else
    tab->addTab(createCoordsPage(),i18n("Coor&dinates"));

  tab->addTab(createJavascriptPage(),i18n("&JavaScript"));

  setMinimumHeight(360);
  setMinimumWidth(327);

  connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
  connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
  connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()));


  resize(327,360);
}

AreaDialog::~AreaDialog() {
  delete areaCopy;
  delete oldArea;
}

CoordsEdit* AreaDialog::createCoordsEdit(QWidget *parent, Area *a) {
  if (!a) return 0;
  switch (a->type()) {
    case Area::Rectangle :
        return new RectCoordsEdit(parent,a);
      break;
    case Area::Circle :
        return new CircleCoordsEdit(parent,a);
      break;
    case Area::Polygon :
        return new PolyCoordsEdit(parent,a);
      break;
    case Area::Selection :
        return new SelectionCoordsEdit(parent,a);
      break;
    case Area::Default : return new CoordsEdit(parent,a); break;
    default : return new CoordsEdit(parent,a);break;
  }
}

void AreaDialog::slotChooseHref() {
  KUrl url=KFileDialog::getOpenUrl(KUrl(), "*|" + i18n( "All Files" ), this, i18n("Choose File"));
  if (!url.isEmpty()) {
    hrefEdit->setText(url.url());
  }
}

void AreaDialog::slotOk() {
  if (area)
  {
    area->highlightSelectionPoint(-1);
    if (area->type()==Area::Default)
      area->setFinished(defaultAreaChk->isChecked());
  }
  slotApply();
  accept();

}

void AreaDialog::slotApply() {
  if (area) {
    if (area->type()!=Area::Default)
      coordsEdit->applyChanges();

    area->setAttribute("href",hrefEdit->text());
    area->setAttribute("alt",altEdit->text());
    area->setAttribute("target",targetEdit->text());
    area->setAttribute("title",titleEdit->text());
    area->setAttribute("onclick",onDblClickEdit->text());
    area->setAttribute("ondblclick",onClickEdit->text());
    area->setAttribute("onmousedown",onMouseDownEdit->text());
    area->setAttribute("onmouseup",onMouseUpEdit->text());
    area->setAttribute("onmousemove",onMouseMoveEdit->text());
    area->setAttribute("onmouseover",onMouseOverEdit->text());
    area->setAttribute("onmouseout",onMouseOutEdit->text());

    // redraw old area to get rid of it
    emit areaChanged(oldArea);
    // draw new area
    emit areaChanged(area);
    oldArea->setRect(area->rect());
  }
}

void AreaDialog::slotCancel() {
  if (area) {
    AreaSelection *selection=0L;
    if ( (selection=dynamic_cast<AreaSelection*>(areaCopy)) )
      area->setArea(*selection);
    else
      area->setArea(*areaCopy);
    area->highlightSelectionPoint(-1);
    emit areaChanged(oldArea);
    emit areaChanged(area);
  }
  reject();
}

void AreaDialog::slotUpdateArea() {
    emit areaChanged(oldArea);
    // draw new area
    emit areaChanged(area);
    oldArea->setRect(area->rect());
}


PreferencesDialog::PreferencesDialog(QWidget *parent, KConfig* conf)
  : KDialog(parent)
{
  config = conf;
  setCaption(i18n("Preferences"));
  setButtons(Ok|Apply|Cancel);
  setDefaultButton(Ok);
  setModal(true);
  showButtonSeparator(true);
  KVBox *page=new KVBox(this);
  page->setSpacing(6);
  setMainWidget(page);

  KHBox *hbox= new KHBox(page);

  QLabel *lbl = new QLabel(i18n("&Maximum image preview height:")+' ',hbox);
  rowHeightSpinBox = new QSpinBox(hbox);
  lbl->setBuddy(rowHeightSpinBox);

  int maxPrevHeight = config->group("Appearance").readEntry("maximum-preview-height",50);
  rowHeightSpinBox->setMaximum(1000);
  rowHeightSpinBox->setMinimum(15);
  rowHeightSpinBox->setFixedWidth(60);
  rowHeightSpinBox->setValue(maxPrevHeight);

  KConfigGroup general = config->group("General");

  hbox= new KHBox(page);
  lbl = new QLabel(i18n("&Undo limit:")+' ',hbox);
  undoSpinBox = new QSpinBox(hbox);
  undoSpinBox->setFixedWidth(60);
  lbl->setBuddy(undoSpinBox);

  undoSpinBox->setMaximum(100);
  undoSpinBox->setMinimum(1);
  undoSpinBox->setValue(general.readEntry("undo-level",20));

  hbox= new KHBox(page);
  lbl = new QLabel(i18n("&Redo limit:")+' ',hbox);

  redoSpinBox = new QSpinBox(hbox);
  redoSpinBox->setFixedWidth(60);
  redoSpinBox->setMaximum(100);
  redoSpinBox->setMinimum(1);
  redoSpinBox->setValue(general.readEntry("redo-level",20));
  lbl->setBuddy(redoSpinBox);

  startWithCheck = new QCheckBox(i18n("&Start with last used document"),page);
  startWithCheck->setChecked(general.readEntry("start-with-last-used-document",true));

/*
  hbox= new QHBox(page);
  (void)new QLabel(i18n("Highlight Areas")+" ",hbox);

  colorizeAreaChk = new QCheckBox(hbox);
  colorizeAreaChk->setFixedWidth(60);
  colorizeAreaChk->setChecked(KGlobal::config()->readEntry("highlightareas",true));

  hbox= new QHBox(page);
  (void)new QLabel(i18n("Show alternative text")+" ",hbox);

  showAltChk = new QCheckBox(hbox);
  showAltChk->setFixedWidth(60);
  showAltChk->setChecked(KGlobal::config()->readEntry("showalt",true));
*/
  connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
  connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

PreferencesDialog::~PreferencesDialog() {
}

void PreferencesDialog::slotDefault( void ) {
  rowHeightSpinBox->setValue(50);
}

void PreferencesDialog::slotOk( void ) {
  slotApply();
  accept();
}

void PreferencesDialog::slotApply( void ) {
  KConfigGroup group = config->group("Appearance");
  group.writeEntry("maximum-preview-height",rowHeightSpinBox->cleanText().toInt());

  group = config->group("General Options");
  group.writeEntry("undo-level",undoSpinBox->cleanText().toInt());
  group.writeEntry("redo-level",redoSpinBox->cleanText().toInt());
  group.writeEntry("start-with-last-used-document", startWithCheck->isChecked());
 
  config->sync();
  emit preferencesChanged();
}

HTMLPreviewDialog::HTMLPreviewDialog(QWidget* parent, const KUrl & url, const QString & htmlCode)
  : KDialog(parent)
{
  tempFile = new KTemporaryFile();
  tempFile->setPrefix(url.directory(KUrl::AppendTrailingSlash));
  tempFile->setSuffix(".html");
  tempFile->open();
  setCaption(i18n("Preview"));
  setButtons(Ok);
  setDefaultButton(Ok);
  setModal(true);
  QTextStream stream(tempFile);
  stream << htmlCode;
  kDebug() << "HTMLPreviewDialog: TempFile : " << tempFile->fileName();
  stream.flush();

  KVBox *page = new KVBox(this);
  setMainWidget(page);

  htmlPart = new KHTMLPart(page);
//  htmlView = new KHTMLView(htmlPart, page);
//  htmlView->setVScrollBarMode(QScrollView::Auto);
//  htmlView->setHScrollBarMode(QScrollView::Auto);
//  dialog->resize(dialog->calculateSize(edit->maxLineWidth(),edit->numLines()*));
//	dialog->adjustSize();
  QLabel* lbl = new QLabel( page );
  lbl->setObjectName( "urllabel" );

  connect( htmlPart, SIGNAL( onURL(const QString&)), lbl, SLOT( setText(const QString&)));
}

HTMLPreviewDialog::~HTMLPreviewDialog() {
  delete tempFile;
  delete htmlPart;
}

void HTMLPreviewDialog::show() {
  KDialog::show();
  htmlPart->openUrl(KUrl( tempFile->fileName() ));
//  htmlView->layout();
//  htmlView->repaint();
  resize(800,600);
}

#include "kimedialogs.moc"
