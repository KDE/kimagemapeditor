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
#include <qptrlist.h>
#include <qregexp.h>

class QExtFileInfo:public QObject
{
 Q_OBJECT
public:
  QExtFileInfo() {};
  ~QExtFileInfo() {};

  /** create to ralative short name */
  static KURL toRelative(const KURL& urlToConvert,const KURL& baseURL);
  /** convert relative filename to absolute */
  static KURL toAbsolute(const KURL& urlToConvert,const KURL& baseURL);
  /** recurse function for all files in dir */
  static KURL::List allFiles( const KURL& path, const QString &mask);
  static KURL::List allFilesRelative( const KURL& path, const QString &mask);
  /** create dir if don't exists */
  static bool createDir(const KURL & path );
  static KURL cdUp(const KURL &dir);
  static QString shortName(const QString &fname );
  static KURL path(const KURL &);
  static KURL home();
  static bool exists(const KURL& url);
  static bool copy( const KURL& src, const KURL& dest, int permissions=-1,
                    bool overwrite=false, bool resume=false, QWidget* window = 0L );

  void enter_loop();

private:
  bool internalExists(const KURL& url);
  bool internalCopy(const KURL& src, const KURL& target, int permissions,
                    bool overwrite, bool resume, QWidget* window);

  bool bJobOK;
  static QString lastErrorMsg;
  KIO::UDSEntry m_entry;
  KURL::List dirListItems;
  QPtrList<QRegExp> lstFilters;

  /** No descriptions */
  KURL::List allFilesInternal(const KURL& startURL, const QString& mask);

//  friend class I_like_this_class;

private slots:
   void slotResult( KIO::Job * job );
   void slotNewEntries(KIO::Job *job, const KIO::UDSEntryList& udsList);
public slots: // Public slots
  /** Timeout occured while waiting for some network function to return. */
  void slotTimeout();
};


#endif
