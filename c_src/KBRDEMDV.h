/*
	KBRDEMDV.h

	Copyright (C) 2002 Philip Cummins, Paul Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifdef KBRDEMDV_H
#error "header already included"
#else
#define KBRDEMDV_H
#endif

EXPORTPROC Keyboard_Put(ui3b in);
EXPORTPROC Keyboard_Get(void);

EXPORTPROC KeyBoard_Update(void);
