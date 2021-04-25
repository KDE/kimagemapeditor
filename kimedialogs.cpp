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

// LOCAL
#include "kimedialogs.h"

// Qt
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QLinkedList>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QVBoxLayout>

// KDE Frameworks
#include "kimagemapeditor_debug.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

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
  QFormLayout *layout= new QFormLayout(this);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(INT_MAX);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  connect( topXSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Top &X:"), topXSpin);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(INT_MAX);
  topYSpin->setMinimum(0);
  topYSpin->setValue(a->rect().top());
  connect( topYSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Top &Y:"), topYSpin);

  widthSpin = new QSpinBox(this);
  widthSpin->setMaximum(INT_MAX);
  widthSpin->setMinimum(0);
  widthSpin->setValue(a->rect().width());
  connect( widthSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("&Width:"), widthSpin);

  heightSpin = new QSpinBox(this);
  heightSpin->setMaximum(INT_MAX);
  heightSpin->setMinimum(0);
  heightSpin->setValue(a->rect().height());
  connect( heightSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Hei&ght:"), heightSpin);
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
  QFormLayout *layout = new QFormLayout(this);

  centerXSpin = new QSpinBox(this);
  centerXSpin->setMaximum(INT_MAX);
  centerXSpin->setMinimum(0);
  centerXSpin->setValue(a->rect().center().x());
  connect( centerXSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Center &X:"), centerXSpin);

  centerYSpin = new QSpinBox(this);
  centerYSpin->setMaximum(INT_MAX);
  centerYSpin->setMinimum(0);
  centerYSpin->setValue(a->rect().center().y());
  connect( centerYSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Center &Y:"), centerYSpin);

  radiusSpin = new QSpinBox(this);
  radiusSpin->setMaximum(INT_MAX);
  radiusSpin->setMinimum(0);
  radiusSpin->setValue(a->rect().width()/2);
  connect( radiusSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("&Radius:"), radiusSpin);

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
  QVBoxLayout *layout = new QVBoxLayout(this);

  coordsTable = new QTableWidget(0, 2);
  coordsTable->verticalHeader()->hide();
  coordsTable->setSelectionMode( QTableWidget::SingleSelection );
  connect( coordsTable, SIGNAL(currentChanged(int,int)), this, SLOT(slotHighlightPoint(int)));

  updatePoints();
//	coordsTable->setMinimumHeight(50);
//	coordsTable->setMaximumHeight(400);
//	coordsTable->resizeContents(100,100);
  coordsTable->resize(coordsTable->width(), 100);
  layout->addWidget(coordsTable);
  layout->setStretchFactor(coordsTable, -1);

  QHBoxLayout *hbox = new QHBoxLayout;
  QPushButton *addBtn = new QPushButton(i18n("Add"));
  hbox->addWidget(addBtn);
  connect( addBtn, SIGNAL(pressed()), this, SLOT(slotAddPoint()));
  QPushButton *removeBtn = new QPushButton(i18n("Remove"));
  hbox->addWidget(removeBtn);
  connect( removeBtn, SIGNAL(pressed()), this, SLOT(slotRemovePoint()));

  layout->addLayout(hbox);

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
  QFormLayout *layout = new QFormLayout(this);

  topXSpin = new QSpinBox(this);
  topXSpin->setMaximum(INT_MAX);
  topXSpin->setMinimum(0);
  topXSpin->setValue(a->rect().left());
  connect( topXSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));
  layout->addRow(i18n("Top &X"), topXSpin);

  topYSpin = new QSpinBox(this);
  topYSpin->setMaximum(INT_MAX);
  topYSpin->setMinimum(0);
  topYSpin->setValue(a->rect().top());
  connect( topYSpin, SIGNAL(valueChanged(QString)), this, SLOT(slotTriggerUpdate()));

  layout->addRow(i18n("Top &Y"), topYSpin);
}

void SelectionCoordsEdit::applyChanges() {
  area->moveTo(topXSpin->text().toInt(), topYSpin->text().toInt());
}



QLineEdit* AreaDialog::createLineEdit(QFormLayout *layout, const QString &value, const QString &name)
{
  QLineEdit *edit = new QLineEdit(value);
  layout->addRow(name, edit);
  return edit;
}

QWidget* AreaDialog::createGeneralPage()
{
  QFrame *page = new QFrame(this);
  QFormLayout *layout = new QFormLayout(page);

  // A separate widget, not just a layout, is needed so that
  // the accelerator for the row is working
  QWidget *hbox = new QWidget;
  QHBoxLayout *hboxLayout = new QHBoxLayout(hbox);
  hboxLayout->setContentsMargins(0, 0, 0, 0);
  hrefEdit = new QLineEdit(area->attribute("href"));
  hboxLayout->addWidget(hrefEdit);
  QPushButton *btn = new QPushButton;
  btn->setIcon(QIcon::fromTheme("document-open"));
  connect( btn, SIGNAL(pressed()), this, SLOT(slotChooseHref()));
  hboxLayout->addWidget(btn);

  QLabel *lblHREF = new QLabel(i18n("&HREF:"));
  lblHREF->setBuddy(hrefEdit);
  layout->addRow(lblHREF, hbox);

  altEdit = createLineEdit(layout,
			   area->attribute("alt"),
			   i18n("Alt. &Text:"));
  targetEdit = createLineEdit(layout,
			      area->attribute("target"),
			      i18n("Tar&get:"));
  titleEdit = createLineEdit(layout,
			     area->attribute("title"),
			     i18n("Tit&le:"));

  if (area->type() == Area::Default) {
    defaultAreaChk = new QCheckBox(i18n("On"));
    if (area->finished()) {
      defaultAreaChk->setChecked(true);
    }
    layout->addRow(i18n("Enable default map"), defaultAreaChk);
  }

  return page;
}

QWidget* AreaDialog::createCoordsPage()
{
  QFrame* page = new QFrame(this);
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(5, 5, 5, 5);

  coordsEdit = createCoordsEdit(page, area);
  layout->addWidget(coordsEdit);
  connect( coordsEdit, SIGNAL(update()), this, SLOT(slotUpdateArea()));

  return page;
}

QWidget* AreaDialog::createJavascriptPage()
{
  QFrame *page = new QFrame(this);
  QFormLayout *layout = new QFormLayout(page);

  onClickEdit = createLineEdit(layout, area->attribute("onClick"), i18n("OnClick:"));
  onDblClickEdit = createLineEdit(layout, area->attribute("onDblClick"), i18n("OnDblClick:"));
  onMouseDownEdit = createLineEdit(layout, area->attribute("onMouseDown"), i18n("OnMouseDown:"));
  onMouseUpEdit = createLineEdit(layout, area->attribute("onMouseUp"), i18n("OnMouseUp:"));
  onMouseOverEdit = createLineEdit(layout, area->attribute("onMouseOver"), i18n("OnMouseOver:"));
  onMouseMoveEdit = createLineEdit(layout, area->attribute("onMouseMove"), i18n("OnMouseMove:"));
  onMouseOutEdit = createLineEdit(layout, area->attribute("onMouseOut"), i18n("OnMouseOut:"));

  return page;
}

AreaDialog::AreaDialog(KImageMapEditor* parent, Area* a)
  : QDialog(parent->widget())
{
  setWindowTitle(i18n("Area Tag Editor"));
  //  setFaceType( KPageDialog::Tabbed );
  setObjectName( "Area Tag Editor" );
  setModal(true);

  _document = parent;

  if (!a) {
      slotCancel();
      return;
  }

  area = a;
  QString shape("Default");
  areaCopy = a->clone();
  oldArea = new Area();
  oldArea->setRect( a->rect() );

  switch (a->type()) {
    case Area::Rectangle: shape = i18n("Rectangle"); break;
    case Area::Circle: shape = i18n("Circle"); break;
    case Area::Polygon: shape = i18n("Polygon"); break;
    case Area::Selection: shape = i18n("Selection"); break;
    default: break;
  }

  QVBoxLayout *layout = new QVBoxLayout(this);

  // To get a margin around everything
  layout->setContentsMargins(5, 5, 5, 5);

  QLabel *lbl = new QLabel("<b>"+shape+"</b>");
  lbl->setTextFormat(Qt::RichText);
  layout->addWidget(lbl);

  QFrame *line = new QFrame;
  line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line);

  QTabWidget *tab = new QTabWidget;
  tab->addTab(createGeneralPage(), i18n("&General"));
  layout->addWidget(tab);

  if (a->type() == Area::Default) {
    // FIXME? Why this useless assignment?
    shape = i18n("Default");
  } else {
    tab->addTab(createCoordsPage(), i18n("Coor&dinates"));
  }
  tab->addTab(createJavascriptPage(), i18n("&JavaScript"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Apply);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  layout->addWidget(buttonBox);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCancel()));
  connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()));

  setMinimumHeight(360);
  setMinimumWidth(327);

  resize(327,360);
}

