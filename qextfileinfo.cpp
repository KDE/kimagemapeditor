/*
    From WebMaker - KDE HTML Editor
    Copyright (C) 1998, 1999 Alexei Dets <dets@services.ru>

    Rewritten for Quanta Plus: (C) 2002 Andras Mantia <amantia@freemail.hu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/


//qt includes
#include <qdir.h>
#include <qapplication.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtimer.h>

//kde includes
#include <kurl.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kdirlister.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdebug.h>

//app includes
#include "qextfileinfo.h"

QString QExtFileInfo::lastErrorMsg = "";

/** create a relative short url based in baseURL*/
KURL QExtFileInfo::toRelative(const KURL& urlToConvert,const KURL& baseURL)
{
  KURL resultURL = urlToConvert;
  if (urlToConvert.protocol() == baseURL.protocol())
  {
    QString path = urlToConvert.path();
    QString basePath = baseURL.path(1);
    if (path.startsWith("/"))
    {
      path.remove(0, 1);
      basePath.remove(0, 1);
      if ( basePath.right(1) != "/" ) basePath.append("/");

      int pos=0;
      int pos1=0;
      for (;;)
      {
        pos=path.find("/");
        pos1=basePath.find("/");
        if ( pos<0 || pos1<0 ) break;
        if ( path.left(pos+1 ) == basePath.left(pos1+1) )
        {
          path.remove(0, pos+1);
          basePath.remove(0, pos1+1);
        }
        else
          break;
      };

      if ( basePath == "/" ) basePath="";
      int level = basePath.contains("/");
      for (int i=0; i<level; i++)
      {
        path="../"+path;
      };
    }

    resultURL.setPath(QDir::cleanDirPath(path));
  }

  if (urlToConvert.path().endsWith("/")) resultURL.adjustPath(1);
  return resultURL;
}
/** convert relative filename to absolute */
KURL QExtFileInfo::toAbsolute(const KURL& urlToConvert,const KURL& baseURL)
{
  KURL resultURL = urlToConvert;
  if (urlToConvert.protocol() == baseURL.protocol() && !urlToConvert.path().startsWith("/"))
  {
    int pos;
    QString cutname = urlToConvert.path();
    QString cutdir = baseURL.path(1);
    while ( (pos = cutname.find("../")) >=0 )
    {
       cutname.remove( 0, pos+3 );
       cutdir.remove( cutdir.length()-1, 1 );
       cutdir.remove( cutdir.findRev('/')+1 , 1000);
    }
    resultURL.setPath(QDir::cleanDirPath(cutdir+cutname));
  }

  if (urlToConvert.path().endsWith("/")) resultURL.adjustPath(1);
  return resultURL;
}

/** All files in a dir.
  The return will also contain the name of the subdirectories.
  This is needed for empty directory adding/handling. (Andras)
  Currently works only for local directories
*/
KURL::List QExtFileInfo::allFiles( const KURL& path, const QString& mask)
{
  QExtFileInfo internalFileInfo;
  return internalFileInfo.allFilesInternal(path, mask);
}

KURL::List QExtFileInfo::allFilesRelative( const KURL& path, const QString& mask)
{
  QExtFileInfo internalFileInfo;
  KURL::List r = internalFileInfo.allFilesInternal( path, mask);

  KURL::List::Iterator it;
  for ( it = r.begin(); it != r.end(); ++it )
  {
    *it = QExtFileInfo::toRelative( *it, path );
  }

  return r;
}

bool QExtFileInfo::createDir( const KURL& path )
{
  int i=0;
  bool result;
  KURL dir1, dir2 = KURL();
  while ( !exists(path) && dir2.path() != path.path() )
  {
    dir1 = path;
    dir2 = path;

    dir1=cdUp(dir1);
    while ( !exists(dir1) && dir1.path() != "/" )
    {
      dir1=cdUp(dir1);
      dir2=cdUp(dir2);
    //  debug(d1);
    }
  //  dir2.setPath(dir2.path(-1));
    result = KIO::NetAccess::mkdir(dir2, 0L, -1);
    i++;
  }
 result = exists(path);
 return result;
}

KURL QExtFileInfo::cdUp(const KURL &url)
{
  KURL u = url;
  QString dir = u.path(-1);
  while ( !dir.isEmpty() && dir.right(1) != "/" )
  {
    dir.remove( dir.length()-1,1);
  }
  u.setPath(dir);
  return u;
}

QString QExtFileInfo::shortName(const QString &fname)
{
  return fname.section("/",-1);
}

KURL QExtFileInfo::path( const KURL &url )
{
  return KURL( url.directory(false,false) );
}

KURL QExtFileInfo::home()
{
  KURL url;
  url.setPath(QDir::currentDirPath()+"/");
  return url;
}


