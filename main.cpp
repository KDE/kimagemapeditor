/***************************************************************************
                          main.cpp  -  description
                            -------------------
    begin                : Die Apr 10 19:46:49 CEST 2001
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

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>

#include <KAboutData>
#include <KLocalizedString>

#include "kimeshell.h"
#include "kimagemapeditor_version.h"

static const char *description =
  I18N_NOOP("An HTML imagemap editor");


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  KLocalizedString::setApplicationDomain("kimagemapeditor");

  KAboutData aboutData( "kimagemapeditor", i18n("KImageMapEditor"),
    KIMAGEMAPEDITOR_VERSION_STRING, i18n(description), KAboutLicense::GPL,
    i18n("(c) 2001-2007 Jan Schaefer"), QString(),
    QStringLiteral("https://kde.org/applications/development/org.kde.kimagemapeditor"), QStringLiteral("janschaefer@users.sourceforge.net"));
  aboutData.addAuthor(i18n("Jan Schaefer"),QString(), "janschaefer@users.sourceforge.net");
  aboutData.addCredit(i18n("Joerg Jaspert"),i18n("For helping me with the Makefiles, and creating the Debian package"));
  aboutData.addCredit(i18n("Aaron Seigo and Michael"),i18n("For helping me fixing --enable-final mode"));
  aboutData.addCredit(i18n("Antonio Crevillen"),i18n("For the Spanish translation"));
  aboutData.addCredit(i18n("Fabrice Mous"),i18n("For the Dutch translation"));
  aboutData.addCredit(i18n("Germain Chazot"),i18n("For the French translation"));

  aboutData.setOrganizationDomain("kde.org");
  aboutData.setDesktopFileName(QStringLiteral("org.kde.kimagemapeditor"));
  KAboutData::setApplicationData(aboutData);

  app.setOrganizationName(QStringLiteral("KDE"));

  app.setWindowIcon(QIcon::fromTheme("kimagemapeditor", app.windowIcon()));

  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);

  parser.addOption(QCommandLineOption(QStringList() << QLatin1String("c") << QLatin1String("stdout"), i18n("Write HTML-Code to stdout on exit")));
  parser.addPositionalArgument(QLatin1String("[File]"), i18n("File to open"));
  parser.process(app);
  aboutData.processCommandLine(&parser);

  if (app.isSessionRestored())
  {
      kRestoreMainWindows<KimeShell>();
  }
  else
  {
      if ( parser.positionalArguments().count() == 0 )
      {
        KimeShell *kimeShell = new KimeShell();
        kimeShell->setStdout(parser.isSet("stdout"));
        kimeShell->readConfig();
        kimeShell->show();
        kimeShell->openLastFile();
      }
      else
      {
          int i = 0;
          for (; i < parser.positionalArguments().count(); i++ )
          {
            KimeShell *kimeShell = new KimeShell();
            kimeShell->setStdout(parser.isSet("stdout"));
            kimeShell->readConfig();
            kimeShell->show();
            kimeShell->openFile(QUrl::fromUserInput(parser.positionalArguments().at(i), QDir::currentPath(), QUrl::AssumeLocalFile));
          }
      }

  }

  return app.exec();
}
