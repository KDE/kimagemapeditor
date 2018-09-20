/***************************************************************************
                          kimecommands.cpp  -  description
                             -------------------
    begin                : Fri May 25 2001
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
#include "kimecommands.h"

#include <QString>
#include "kimagemapeditor_debug.h"
#include <KLocalizedString>

#include "drawzone.h"

CutCommand::CutCommand(KImageMapEditor * document, const AreaSelection & a)
  : QUndoCommand(i18n( "Cut %1", a.typeString() ))
{
	_document=document;
	_cutAreaSelection=new AreaSelection();
	_cutAreaSelection->setAreaList( a.getAreaList() );
	_cutted=true;
}


CutCommand::~CutCommand()
{
	if (_cutted)
	{
		AreaListIterator it(_cutAreaSelection->getAreaList());
    while (it.hasNext()) {
      delete it.next();
  	}
	}

	delete _cutAreaSelection;
}

void CutCommand::redo()
{
	// The Area won't be really delete
	// it only gets removed from the AreaList
	_document->deleteArea(_cutAreaSelection );
  _document->updateActionAccess();
	_cutted=true;
}

void CutCommand::undo()
{
	if (_document) {
		_document->addArea( _cutAreaSelection );
		_document->select( _cutAreaSelection );
		_document->slotAreaChanged( _cutAreaSelection );
		_cutted=false;
	}
}

DeleteCommand::DeleteCommand(KImageMapEditor * document, const AreaSelection & a)
	: CutCommand(document,a)
{
	setText(i18n( "Delete %1", a.typeString() ));
}

PasteCommand::PasteCommand(KImageMapEditor *document, const AreaSelection & a)
	: QUndoCommand(i18n( "Paste %1", a.typeString() ))
{
	_document=document;
	_pasteAreaSelection=new AreaSelection();
	_pasteAreaSelection->setAreaList( a.getAreaList() );
	_pasted=true;
	_wasUndoed=false;
}

PasteCommand::~PasteCommand ()
{
	if ( ! _pasted ) {
		AreaListIterator it(_pasteAreaSelection->getAreaList());
    while (it.hasNext()) {
  		delete it.next();
  	}
	}

	delete _pasteAreaSelection;
}

void PasteCommand::redo()
{
	_document->deselectAll();
	_document->addArea( _pasteAreaSelection );
	_document->select( _pasteAreaSelection );
	_document->slotAreaChanged( _pasteAreaSelection );
	_pasted=true;
}

void PasteCommand::undo()
{
	_document->deleteArea(_pasteAreaSelection );
	_pasted=false;
	_wasUndoed=true;
}


MoveCommand::MoveCommand (KImageMapEditor *document, AreaSelection * a, const QPoint & oldPoint)
	: QUndoCommand(i18n( "Move %1", a->typeString() ))
{
	_document=document;
	_areaSelection=new AreaSelection();
	_areaSelection->setAreaList( a->getAreaList() );
	_oldPoint.setX(oldPoint.x());
	_oldPoint.setY(oldPoint.y());

	_newPoint.setX(a->rect().left());
	_newPoint.setY(a->rect().top());
}

MoveCommand::~MoveCommand () {
	delete _areaSelection;
}

void MoveCommand::redo()
{
	// only for repainting reasons
	Area* tempArea = _areaSelection->clone();

	_areaSelection->moveTo( _newPoint.x(), _newPoint.y() );

  if (!_areaSelection->allAreasWithin(_document->getDrawZone()->getImageRect()))
  	_areaSelection->moveTo( _oldPoint.x(), _oldPoint.y() );

  _document->selected()->invalidate();


	_document->slotAreaChanged( tempArea );
	_document->slotAreaChanged( _areaSelection );

	delete tempArea;

}

void MoveCommand::undo()
{
  // only to erase the old Area
  Area* tempArea = _areaSelection->clone();

  _areaSelection->setMoving(true);
	_areaSelection->moveTo( _oldPoint.x(), _oldPoint.y() );
  _areaSelection->setMoving(false);

  _document->selected()->invalidate();

	_document->slotAreaChanged( tempArea );
	_document->slotAreaChanged( _areaSelection );

	delete tempArea;

}


ResizeCommand::ResizeCommand (KImageMapEditor *document, AreaSelection *a, Area *oldArea)
	:QUndoCommand(i18n( "Resize %1", a->typeString() ))
{
	_areaSelection=new AreaSelection();
	_areaSelection->setAreaList( a->getAreaList() );

	_newArea = a->clone();
	_oldArea = oldArea->clone();
	_document=document;
}

ResizeCommand::~ResizeCommand ()
{
	delete _newArea;
	delete _oldArea;
	delete _areaSelection;
}

void ResizeCommand::redo()
{
  _areaSelection->setArea ( *_newArea);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
	_document->slotAreaChanged( _oldArea );


}

void ResizeCommand::undo()
{
  _areaSelection->setArea ( *_oldArea);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
	_document->slotAreaChanged( _newArea );

}



AddPointCommand::AddPointCommand (KImageMapEditor *document, AreaSelection *a, const QPoint & p)
	:QUndoCommand(i18n( "Add point to %1", a->typeString() ))
{
  if (a->type()!=Area::Polygon)
  {
     qCDebug(KIMAGEMAPEDITOR_LOG) << "trying to add a point to a " << a->typeString();
     return;
  }

 	_areaSelection=new AreaSelection();
	_areaSelection->setAreaList( a->getAreaList() );

	_point = p;
	_document=document;
}

AddPointCommand::~AddPointCommand ()
{
	delete _areaSelection;
}

void AddPointCommand::redo()
{
  _coordpos = _areaSelection->addCoord(_point);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
}

void AddPointCommand::undo()
{
//  QRect *selectionPoint = _areaSelection->onSelectionPoint(_point);
  Area* repaintArea = _areaSelection->clone();

  _areaSelection->removeCoord(_coordpos);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
	_document->slotAreaChanged( repaintArea );

  delete repaintArea;
}

RemovePointCommand::RemovePointCommand(KImageMapEditor *document, 
       AreaSelection *a, Area *oldArea)
  : QUndoCommand(i18n( "Remove point from %1", a->typeString() ))
{
  if (a->type()!=Area::Polygon)
  {
     qCDebug(KIMAGEMAPEDITOR_LOG) << "trying to remove a point to a " << a->typeString();
     return;
  }

	_areaSelection=new AreaSelection();
	_areaSelection->setAreaList( a->getAreaList() );

	_newArea = a->clone();
	_oldArea = oldArea->clone();
	_document=document;
}

RemovePointCommand::~RemovePointCommand ()
{
	delete _newArea;
	delete _oldArea;
	delete _areaSelection;
}

void RemovePointCommand::redo()
{
  _areaSelection->setArea ( *_newArea);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
	_document->slotAreaChanged( _oldArea );


}

void RemovePointCommand::undo()
{
  _areaSelection->setArea ( *_oldArea);
  _areaSelection->setMoving(false);

	_document->slotAreaChanged( _areaSelection );
	_document->slotAreaChanged( _newArea );

}



CreateCommand::CreateCommand (KImageMapEditor *document, Area *area)
	: QUndoCommand(i18n( "Create %1", area->typeString() ))
{
	_document=document;
	_area=area;
	_created=true;
	_wasUndoed=false;

}

CreateCommand::~CreateCommand ()
{
	if ( ! _created)
 		delete _area;
}

void CreateCommand::redo()
{
	if (_document) {

		if ( _wasUndoed ) {
			_document->addArea( _area );
			_document->deselectAll();
			_document->select( _area );
			_document->slotAreaChanged( _area );
		}
		else
			_document->addAreaAndEdit( _area );

		_created=true;
	}

}

void CreateCommand::undo()
{
	if (_document) {
		_document->deleteArea( _area );
		_created=false;
		_wasUndoed=true;
	}

}
