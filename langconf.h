/*
 *  langconf.h : GeeXboX Win32 generator languages management.
 *  Copyright (C) 2004-2005  Amir Shalem
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

#ifndef LANGCONF_H_
#define LANGCONF_H_

#define PATH_LANGCONF "language/lang.conf"

struct langinfo {
  char shortname[10];
  char name[50];
  char font[50];
  int bitmapmenu;
};

struct fontinfo {
  char font[50];
};

extern int langcount;		 /* count of languages in array */
extern struct langinfo *langs;	 /* array of languages */

extern struct langinfo *deflang; /* points to the default language */

extern int fontcount;
extern struct fontinfo *fonts;

void free_langconf (void);
void init_langconf (void);

int find_language (const char *name);

#endif
