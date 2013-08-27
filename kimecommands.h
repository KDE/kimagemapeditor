/***************************************************************************
                          kimecommands.h  -  description
                             -------------------
    begin                : Fri May 25 2001
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

#ifndef KIMECOMMANDS_H
#define KIMECOMMANDS_H


#include <qundostack.h>

#include <kdeversion.h>

class KImageMapEditor;
class AreaSelection;



class CutCommand : public QUndoCommand

{
	public:
  	CutCommand (KImageMapEditor * document, const AreaSelection & selection);
  	virtual ~CutCommand();

  	virtual void redo();
  	virtual void undo();

  protected:
  	AreaSelection *_cutAreaSelection;
		KImageMapEditor* _document;
		bool _cutted;
};

/**
 * Does the same like the cut command
 * only have a different name in the Undo-History
 **/
class DeleteCommand : public CutCommand
{
 public :
 	 DeleteCommand (KImageMapEditor * document, const AreaSelection & selection);
};

class PasteCommand : public QUndoCommand
{
	public:
  	PasteCommand (KImageMapEditor * document, const AreaSelection & selection);
  	~PasteCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:
  	AreaSelection *_pasteAreaSelection;
		KImageMapEditor* _document;
		bool _pasted;
		bool _wasUndoed;

};

class MoveCommand : public QUndoCommand
{
	public:
  	MoveCommand (KImageMapEditor *document, AreaSelection *a,const QPoint & oldPoint);
  	~MoveCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:
		QPoint _newPoint;
		QPoint _oldPoint;

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
//-		Area *_oldArea;
};

class ResizeCommand : public QUndoCommand
{
	public:
  	ResizeCommand (KImageMapEditor *document, AreaSelection *a, Area *oldArea);
  	~ResizeCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
		Area *_oldArea;
		Area *_newArea;
};

class AddPointCommand : public QUndoCommand
{
	public:
  	AddPointCommand (KImageMapEditor *document, AreaSelection *a, const QPoint & p);
  	~AddPointCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
    QPoint _point;
    int _coordpos;
};

class RemovePointCommand : public QUndoCommand
{
	public:
  	RemovePointCommand (KImageMapEditor *document, AreaSelection *a, Area *oldArea);
  	~RemovePointCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
		Area *_oldArea;
		Area *_newArea;
};


class CreateCommand : public QUndoCommand
{
	public:
  	CreateCommand (KImageMapEditor *document, Area *area);
  	~CreateCommand ();

  	virtual void redo();
  	virtual void undo();

  protected:

		KImageMapEditor* _document;
		Area *_area;
		bool _created;
		bool _wasUndoed;
};


#endif
