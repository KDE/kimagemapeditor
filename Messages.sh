#! /usr/bin/env bash
LIST=`find . -name \*.h -o -name \*.cpp`
if test -n "$LIST"; then 
	$XGETTEXT $LIST -o $podir/kimagemapeditor.pot 
fi