bool QExtFileInfo::exists(const KURL& a_url)
{
// Andras: Don't use it now, as it brings up an extra dialog and need manual
// intervention when usign fish
// return KIO::NetAccess::exists(a_url, false);

// No dialog when stating.
 if ( a_url.isLocalFile() )
 {
    return QFile::exists( a_url.path() );
 } else
 {
  QExtFileInfo internalFileInfo;
  return internalFileInfo.internalExists(a_url);
 }
}

/* Synchronouse copy, like NetAccess::file_copy in KDE 3.2 */
bool QExtFileInfo::copy( const KURL& src, const KURL& target, int permissions,
 bool overwrite, bool resume, QWidget* window )
{
  QExtFileInfo internalFileInfo;
  return internalFileInfo.internalCopy( src, target, permissions, overwrite, resume, window );
}

/** No descriptions */
KURL::List QExtFileInfo::allFilesInternal(const KURL& startURL, const QString& mask)
{
  dirListItems.clear();
  if (internalExists(startURL))
  {
    lstFilters.setAutoDelete(true);
    lstFilters.clear();
    // Split on white space
    QStringList list = QStringList::split( ' ', mask );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
       lstFilters.append( new QRegExp(*it, false, true ) );

    bJobOK = true;
    KIO::ListJob *job = KIO::listRecursive(startURL, false, true);
    connect(job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList&)),
            this, SLOT(slotNewEntries(KIO::Job *, const KIO::UDSEntryList&)));
    connect( job, SIGNAL( result (KIO::Job *) ),
             this, SLOT( slotResult (KIO::Job *) ) );

 //   kdDebug(24000) << "Now listing: " << startURL.url() << endl;
    enter_loop();
    lstFilters.clear();
    if (!bJobOK)
    {
 //     kdDebug(24000) << "Error while listing "<< startURL.url() << endl;
      dirListItems.clear();
    }
  }
  return dirListItems;
}


//Some hackery from KIO::NetAccess as they do not do exactly what we want
/* return true if the url exists*/
bool QExtFileInfo::internalExists(const KURL& url)
{
  bJobOK = true;
 // kdDebug(24000)<<"QExtFileInfo::internalExists"<<endl;
  KIO::StatJob * job = KIO::stat( url, false);
  job->setDetails(0);
  job->setSide(false); //check the url for writing
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  //To avoid lock-ups, start a timer.
  QTimer::singleShot(10*1000, this,SLOT(slotTimeout()));
//  kdDebug(24000)<<"QExtFileInfo::internalExists:before enter_loop"<<endl;
  enter_loop();
//  kdDebug(24000)<<"QExtFileInfo::internalExists:after enter_loop"<<endl;
  return bJobOK;
}

bool QExtFileInfo::internalCopy(const KURL& src, const KURL& target, int permissions,
                                bool overwrite, bool resume, QWidget* window)
{
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = KIO::file_copy( src, target, permissions, overwrite, resume );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}


void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void QExtFileInfo::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
//  kdDebug(24000)<<"QExtFileInfo::enter_loop:before qApp->enter_loop()"<<endl;
  qApp->enter_loop();
//  kdDebug(24000)<<"QExtFileInfo::enter_loop:after qApp->enter_loop()"<<endl;
  qt_leave_modal(&dummy);
}

void QExtFileInfo::slotResult( KIO::Job * job )
{
  bJobOK = !job->error();
  if ( !bJobOK )
  {
    if ( !lastErrorMsg )
     lastErrorMsg = job->errorString();
  }
  if ( job->isA("KIO::StatJob") )
    m_entry = static_cast<KIO::StatJob *>(job)->statResult();
  qApp->exit_loop();
}

void QExtFileInfo::slotNewEntries(KIO::Job *job, const KIO::UDSEntryList& udsList)
{
  KURL url = static_cast<KIO::ListJob *>(job)->url();
  url.adjustPath(-1);
  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");

  KIO::UDSEntryListConstIterator it = udsList.begin();
  KIO::UDSEntryListConstIterator end = udsList.end();
  KURL itemURL;
  for ( ; it != end; ++it )
  {
    QString name;

    // find out about the name
    KIO::UDSEntry::ConstIterator entit = (*it).begin();
    for( ; entit != (*it).end(); ++entit )
      if ( (*entit).m_uds == KIO::UDS_NAME )
      {
        name = (*entit).m_str;
        break;
      }

    if ( ! name.isEmpty() && name != dot && name != dotdot)
    {
      KFileItem* item = new KFileItem( *it, url, false, true );
      itemURL = item->url();
      if (item->isDir()) itemURL.adjustPath(1);
      for ( QPtrListIterator<QRegExp> filterIt( lstFilters ); filterIt.current(); ++filterIt )
      if ( filterIt.current()->exactMatch( item->text() ) )
           dirListItems.append(itemURL);
      delete item;
    }
  }
}

/** Timeout occured while waiting for some network function to return. */
void QExtFileInfo::slotTimeout()
{
  bJobOK = false;
  qApp->exit_loop();
}
#include "qextfileinfo.moc"
