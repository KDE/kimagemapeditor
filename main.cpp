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
#include <config.h>

static const char *description =
  I18N_NOOP("An HTML imagemap editor");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  { "c", 0, 0 },
  { "stdout", I18N_NOOP("Write HTML-Code to stdout on exit"), 0 },
  { "+[File]", I18N_NOOP("File to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "kimagemapeditor", I18N_NOOP("KImageMapEditor"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2001-2003 Jan Schaefer", 0, "http://www.nongnu.org/kimagemap/", "janschaefer@users.sourceforge.net");
  aboutData.addAuthor("Jan Schaefer",0, "janschaefer@users.sourceforge.net");
  aboutData.addCredit("Joerg Jaspert",I18N_NOOP("For helping me with the Makefiles, and creating the Debian package"));
  aboutData.addCredit("Aaron Seigo and Michael",I18N_NOOP("For helping me fixing --enable-final mode"));
  aboutData.addCredit("Antonio Crevillen",I18N_NOOP("For the Spanish translation"));
  aboutData.addCredit("Fabrice Mous",I18N_NOOP("For the Dutch translation"));
  aboutData.addCredit("Germain Chazot",I18N_NOOP("For the French translation"));
  KCmdLineArgs::init( argc, argv, &aboutData );
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
