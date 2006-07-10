/***************************************************************************
                          kimedialogs.cpp  -  description
                            -------------------
    begin                : Tue Apr 17 2001
    copyright            : (C) 2001 by Jan Sch�er
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
//#include <qstring.h>
#include <qcheckbox.h>
#include <q3multilineedit.h>
#include <qlayout.h>
#include <qlabel.h>


#include <qlineedit.h>
#include <q3listbox.h>
#include <q3table.h>
#include <q3groupbox.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <q3pointarray.h>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3PtrList>
#include <Q3Frame>
#include <Q3VBoxLayout>
//#include <qwidget.h>
// KDE
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <ktempfile.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
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
  Q3GridLayout *layout= new Q3GridLayout(this,5,2,5,5);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(2000);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  layout->addWidget(topXSpin,0,1);
  connect( topXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Top &X:"),this);
  lbl->setBuddy(topXSpin);
  layout->addWidget(lbl,0,0);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(2000);
  topYSpin->setMinimum(0);
  topYSpin->setValue(a->rect().top());
  layout->addWidget(topYSpin,1,1);
  connect( topYSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("Top &Y:"),this);
  lbl->setBuddy(topYSpin);
  layout->addWidget(lbl,1,0);

  widthSpin = new QSpinBox(this);
  widthSpin->setMaximum(2000);
  widthSpin->setMinimum(0);
  widthSpin->setValue(a->rect().width());
  layout->addWidget(widthSpin,2,1);
  connect( widthSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  lbl= new QLabel(i18n("&Width:"),this);
  lbl->setBuddy(widthSpin);
  layout->addWidget(lbl,2,0);

  heightSpin = new QSpinBox(this);
  heightSpin->setMaximum(2000);
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
  Q3GridLayout *layout= new Q3GridLayout(this,4,2,5,5);

  centerXSpin = new QSpinBox(this);
  centerXSpin->setMaximum(2000);
  centerXSpin->setMinimum(0);
  centerXSpin->setValue(a->rect().center().x());
  layout->addWidget(centerXSpin,0,1);
  connect( centerXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Center &X:"),this);
  lbl->setBuddy(centerXSpin);
  layout->addWidget(lbl,0,0);

  centerYSpin = new QSpinBox(this);
  centerYSpin->setMaximum(2000);
  centerYSpin->setMinimum(0);
  centerYSpin->setValue(a->rect().center().y());
  layout->addWidget(centerYSpin,1,1);
  connect( centerYSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));


  lbl= new QLabel(i18n("Center &Y:"),this);
  lbl->setBuddy(centerYSpin);
  layout->addWidget(lbl,1,0);

  radiusSpin = new QSpinBox(this);
  radiusSpin->setMaximum(2000);
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
  Q3VBoxLayout *layout= new Q3VBoxLayout(this);
  int numPoints=a->coords()->count();
  coordsTable= new Q3Table(numPoints,2,this);
  coordsTable->horizontalHeader()->setLabel(0,"X");
  coordsTable->horizontalHeader()->setLabel(1,"Y");
  coordsTable->verticalHeader()->hide();
  coordsTable->setLeftMargin(0);
  coordsTable->setSelectionMode( Q3Table::Single );


  for (int i=0;i<numPoints;i++) {
    coordsTable->setText(i,0, QString::number(area->coords()->point(i).x()) );
    coordsTable->setText(i,1, QString::number(area->coords()->point(i).y()) );
  }

  connect( coordsTable, SIGNAL(currentChanged(int,int)), this, SLOT(slotHighlightPoint(int)));

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
  if (area)
    area->highlightSelectionPoint(-1);
}

void PolyCoordsEdit::slotHighlightPoint(int row) {
  if (!area) return;
  area->highlightSelectionPoint(row);
  emit update();
}


void PolyCoordsEdit::slotAddPoint() {
  int newPos= coordsTable->currentRow();
  QPoint currentPoint=area->coords()->point(newPos);
  area->insertCoord(newPos,currentPoint);

  int count=area->coords()->size();

  coordsTable->setNumRows(count);

  for (int i=0;i<count;i++) {
    coordsTable->setText(i,0, QString::number(area->coords()->point(i).x()) );
    coordsTable->setText(i,1, QString::number(area->coords()->point(i).y()) );
  }

  emit update();
}

void PolyCoordsEdit::slotRemovePoint() {
  int currentPos= coordsTable->currentRow();

  area->removeCoord(currentPos);

  int count=area->coords()->size();

  coordsTable->setNumRows(count);

  for (int i=0;i<count;i++) {
    coordsTable->setText(i,0, QString::number(area->coords()->point(i).x()) );
    coordsTable->setText(i,1, QString::number(area->coords()->point(i).y()) );
  }

  emit update();
}

void PolyCoordsEdit::applyChanges() {
  int count=coordsTable->numRows();

  for (int i=0;i<count;i++) {
    QPoint newPoint( coordsTable->text(i,0).toInt(),
                    coordsTable->text(i,1).toInt());

    area->moveCoord(i,newPoint);
  }
}

SelectionCoordsEdit::SelectionCoordsEdit(QWidget *parent, Area* a)
  : CoordsEdit(parent,a)
{
  Q3GridLayout *layout= new Q3GridLayout(this,2,2);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(2000);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  layout->addWidget(topXSpin,0,1);
  connect( topXSpin, SIGNAL(valueChanged(const QString &)), this, SLOT(slotTriggerUpdate()));

  QLabel *lbl= new QLabel(i18n("Top &X"),this);
  lbl->setBuddy(topXSpin);
  layout->addWidget(lbl,0,0);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(2000);
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



QLineEdit* AreaDialog::createLineEdit(QWidget* parent, Q3GridLayout *layout, int y, const QString & value, const QString & name)
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
  Q3Frame* page = new Q3Frame(this);
  Q3GridLayout* layout = new Q3GridLayout(page,5,2,5,5);


  KHBox *hbox= new KHBox(page);
  hrefEdit = new QLineEdit(area->attribute("href"),hbox);
  QPushButton *btn = new QPushButton("",hbox);
  btn->setPixmap(SmallIcon("fileopen"));
  connect( btn, SIGNAL(pressed()), this, SLOT(slotChooseHref()));
  hbox->setMinimumHeight(hbox->height());

  layout->addWidget(hbox,0,2);
  QLabel *lbl=new QLabel(i18n( "&HREF:" ),page);
  lbl->setBuddy(hrefEdit);
  layout->addWidget(lbl,0,1);

  altEdit = createLineEdit(page,layout,1,area->attribute("alt"),i18n("Alt. &Text:"));
  targetEdit = createLineEdit(page,layout,2,area->attribute("target"),i18n("Tar&get:"));
  titleEdit = createLineEdit(page,layout,3,area->attribute("title"),i18n("Tit&le:"));

  if (area->type()==Area::Default)
  {
    defaultAreaChk = new QCheckBox(i18n("Enable default map"),page);
    if (area->finished())
      defaultAreaChk->setChecked(true);
    layout->addWidget(defaultAreaChk,3,2);
  }


  layout->setRowStretch(4,10);

  return page;
}

QWidget* AreaDialog::createCoordsPage()
{
  Q3Frame* page = new Q3Frame(this);
  Q3VBoxLayout *layout = new Q3VBoxLayout(page);
  layout->setMargin(5);

  coordsEdit = createCoordsEdit(page,area);
  layout->addWidget(coordsEdit);
  connect( coordsEdit, SIGNAL(update()), this, SLOT(slotUpdateArea()));

  return page;
}

QWidget* AreaDialog::createJavascriptPage()
{
  Q3Frame* page = new Q3Frame(this);
  Q3GridLayout* layout = new Q3GridLayout(page,8,2,5,5);

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

QWidget* AreaDialog::createButtonBar()
{
  KHBox *box = new KHBox(this);
  QWidget *spacer = new QWidget(box);
  QPushButton *okBtn = new KPushButton(KStdGuiItem::ok(),box);
  QPushButton *applyBtn = new KPushButton(KStdGuiItem::apply(),box);
  QPushButton *cancelBtn = new KPushButton(KStdGuiItem::cancel(),box);

  connect(okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(applyBtn, SIGNAL(clicked()), this, SLOT(slotApply()));
  connect(cancelBtn, SIGNAL(clicked()), this, SLOT(slotCancel()));

  box->setSpacing(5);
  box->setStretchFactor(spacer,10);

  okBtn->setDefault(true);

  return box;

}

AreaDialog::AreaDialog(KImageMapEditor* parent,Area * a)
  : KDialog(parent->widget())
// : KDialogBase(Tabbed,i18n("Area Tag Editor"),Ok|Apply|Cancel,Ok,parent,"")
//	: KDialogBase(parent,"",true,"Area Tag Editor",Ok|Apply|Cancel,Ok,true)
{
  if (!a) slotCancel();

  _document=parent;

  setCaption(i18n("Area Tag Editor"));

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

  Q3VBoxLayout *layout = new Q3VBoxLayout(this);

  layout->setMargin(5);

  QLabel *lbl = new QLabel("<b>"+shape+"</b>",this);
  lbl->setTextFormat(Qt::RichText);
  layout->addWidget(lbl);

  Q3Frame *line = new Q3Frame(this);
  line->setFrameStyle(Q3Frame::HLine  | Q3Frame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line);

  QTabWidget *tab = new QTabWidget(this);

  layout->addWidget(tab);

  tab->addTab(createGeneralPage(),i18n("&General"));

  if (a->type()==Area::Default)
  {
      shape=i18n("Default");
  }
  else
    tab->addTab(createCoordsPage(),i18n("Coor&dinates"));

  tab->addTab(createJavascriptPage(),i18n("&JavaScript"));

  line = new Q3Frame(this);
  line->setFrameStyle(Q3Frame::HLine  | Q3Frame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line);

  layout->addWidget(createButtonBar());

  setMinimumHeight(360);
  setMinimumWidth(327);

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
  KUrl url=KFileDialog::getOpenUrl(QString::null, "*|" + i18n( "All Files" ), this, i18n("Choose File"));
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

ImageMapChooseDialog::ImageMapChooseDialog(QWidget* parent,Q3PtrList<MapTag> *_maps,Q3PtrList<ImageTag> *_images,const KUrl & _baseUrl)
  : KDialog(parent)
{
  setCaption(i18n( "Choose Map & Image to Edit" ));
  setModal(true);
  setButtons(Ok);
  setDefaultButton(Ok);
  showButtonSeparator(true);
  baseUrl=_baseUrl;
  maps=_maps;
  images=_images;
  currentMap=0L;
  QWidget *page=new QWidget(this);
  setMainWidget(page);
  setCaption(baseUrl.fileName());
  Q3VBoxLayout *layout = new Q3VBoxLayout(page,5,5);

  QLabel *lbl= new QLabel(i18n("Select an image and/or a map that you want to edit"),page);
  lbl->setFont(QFont("Sans Serif",12, QFont::Bold));
  layout->addWidget(lbl);
  Q3Frame *line= new Q3Frame(page);
  line->setFrameStyle(Q3Frame::HLine  | Q3Frame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line,0);

  Q3GridLayout *gridLayout= new Q3GridLayout(layout,2,3,5);
  gridLayout->setRowStretch(0,0);
  gridLayout->setRowStretch(1,100);
  lbl=new QLabel(i18n("&Maps"),page);
  mapListBox= new Q3ListBox(page);
  lbl->setBuddy(mapListBox);
  gridLayout->addWidget(lbl,0,0);
  gridLayout->addWidget(mapListBox,1,0);

  line= new Q3Frame(page);
  line->setFrameStyle(Q3Frame::VLine | Q3Frame::Sunken);
  line->setFixedWidth(10);
//	line->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  gridLayout->addWidget(line,1,1);

  lbl=new QLabel(i18n("Image Preview"),page);
  gridLayout->addWidget(lbl,0,2);

  imagePreview= new QLabel(page);
  imagePreview->setFixedSize(310,210);
  imagePreview->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  imagePreview->setFrameStyle(QLabel::Panel | QLabel::Sunken);
  imagePreview->setIndent(5);
  //imagePreview->setBackground(QBrush(QColor("white")));
//	imagePreview->setLineWidth(2);
//	imagePreview->setScaledContents(true);
//	lbl= new QLabel(i18n("&Maps"),page);
//	lbl->setBuddy(mapListBox);
  gridLayout->addWidget(imagePreview,1,2);
//	layout->addLayout(gridLayout,1);

  line= new Q3Frame(page);
  line->setFrameStyle(Q3Frame::HLine  | Q3Frame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line,0);


  if (maps->isEmpty()) {
    mapListBox->insertItem(i18n("No maps found"));
    mapListBox->setEnabled(false);
  }
  else {
    for (MapTag *tag = maps->first(); tag!=0L; tag=maps->next()) {
      mapListBox->insertItem(tag->name);
    }
    connect (mapListBox, SIGNAL(highlighted(int)), this, SLOT(slotMapChanged(int)));
  }

  initImageListTable(page);

  if (! maps->isEmpty()) {
    mapListBox->setCurrentItem(0);
    slotMapChanged(0);
  }

  resize(510,460);
}

void ImageMapChooseDialog::initImageListTable(QWidget* parent) {


  if (images->isEmpty()) {
    imageListTable= new Q3Table(1,1,parent);
    imageListTable->setText(0,0,i18n("No images found"));
    imageListTable->setEnabled(false);
    imageListTable->horizontalHeader()->hide();
    imageListTable->setTopMargin(0);
    imageListTable->setColumnStretchable(0,true);
  } else {
    imageListTable= new Q3Table(images->count(),2,parent);
    imageListTable->setColumnStretchable(0,true);
  }

  imageListTable->verticalHeader()->hide();
  imageListTable->setLeftMargin(0);

  QLabel *lbl= new QLabel(i18n("&Images"),parent);
  lbl->setBuddy(imageListTable);

  parent->layout()->add(lbl);
  parent->layout()->add(imageListTable);

  if (images->isEmpty())
    return;

  imageListTable->horizontalHeader()->setLabel(0,i18n("Path"));
  imageListTable->horizontalHeader()->setLabel(1,"usemap");

  imageListTable->setSelectionMode(Q3Table::SingleRow);
  imageListTable->setFocusStyle(Q3Table::FollowStyle);
  imageListTable->clearSelection(true);


  int row=0;
  for (ImageTag *tag = images->first(); tag!=0L; tag=images->next()) {
    QString src="";
    QString usemap="";
    if (tag->find("src"))
      src=*tag->find("src");
    if (tag->find("usemap"))
      usemap=*tag->find("usemap");

    imageListTable->setText(row,0,src);
    imageListTable->setText(row,1,usemap);
    row++;
  }
  connect (imageListTable, SIGNAL(selectionChanged()), this, SLOT(slotImageChanged()));

  imageListTable->selectRow(0);
  slotImageChanged();


}

ImageMapChooseDialog::~ImageMapChooseDialog() {
}

void ImageMapChooseDialog::slotImageChanged()
{
  int i=imageListTable->currentRow();
  QImage pix;
  if (images->at(i)->find("src")) {
    QString str=*images->at(i)->find("src");
    // relative url
    pixUrl=KUrl(baseUrl,str);
    pix=QImage(pixUrl.path());
    double zoom1=1;
    double zoom2=1;
    if (pix.width()>300)
      zoom1=(double) 300/pix.width();
    if (pix.height()>200)
      zoom2=(double) 200/pix.height();


    zoom1= zoom1 < zoom2 ? zoom1 : zoom2;
    pix=pix.smoothScale((int)(pix.width()*zoom1),int(pix.height()*zoom1));
  }
  QPixmap pix2;
  pix2.convertFromImage(pix);
  imagePreview->setPixmap(pix2);

//	imagePreview->repaint();
}

void ImageMapChooseDialog::selectImageWithUsemap(const QString & usemap) {
  for (int i=0; i<imageListTable->numRows(); i++) {
    if (imageListTable->text(i,1)==usemap) {
      imageListTable->selectRow(i);
      slotImageChanged();
      return;
    }
  }
}

void ImageMapChooseDialog::slotMapChanged(int i) {
  currentMap=maps->at(i);
  selectImageWithUsemap(currentMap->name);
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

  QLabel *lbl = new QLabel(i18n("&Maximum image preview height:")+" ",hbox);
  rowHeightSpinBox = new QSpinBox(hbox);
  lbl->setBuddy(rowHeightSpinBox);

  config->setGroup("Appearance");
  rowHeightSpinBox->setMaximum(1000);
  rowHeightSpinBox->setMinimum(15);
  rowHeightSpinBox->setFixedWidth(60);
  rowHeightSpinBox->setValue(config->readEntry("maximum-preview-height",50));

  config->setGroup("General");

  hbox= new KHBox(page);
  lbl = new QLabel(i18n("&Undo limit:")+" ",hbox);
  undoSpinBox = new QSpinBox(hbox);
  undoSpinBox->setFixedWidth(60);
  lbl->setBuddy(undoSpinBox);

  undoSpinBox->setMaximum(100);
  undoSpinBox->setMinimum(1);
  undoSpinBox->setValue(config->readEntry("undo-level",20));

  hbox= new KHBox(page);
  lbl = new QLabel(i18n("&Redo limit:")+" ",hbox);

  redoSpinBox = new QSpinBox(hbox);
  redoSpinBox->setFixedWidth(60);
  redoSpinBox->setMaximum(100);
  redoSpinBox->setMinimum(1);
  redoSpinBox->setValue(config->readEntry("redo-level",20));
  lbl->setBuddy(redoSpinBox);

  startWithCheck = new QCheckBox(i18n("&Start with last used document"),page);
  startWithCheck->setChecked(config->readEntry("start-with-last-used-document",true));

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
  config->setGroup("Appearance");
  config->writeEntry("maximum-preview-height",rowHeightSpinBox->cleanText().toInt());

  config->setGroup("General Options");
  config->writeEntry("undo-level",undoSpinBox->cleanText().toInt());
  config->writeEntry("redo-level",redoSpinBox->cleanText().toInt());
  config->writeEntry("start-with-last-used-document", startWithCheck->isChecked());

  config->sync();
  emit applyClicked();
}

HTMLPreviewDialog::HTMLPreviewDialog(QWidget* parent, KUrl url, const QString & htmlCode)
  : KDialog(parent)
{
  tempFile = new KTempFile(url.directory(false), ".html");
  setCaption(i18n("Preview"));
  setButtons(Ok);
  setDefaultButton(Ok);
  setModal(true);
  tempFile->setAutoDelete(true);
  (*tempFile->textStream()) << htmlCode;
  kDebug() << "HTMLPreviewDialog: TempFile : " << tempFile->name() << endl;
  tempFile->close();

  KVBox *page = new KVBox(this);
  setMainWidget(page);

  htmlPart = new KHTMLPart(page);
//  htmlView = new KHTMLView(htmlPart, page);
//  htmlView->setVScrollBarMode(QScrollView::Auto);
//  htmlView->setHScrollBarMode(QScrollView::Auto);
//  dialog->resize(dialog->calculateSize(edit->maxLineWidth(),edit->numLines()*));
//	dialog->adjustSize();
  QLabel* lbl = new QLabel(page,"urllabel");

  connect( htmlPart, SIGNAL( onURL(const QString&)), lbl, SLOT( setText(const QString&)));
}

HTMLPreviewDialog::~HTMLPreviewDialog() {
  delete tempFile;
  delete htmlPart;
}

void HTMLPreviewDialog::show() {
  KDialog::show();
  htmlPart->openURL(KUrl( tempFile->name() ));
//  htmlView->layout();
//  htmlView->repaint();
  resize(800,600);
}

#include "kimedialogs.moc"
