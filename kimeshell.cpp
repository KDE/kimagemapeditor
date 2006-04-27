/***************************************************************************
                          kimepart.cpp  -  description
                             -------------------
    begin                : Mon Aug 5 2002
    copyright            : (C) 2002 by Jan Sch�er
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

#include <kaction.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kdebug.h>

#include <q3hbox.h>

#include "drawzone.h"
#include "kimagemapeditor.h"	// the KPart
#include "kimeshell.h"
#include "kimeshell.moc"

KimeShell::KimeShell(const char *name )
  : KParts::DockMainWindow( 0L, name )
{
	setXMLFile("kimagemapeditorui.rc");


  KDockWidget* mainDock;
  mainDock = createDockWidget( "MainDockWidget", 0L, 0L, "main_dock_widget");
  QWidget *w = new Q3HBox( mainDock );
//  QLayout* layout = new QGridLayout( mainDock );

  mainDock->setWidget( w );
  // allow others to dock to the 4 sides
  mainDock->setDockSite(KDockWidget::DockCorner);
  // forbit docking abilities of mainDock itself
  mainDock->setEnableDocking(KDockWidget::DockNone);
  setView( mainDock); // central widget in a KDE mainwindow
  setMainDockWidget( mainDock); // master dockwidget
  m_part = new KImageMapEditor( w, "kimagemapeditor", this, "kimagemapeditor");


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
		std::cout << m_part->getHtmlCode() << std::endl;
	}

  return m_part->queryClose();
}


bool KimeShell::queryExit()
{
//  writeConfig();
  saveProperties(kapp->config());

  return true;
}


void KimeShell::setupActions()
{
  (void)KStdAction::openNew(this, SLOT(fileNew()), actionCollection());

	// File Quit
	(void)KStdAction::quit(this, SLOT(close()),actionCollection());


	(void)KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
  (void)KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	(void)KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
  (void)KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());


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
  if (m_part->config()->readBoolEntry("start-with-last-used-document",true))
     m_part->openLastURL(m_part->config());
}

void KimeShell::fileOpen()
{
  KUrl url=KFileDialog::getOpenURL(QString::null,
          "*.png *.jpg *.jpeg *.gif *.htm *.html|" + i18n( "Web Files" ) + "\n"
          "*.png *.jpg *.jpeg *.gif *.bmp *.xbm *.xpm *.pnm *.mng|" + i18n( "Images" ) + "\n"
          "*.htm *.html|" + i18n( "HTML Files" ) + "\n"
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
  KConfig *config;

  config = kapp->config();

  config->setGroup("General Options");
  readConfig(config);

}

void KimeShell::readConfig(KConfig* config) {
//	applyMainWindowSettings(config);
//	restoreWindowSize(config);
  readDockConfig(config);
}

void KimeShell::writeConfig() {
  KConfig *config;

  config = kapp->config();

  config->setGroup("General Options");
  writeConfig(config);
}

void KimeShell::writeConfig(KConfig* config) {
	saveMainWindowSettings(config);
	saveWindowSize(config);
  writeDockConfig(config);
  config->sync();

}


void KimeShell::saveProperties(KConfig *config)
{
  //writeConfig(config);
  m_part->saveProperties(config);
  writeConfig();

}

void KimeShell::readProperties(KConfig *config)
{
  readConfig();
  m_part->readProperties(config);


}


void KimeShell::optionsConfigureKeys() {
//  KKeyDialog::configureKeys(actionCollection(), "testprog_shell.rc");

  KKeyDialog dlg;
  dlg.insert(actionCollection());
  dlg.insert(m_part->actionCollection());
  dlg.configure();
}

void KimeShell::optionsConfigureToolbars()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config() );
# endif
#else
    saveMainWindowSettings(KGlobal::config() );
#endif

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void KimeShell::applyNewToolbarConfig()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
#endif
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


