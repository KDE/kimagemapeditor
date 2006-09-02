/*
    From WebMaker - KDE HTML Editor
    Copyright (C) 1998, 1999 Alexei Dets <dets@services.ru>
    Rewritten for Quanta Plus: (C) 2002 Andras Mantia <amantia@freemail.hu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef _QEXTFILEINFO_H_
#define _QEXTFILEINFO_H_

#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>
#include <kfileitem.h>

#include <qobject.h>
#include <q3ptrlist.h>
#include <qregexp.h>

class QExtFileInfo:public QObject
{
 Q_OBJECT
public:
  QExtFileInfo() {};
  ~QExtFileInfo() {};

  /** create to ralative short name */
  static KUrl toRelative(const KUrl& urlToConvert,const KUrl& baseURL);
  /** convert relative filename to absolute */
  static KUrl toAbsolute(const KUrl& urlToConvert,const KUrl& baseURL);
  /** recurse function for all files in dir */
  static KUrl::List allFiles( const KUrl& path, const QString &mask);
  static KUrl::List allFilesRelative( const KUrl& path, const QString &mask);
  /** create dir if don't exists */
  static bool createDir(const KUrl & path );
  static KUrl cdUp(const KUrl &dir);
  static QString shortName(const QString &fname );
  static KUrl path(const KUrl &);
  static KUrl home();
  static bool exists(const KUrl& url);
  static bool copy( const KUrl& src, const KUrl& dest, int permissions=-1,
                    bool overwrite=false, bool resume=false, QWidget* window = 0L );

  void enter_loop();

private:
  bool internalExists(const KUrl& url);
  bool internalCopy(const KUrl& src, const KUrl& target, int permissions,
                    bool overwrite, bool resume, QWidget* window);

  bool bJobOK;
  static QString lastErrorMsg;
  KIO::UDSEntry m_entry;
  KUrl::List dirListItems;
  Q3PtrList<QRegExp> lstFilters;

  /** No descriptions */
  KUrl::List allFilesInternal(const KUrl& startURL, const QString& mask);

//  friend class I_like_this_class;

private slots:
   void slotResult( KJob * job );
   void slotNewEntries(KIO::Job *job, const KIO::UDSEntryList& udsList);
public slots: // Public slots
  /** Timeout occurerd while waiting for some network function to return. */
  void slotTimeout();
};


#endif
