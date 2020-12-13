/***************************************************************************
                          kimeshell.h  -  description
                             -------------------
    begin                : Mon Aug 5 2002
    copyright            : (C) 2002 by Jan Schäfer
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

#ifndef KIMESHELL_H
#define KIMESHELL_H

#include <KParts/MainWindow>
#include <KConfigGroup>
#include <QUrl>

namespace KParts {
class ReadWritePart;
}
class KImageMapEditorInterface;

class KimeShell : public KParts::MainWindow
{
  Q_OBJECT

public:
  explicit KimeShell( const char *name=nullptr );
  ~KimeShell() override;

  void setStdout(bool b);
  void openFile(const QUrl & url);

  /**
   * Opens the last open file, if the
   * user has configured to open the last
   * file. Otherwise does nothing
   */
  void openLastFile();
  void readConfig();
  void writeConfig();

protected:
  void setupActions();
  void readConfig(const KConfigGroup&);
  void writeConfig(KConfigGroup&);

//  virtual bool queryClose();
  void readProperties(const KConfigGroup &config) override;
  void saveProperties(KConfigGroup &config) override;

  bool queryClose() override;
  virtual bool queryExit();


private slots:
  void fileNew();
  void fileOpen();
  void optionsShowToolbar();
  void optionsShowStatusbar();
  void optionsConfigureKeys();
  void optionsConfigureToolbars();

  void applyNewToolbarConfig();

private:
  KParts::ReadWritePart *m_part;
  KImageMapEditorInterface *m_editorInterface;

	bool _stdout; // write HTML-Code to stdout on exit ?



};

inline void KimeShell::setStdout(bool b) {
	_stdout=b;
}


#endif
