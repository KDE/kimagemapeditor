/***************************************************************************
                          imagemapchoosedialog.cpp  -  description
                            -------------------
    begin                : 06-03-2007
    copyright            : (C) 2007 by Jan Schäfer
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
#include "imagemapchoosedialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include <KConfigGroup>

#include "kimagemapeditor_debug.h"

ImageMapChooseDialog::ImageMapChooseDialog(
    QWidget* parent,
    QList<MapTag*> _maps,
    QList<ImageTag*> _images,
    const QUrl & _baseUrl)
  : QDialog(parent)
{
  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::ImageMapChooseDialog";
  if (parent == nullptr) {
    qCWarning(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog: parent is null!";
  }

  setWindowTitle(i18n( "Choose Map & Image to Edit" ));
  setModal(true);
  baseUrl = _baseUrl;
  maps = _maps;
  images = _images;
//  currentMap;
  setWindowTitle(baseUrl.fileName());
  QVBoxLayout *layout = new QVBoxLayout(this);

  QLabel *lbl = new QLabel(i18n("Select an image and/or a map that you want to edit"));
  lbl->setFont(QFont("Sans Serif",12, QFont::Bold));
  layout->addWidget(lbl);
  QFrame *line = new QFrame;
  line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line,0);

  QGridLayout *gridLayout = new QGridLayout;
  layout->addLayout(gridLayout);
  gridLayout->setRowStretch(0, 0);
  gridLayout->setRowStretch(1, 100);
  lbl = new QLabel(i18n("&Maps"));
  mapListBox = new QListWidget;
  lbl->setBuddy(mapListBox);
  gridLayout->addWidget(lbl, 0, 0);
  gridLayout->addWidget(mapListBox, 1, 0);

  line = new QFrame;
  line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
  line->setFixedWidth(10);
//	line->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  gridLayout->addWidget(line, 1, 1);

  lbl = new QLabel(i18n("Image Preview"));
  gridLayout->addWidget(lbl, 0, 2);

  imagePreview = new QLabel;
  imagePreview->setFixedSize(310,210);
  imagePreview->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding));
  imagePreview->setFrameStyle(QLabel::Panel | QLabel::Sunken);
  imagePreview->setIndent(5);
  //imagePreview->setBackground(QBrush(QColor("white")));
//	imagePreview->setLineWidth(2);
//	imagePreview->setScaledContents(true);
//	lbl = new QLabel(i18n("&Maps"),page);
//	layout->addWidget(lbl);
//	lbl->setBuddy(mapListBox);
  gridLayout->addWidget(imagePreview, 1, 2);
//	layout->addLayout(gridLayout,1);

  line = new QFrame;
  layout->addWidget(line);
  line->setFrameStyle(QFrame::HLine  | QFrame::Sunken);
  line->setFixedHeight(10);
  layout->addWidget(line, 0);


  if (maps.isEmpty()) {
    mapListBox->addItem(i18n("No maps found"));
    mapListBox->setEnabled(false);
  } else {
    for (int i = 0; i < maps.count(); i++) {
      mapListBox->addItem(maps.at(i)->name);
    }
    qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::ImageMapChooseDialog: before connect ";
    // UNSOLVED CRASH: connect(mapListBox, SIGNAL(currentRowChanged(int)), this, SLOT(slotMapChanged(int)));
  }

  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::ImageMapChooseDialog: before call initImageListTable ";
  initImageListTable(layout);

  if (! maps.isEmpty()) {
    mapListBox->setCurrentItem(nullptr);
    slotMapChanged(0);
  }

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  okButton->setDefault(true);
  layout->addWidget(buttonBox);

  resize(510,460);
}

void ImageMapChooseDialog::initImageListTable(QLayout* layout) {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::initImageListTable ";


  if (images.isEmpty()) {
    imageListTable = new QTableWidget(1, 1);
    imageListTable->setItem(0,0, new QTableWidgetItem(i18n("No images found")));
    imageListTable->setEnabled(false);
    imageListTable->horizontalHeader()->hide();
    // PORT: imageListTable->setTopMargin(0);
    // PORT: imageListTable->setColumnStretchable(0,true);
  } else {
    imageListTable = new QTableWidget(images.count(), 2);
    // PORT: imageListTable->setColumnStretchable(0,true);
  }

  imageListTable->verticalHeader()->hide();
  // PORT imageListTable->setLeftMargin(0);

  QLabel *lbl = new QLabel(i18n("&Images"));
  lbl->setBuddy(imageListTable);

  layout->addWidget(lbl);
  layout->addWidget(imageListTable);

  if (images.isEmpty())
    return;

  imageListTable->setHorizontalHeaderLabels(QStringList() << i18n("Path") << "usemap");

  imageListTable->setSelectionMode(QAbstractItemView::SingleSelection);
  // PORT:  imageListTable->setFocusStyle(QTableWidget::FollowStyle);
  imageListTable->clearSelection();


  int row=0;
  QListIterator<ImageTag*> it(images);
  while (it.hasNext()) {
    QString src="";
    QString usemap="";
    ImageTag* tag = it.next();
    if (tag->contains("src"))
      src=tag->value("src");
    if (tag->contains("usemap"))
      usemap=tag->value("usemap");

    imageListTable->setItem(row,0, new QTableWidgetItem(src));
    imageListTable->setItem(row,1, new QTableWidgetItem(usemap));
    row++;
  }

  // UNSOLVED CRASH: connect (imageListTable, SIGNAL(itemSelectionChanged()), 
  // this, SLOT(slotImageChanged()));

  imageListTable->selectRow(0);
  slotImageChanged();


}

ImageMapChooseDialog::~ImageMapChooseDialog() {
}

void ImageMapChooseDialog::slotImageChanged()
{
  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::slotImageChanged";
  int i=imageListTable->currentRow();
  if (i < 0 || i > images.count()) 
    i = 0;
  QImage pix;
  if (images.at(i)->contains("src")) {
    QString str = images.at(i)->value("src");
    // relative url
    if (baseUrl.path().isEmpty() | !baseUrl.path().endsWith('/')) {
        pixUrl=QUrl(baseUrl.path() + '/').resolved(QUrl(str));
    }
    else {
        pixUrl=baseUrl.resolved(QUrl(str));
    }
    pix=QImage(pixUrl.path());
    double zoom1=1;
    double zoom2=1;
    if (pix.width()>300)
      zoom1=(double) 300/pix.width();
    if (pix.height()>200)
      zoom2=(double) 200/pix.height();


    zoom1= zoom1 < zoom2 ? zoom1 : zoom2;
    pix=pix.scaled((int)(pix.width()*zoom1),
		   int(pix.height()*zoom1),
		   Qt::KeepAspectRatio,
		   Qt::SmoothTransformation);
  }
  QPixmap pix2;
  pix2.fromImage(pix);
  imagePreview->setPixmap(pix2);

//	imagePreview->repaint();
}


void ImageMapChooseDialog::selectImageWithUsemap(const QString & usemap) {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::selectImageWithUsemap: " << usemap;

  for (int i=0; i<imageListTable->rowCount(); i++) {
    QTableWidgetItem *item = imageListTable->item(i,1);
    if (item && (item->text()==usemap)) {
      imageListTable->selectRow(i);
      slotImageChanged();
      return;
    }
  }
}

void ImageMapChooseDialog::slotMapChanged(int i) {
  qCDebug(KIMAGEMAPEDITOR_LOG) << "ImageMapChooseDialog::slotMapChanged: " << i;
  currentMap=maps.at(i);
  selectImageWithUsemap(currentMap->name);
}

