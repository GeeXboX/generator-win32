/*
 *  fs.h : GeeXboX Win32 generator operations on files.
 *  Copyright (C) 2003-2005  Benjamin Zores
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FS_H_
#define FS_H_

BOOL FileExists (char *file);
void MultipleFileDelete (char *token, char *src, BOOL recursive);
void MultipleFileCopy (char *token, char *src,
                       char *dest, char *exclude, BOOL recursive);

#endif