AreaDialog::~AreaDialog() {
  delete areaCopy;
  delete oldArea;
}

CoordsEdit* AreaDialog::createCoordsEdit(QWidget *parent, Area *a) {
  if (!a) return nullptr;
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
  QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Choose File"), QUrl(), i18n("All Files (*)"));
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
    area->setAttribute("onclick",onClickEdit->text());
    area->setAttribute("ondblclick",onDblClickEdit->text());
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
    AreaSelection *selection = nullptr;
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
  : QDialog(parent)
{
  config = conf;
  setWindowTitle(i18n("Preferences"));
  setModal(true);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QFormLayout *optionsLayout = new QFormLayout;
  mainLayout->addLayout(optionsLayout);

  rowHeightSpinBox = new QSpinBox;
  int maxPrevHeight = config->group("Appearance").readEntry("maximum-preview-height",50);
  rowHeightSpinBox->setMaximum(1000);
  rowHeightSpinBox->setMinimum(15);
  rowHeightSpinBox->setFixedWidth(60);
  rowHeightSpinBox->setValue(maxPrevHeight);
  optionsLayout->addRow(i18n("&Maximum image preview height:"), rowHeightSpinBox);

  KConfigGroup general = config->group("General");

  undoSpinBox = new QSpinBox;
  undoSpinBox->setFixedWidth(60);
  undoSpinBox->setMaximum(100);
  undoSpinBox->setMinimum(1);
  undoSpinBox->setValue(general.readEntry("undo-level",20));
  optionsLayout->addRow(i18n("&Undo limit:"), undoSpinBox);

  redoSpinBox = new QSpinBox;
  redoSpinBox->setFixedWidth(60);
  redoSpinBox->setMaximum(100);
  redoSpinBox->setMinimum(1);
  redoSpinBox->setValue(general.readEntry("redo-level",20));
  optionsLayout->addRow(i18n("&Redo limit:"), redoSpinBox);

  startWithCheck = new QCheckBox(i18n("On"));
  startWithCheck->setChecked(general.readEntry("start-with-last-used-document",true));
  optionsLayout->addRow(i18n("&Start with last used document"), startWithCheck);

/*
  colorizeAreaChk = new QCheckBox(i18n("On"));
  colorizeAreaChk->setFixedWidth(60);
  colorizeAreaChk->setChecked(KSharedConfig::openConfig()->readEntry("highlightareas",true));
  optionsLayout->addRow(i18n("Highlight Areas"), colorizeAreaChk);

  showAltChk = new QCheckBox(i18n("On"));
  showAltChk->setFixedWidth(60);
  showAltChk->setChecked(KSharedConfig::openConfig()->readEntry("showalt",true));
  optionsLayout->addRow(i18n("Show alternative text"), showAltChk);
*/

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Apply);
  mainLayout->addWidget(buttonBox);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(buttonBox->button(QDialogButtonBox::Apply),SIGNAL(clicked()),this,SLOT(slotApply()));

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

HTMLPreviewDialog::HTMLPreviewDialog(QWidget* parent, const QString & htmlCode)
  : QDialog(parent)
{
  tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/kime_preview_XXXXXX.html"));
  tempFile->open();
  setWindowTitle(i18n("Preview"));
  setModal(true);
  QTextStream stream(tempFile);
  stream << htmlCode;
  qCDebug(KIMAGEMAPEDITOR_LOG) << "HTMLPreviewDialog: TempFile : " << tempFile->fileName();
  stream.flush();

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  htmlPart = new QWebEngineView;
  mainLayout->addWidget(htmlPart);
//  htmlView = new KHTMLView(htmlPart, page);
//  mainLayout->addWidget(htmlView);
//  htmlView->setVScrollBarMode(QScrollView::Auto);
//  htmlView->setHScrollBarMode(QScrollView::Auto);
//  dialog->resize(dialog->calculateSize(edit->maxLineWidth(),edit->numLines()*));
//	dialog->adjustSize();
  htmlPart->load(QUrl::fromLocalFile(tempFile->fileName()));
  QLabel *lbl = new QLabel;
  lbl->setObjectName( "urllabel" );
  mainLayout->addWidget(lbl);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  mainLayout->addWidget(buttonBox);

  connect( htmlPart->page(), &QWebEnginePage::linkHovered, lbl, &QLabel::setText);

  resize(800,600);
}

HTMLPreviewDialog::~HTMLPreviewDialog() {
  delete tempFile;
}

