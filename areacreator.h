/***************************************************************************
                          areacreator.h  -  description
                             -------------------
    begin                : Wed Apr 3 2002
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

#ifndef AREACREATOR_H
#define AREACREATOR_H


#include "kimagemapeditor.h"

/**
 * A small creator class which follows the
 * factory method pattern
 */
class AreaCreator
{
	public :
		static Area* create( Area::ShapeType );
		static Area* create( KImageMapEditor::ToolType);
};

#endif
