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


#include <k3command.h>

#include <kdeversion.h>

class KImageMapEditor;
class AreaSelection;



class CutCommand : public K3NamedCommand

{
	public:
  	CutCommand (KImageMapEditor * document, const AreaSelection & selection);
  	virtual ~CutCommand();

  	virtual void execute();
  	virtual void unexecute();

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

class PasteCommand : public K3NamedCommand
{
	public:
  	PasteCommand (KImageMapEditor * document, const AreaSelection & selection);
  	~PasteCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:
  	AreaSelection *_pasteAreaSelection;
		KImageMapEditor* _document;
		bool _pasted;
		bool _wasUndoed;

};

class MoveCommand : public K3NamedCommand
{
	public:
  	MoveCommand (KImageMapEditor *document, AreaSelection *a,const QPoint & oldPoint);
  	~MoveCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:
		QPoint _newPoint;
		QPoint _oldPoint;

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
//-		Area *_oldArea;
};

class ResizeCommand : public K3NamedCommand
{
	public:
  	ResizeCommand (KImageMapEditor *document, AreaSelection *a, Area *oldArea);
  	~ResizeCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
		Area *_oldArea;
		Area *_newArea;
};

class AddPointCommand : public K3NamedCommand
{
	public:
  	AddPointCommand (KImageMapEditor *document, AreaSelection *a, const QPoint & p);
  	~AddPointCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
    QPoint _point;
    int _coordpos;
};

class RemovePointCommand : public K3NamedCommand
{
	public:
  	RemovePointCommand (KImageMapEditor *document, AreaSelection *a, Area *oldArea);
  	~RemovePointCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:

		KImageMapEditor* _document;
  	AreaSelection *_areaSelection;
		Area *_oldArea;
		Area *_newArea;
};


class CreateCommand : public K3NamedCommand
{
	public:
  	CreateCommand (KImageMapEditor *document, Area *area);
  	~CreateCommand ();

  	virtual void execute();
  	virtual void unexecute();

  protected:

		KImageMapEditor* _document;
		Area *_area;
		bool _created;
		bool _wasUndoed;
};


#endif
