#! /usr/bin/env bash
$EXTRACTRC *.rc >> rc.cpp
LIST=`find . -name \*.h -o -name \*.cpp`
if test -n "$LIST"; then 
	$XGETTEXT $LIST -o $podir/kimagemapeditor.pot 
fi

