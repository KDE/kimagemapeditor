/***************************************************************************
                          main.cpp  -  description
                            -------------------
    begin                : Die Apr 10 19:46:49 CEST 2001
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>

#include "kimeshell.h"
#include "version.h"

static const char *description =
  I18N_NOOP("An HTML imagemap editor");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{

  KAboutData aboutData( "kimagemapeditor", 0, ki18n("KImageMapEditor"),
    KIMAGEMAPEDITOR_VERSION, ki18n(description), KAboutData::License_GPL,
    ki18n("(c) 2001-2007 Jan Schaefer"), KLocalizedString(), "http://www.nongnu.org/kimagemap/", "janschaefer@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Jan Schaefer"),KLocalizedString(), "janschaefer@users.sourceforge.net");
  aboutData.addCredit(ki18n("Joerg Jaspert"),ki18n("For helping me with the Makefiles, and creating the Debian package"));
  aboutData.addCredit(ki18n("Aaron Seigo and Michael"),ki18n("For helping me fixing --enable-final mode"));
  aboutData.addCredit(ki18n("Antonio Crevillen"),ki18n("For the Spanish translation"));
  aboutData.addCredit(ki18n("Fabrice Mous"),ki18n("For the Dutch translation"));
  aboutData.addCredit(ki18n("Germain Chazot"),ki18n("For the French translation"));
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("c");
  options.add("stdout", ki18n("Write HTML-Code to stdout on exit"));
  options.add("+[File]", ki18n("File to open"));
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.



  KApplication a;

  if (a.isSessionRestored())
  {
      RESTORE(KimeShell);
  }
  else
  {
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
      if ( args->count() == 0 )
      {
        KimeShell *kimeShell = new KimeShell();
        kimeShell->setStdout(args->isSet("stdout"));
        kimeShell->readConfig();
        kimeShell->show();
        kimeShell->openLastFile();
      }
      else
      {
          int i = 0;
          for (; i < args->count(); i++ )
          {
            KimeShell *kimeShell = new KimeShell();
            kimeShell->setStdout(args->isSet("stdout"));
            kimeShell->readConfig();
            kimeShell->show();
            kimeShell->openFile(args->url(i));
          }
      }
      args->clear();
  }           

  return a.exec();
}
