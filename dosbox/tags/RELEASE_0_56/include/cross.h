/*
 *  Copyright (C) 2002  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _CROSS_H
#define _CROSS_H

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined (_MSC_VER)						/* MS Visual C++ */
#include <direct.h>
#include <io.h>
#else										/* LINUX */
#include <dirent.h>
#include <unistd.h>
#endif

#define CROSS_LEN 512						/* Maximum filename size */


#if defined (WIN32)							/* Win 32 */
#define CROSS_FILENAME(blah)
#define CROSS_FILESPLIT '\\'
#define F_OK 0
#else
#define	CROSS_FILENAME(blah) strreplace(blah,'\\','/')
#define CROSS_FILESPLIT '/'
#endif

#define CROSS_NONE	0
#define CROSS_FILE	1
#define CROSS_DIR	2
#if defined (WIN32)
#define ftruncate(blah,blah2) chsize(blah,blah2)
#endif

extern const char * dosbox_datadir;

#endif

