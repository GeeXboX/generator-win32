/*
 *  utils.c : GeeXboX Win32 generator shell utilities.
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "utils.h"

int
nget_shvar_value (FILE *fp, const char *var, char *dst, size_t dstlen)
{
  static char buf[512];
  char *line, *value, *end;

  rewind (fp);
  while ((line = fgets (buf, sizeof (buf), fp)))
    {
      while (isspace (*line))
        line++;

      if (*line == '#' || (value = strchr (line, '=')) == NULL)
        continue;

      *value++ = '\0';

      if (strcmp (line, var))
        continue;

      if (*value == '\"' || *value == '\'')
        {
	  char strtype = *value++;
          end = strchr(value, strtype);
        }
      else
          for (end = value; !isspace(*end); end++)
	    ;

      *end = '\0';

      strncpy (dst, value, --dstlen);
      return 0;
    }

  *dst = '\0';
  return 1;
}
