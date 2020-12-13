/***************************************************************************
                          kimagemapeditorinterface.h
                             -------------------
    begin                : Wed Apr 4 2001
    copyright            : (C) 2001 by Jan Schäfer
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

#ifndef KIMAGEMAPEDITORINTERFACE_H
#define KIMAGEMAPEDITORINTERFACE_H

class KConfigGroup;
class QUrl;

class KImageMapEditorInterface
{
public:
    virtual ~KImageMapEditorInterface() {}

    /**
     * Opens the given file.
     * If it's an HTML file openURL is called
     * If it's an Image, the image is added to the image list
     */
    virtual void openFile(const QUrl &) = 0;
    virtual bool openURL(const QUrl & url) = 0;

    virtual void openLastURL(const KConfigGroup &) = 0;

    virtual void readProperties(const KConfigGroup &) = 0;
    virtual void saveProperties(KConfigGroup &) = 0;
};

Q_DECLARE_INTERFACE(KImageMapEditorInterface, "org.kde.KImageMapEditorInterface")

#endif
