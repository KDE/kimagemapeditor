/***************************************************************************
                          kimepart.cpp  -  description
                             -------------------
    begin                : Mon Aug 5 2002
    copyright            : (C) 2002 by Jan SchÃ¯Â¿Â½er
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


#include <iostream>

#include <QDockWidget>
#include <QScrollArea>

#include <kaction.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <kstandardaction.h>

#include <khbox.h>
#include <kglobal.h>
#include <ktoolbar.h>
#include <kvbox.h>

#include "drawzone.h"
#include "kimagemapeditor.h"	// the KPart
#include "kimeshell.h"
#include "kimeshell.moc"
#include <kactioncollection.h>
KimeShell::KimeShell(const char * )
  : KParts::MainWindow()
{
  setXMLFile("kimagemapeditorui.rc");

  //  QDockWidget* mainDock = new QDockWidget(this);
  //  mainDock = createDockWidget( "MainDockWidget", 0L, 0L, "main_dock_widget");
  //  QWidget *mainWidget = new KHBox( this );
  //  QScrollArea* mainWidget = new QScrollArea(this);
  //  setCentralWidget(mainWidget);
//  QLayout* layout = new QGridLayout( mainDock );

//  mainDock->setWidget( w );
  // allow others to dock to the 4 sides
  //  mainDock->setDockSite(K3DockWidget::DockCorner);
  // forbit docking abilities of mainDock itself
  //  mainDock->setEnableDocking(K3DockWidget::DockNone);
  //  setView( mainDock); // central widget in a KDE mainwindow
  //  setMainDockWidget( mainDock); // master dockwidget
  m_part = new KImageMapEditor((QWidget*)0L, this);

//	setCentralWidget( part->widget() );

  setupActions();

	_stdout=false;

//  createGUI( part );
	createShellGUI( true );
  guiFactory()->addClient( m_part );
  KParts::GUIActivateEvent ev( true );
  QApplication::sendEvent( m_part, &ev );
  //setCentralWidget(part->widget());

  if (!initialGeometrySet())
    resize( QSize(725, 525).expandedTo(minimumSizeHint()));

  connect( m_part, SIGNAL(setStatusBarText(const QString &)),
           this, SLOT(slotSetStatusBarText ( const QString & )));

  connect( m_part, SIGNAL(setWindowCaption(const QString &)),
           this, SLOT(setCaption( const QString &)));

  setAutoSaveSettings( "General Options" );

}

KimeShell::~KimeShell()
{
//  delete part;
}

bool KimeShell::queryClose()
{
	if (_stdout) {
//FIXME 		std::cout << m_part->getHtmlCode() << std::endl;
	}

  return m_part->queryClose();
}


bool KimeShell::queryExit()
{
//  writeConfig();
#warning what group is correct here? A random one?
  KConfigGroup cg( KGlobal::config(), QString() );
  saveProperties( cg );

  return true;
}


void KimeShell::setupActions()
{
  (void)KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());

	// File Quit
	(void)KStandardAction::quit(this, SLOT(close()),actionCollection());


//FIXME	(void)KStandardAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
  (void)KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	(void)KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
  (void)KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());


}

void KimeShell::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here..
    if ( ! m_part->url().isEmpty() || m_part->isModified() )
    {
        KimeShell * newShell = new KimeShell();

        newShell->show();
        newShell->readConfig();
    };
}

void KimeShell::openFile(const KUrl & url)
{
	m_part->openFile(url);
}

void KimeShell::openLastFile()
{
#warning there is no group defined
    KConfigGroup cg( m_part->config(), QString() );
    if (cg.readEntry("start-with-last-used-document",true))
        m_part->openLastURL( cg );
}

void KimeShell::fileOpen()
{
  KUrl url=KFileDialog::getOpenUrl(KUrl(),
          "*.png *.jpg *.jpeg *.gif *.htm *.html|" + i18n( "Web Files" ) + '\n' +
          "*.png *.jpg *.jpeg *.gif *.bmp *.xbm *.xpm *.pnm *.mng|" + i18n( "Images" ) + '\n' +
          "*.htm *.html|" + i18n( "HTML Files" ) + '\n' +
          "*.png|" + i18n( "PNG Images" ) + "\n*.jpg *.jpeg|" + i18n( "JPEG Images" ) + "\n*.gif|" + i18n( "GIF Images" ) + "\n*|" + i18n( "All Files" )
          ,this,i18n("Choose Picture to Open"));

  if (!url.isEmpty()) {
        // About this function, the style guide (
        // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        if ( m_part->url().isEmpty() && ! m_part->isModified() )
        {
            // we open the file in this window...
            m_part->openURL(url);
        }
        else
        {
            // we open the file in a new window...
            KimeShell* newWin = new KimeShell;
            newWin->openFile( url );
            newWin->show();
        }
  }
}



void KimeShell::readConfig() {
  KSharedConfigPtr config = KGlobal::config();
  readConfig(config->group("General Options") );
}

void KimeShell::readConfig(const KConfigGroup &) {
//	applyMainWindowSettings(config);
//	restoreWindowSize(config);
//  readDockConfig(config);
}

void KimeShell::writeConfig() {
  KConfigGroup config( KGlobal::config(), "General Options");
  writeConfig( config );
}

void KimeShell::writeConfig(KConfigGroup &config) {
	saveMainWindowSettings(config);
	saveWindowSize(config);
	//  writeDockConfig(config);
  config.sync();

}


void KimeShell::saveProperties(KConfigGroup &config)
{
  //writeConfig(config);
  m_part->saveProperties(config);
  writeConfig();

}

void KimeShell::readProperties(const KConfigGroup &config)
{
  readConfig();
  m_part->readProperties(config);


}


void KimeShell::optionsConfigureKeys() {
//  KShortcutsDialog::configureKeys(actionCollection(), "testprog_shell.rc");

  KShortcutsDialog dlg;
  dlg.addCollection(actionCollection());
  dlg.addCollection(m_part->actionCollection());
  dlg.configure();
}

void KimeShell::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void KimeShell::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );
}


void KimeShell::optionsShowToolbar()
{
	if (toolBar()->isVisible())
		toolBar()->hide();
	else
		toolBar()->show();
}

void KimeShell::optionsShowStatusbar()
{
	if (statusBar()->isVisible())
		statusBar()->hide();
	else
		statusBar()->show();
}


