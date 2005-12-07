/***************************************************************************
                          kimecommon.h  -  description
                             -------------------
    begin                : Thu Apr 23 2002
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


#ifndef __KIMECOMMON_H__
#define __KIMECOMMON_H__

inline int myabs(int i) {
	if (i < 0)
		return -i;
	else
		return i;
}

inline double myabs(double i) {
	if (i < 0)
		return -i;
	else
		return i;
}

inline int myround(double d) {
	if ( (d-((int) d)) < 0.5 )
		return (int) d;
	else
		return ((int) d)+1;
}

inline int roundUp(double d)
{
  if ( (d-((int) d)) == 0)
     return (int) d;
  else
    return ((int) d)+1;
}


#endif
