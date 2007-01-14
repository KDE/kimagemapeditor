/***************************************************************************
                          areacreator.cpp  -  description
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

#include "areacreator.h"



Area* AreaCreator::create( Area::ShapeType type)
{
	switch ( type ) {
		case Area::Rectangle : return new RectArea();
		case Area::Circle : return new CircleArea();
		case Area::Polygon : return new PolyArea();
		case Area::Default : return new DefaultArea();
		case Area::Selection : return new AreaSelection();
		default : return new Area();
	}
}


Area* AreaCreator::create( KImageMapEditor::ToolType type)
{
  switch ( type ) {
  case KImageMapEditor::Rectangle : return new RectArea();
  case KImageMapEditor::Circle : return new CircleArea();
  case KImageMapEditor::Polygon : return new PolyArea();
  case KImageMapEditor::Freehand : return new PolyArea();
  default : return new Area();
  }
}
