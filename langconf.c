/*
 *  langconf.c : GeeXboX Win32 generator languages management.
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

#include "langconf.h"
#include "options.h"
#include "fs.h"

static FILE *langfile = NULL;

struct langinfo *deflang;
struct langinfo *langs;
int langcount;

struct fontinfo *fonts;
int fontcount;

static int
get_sh_value (const char *var, char *dst)
{
  static char buf[256];
  char *line, *value;

  rewind (langfile);
  while ((line = fgets (buf, sizeof (buf), langfile)))
    {
      while (isspace (*line))
        line++;
      while (isspace (line[strlen(line)-1]))
        line[strlen(line)-1] = '\0';

      if (*line == '#' || (value = strchr (line, '=')) == NULL)
        continue;

      *value++ = '\0';

      if (strcmp (line, var))
        continue;

      if (*value == '"' && value[strlen(value)-1] == '"')
        {
          value++;
          value[strlen(value)-1] = '\0';
        }

      strcpy(dst, value);

      return 0;
    }

  *dst = '\0';

  return 1;
}

static void
add_language (const char *name, int loc)
{
  char buf[50];

  strcpy (langs[loc].shortname, name);

  sprintf (buf, "%s_name", name);
  get_sh_value (buf, langs[loc].name);

  sprintf (buf, "%s_font", name);
  get_sh_value (buf, langs[loc].font);

  sprintf (buf, "%s_bitmapmenu", name);
  if (!get_sh_value (buf, buf))
    langs[loc].bitmapmenu = !strcasecmp (buf, "yes")
      || !strcasecmp (buf, "true")
      || !strcasecmp (buf, "1");
}

static void
add_font (const char *name, int loc)
{
  strcpy (fonts[loc].font, name);
}

static int
cmp_languages (const void *a, const void *b)
{
  return strcmp (((const struct langinfo*) a)->name,
                 ((const struct langinfo*) b)->name);
}

static int
cmp_fonts (const void *a, const void *b)
{
  return strcmp (((const struct fontinfo*) a)->font,
                 ((const struct fontinfo*) b)->font);
}

static int
find_language (const char *name)
{
  int i;

  for (i = 0; i < langcount; i++)
    {
      if (!strcmp (name, langs[i].name))
        return i;
    }

  return -1;
}

void
init_langconf (void)
{
  char buf[256], *tmp;
  int i;

  langfile = fopen (PATH_LANGCONF, "r");
  if (langfile == NULL)
    {
      perror ("fopen");
      exit (1);
    }

  if (get_sh_value ("LANGUAGES", buf) || buf[0] == '\0')
    {
      printf ("ERROR: no languages found.\n");
      exit (1);
    }

  langcount = 1;
  while ((tmp = strrchr (buf, ' ')))
    {
      *tmp = '\0';
      langcount++;
    }

  langs = (struct langinfo *) malloc (sizeof (struct langinfo) * langcount);
  if (langs == NULL)
    exit (1);

  for (tmp = buf, i = 0; i < langcount && *tmp; tmp += strlen (tmp)+1, i++)
    add_language (tmp, i);

  qsort (langs, langcount, sizeof (struct langinfo), cmp_languages);

  get_sh_value ("DEFAULT_LANGUAGE", buf);
  for (i = 0; i < langcount; i++)
    if (!strcmp (langs[i].shortname, buf))
      deflang = &langs[i];

  if (get_sh_value ("FONTS", buf) || buf[0] == '\0')
    {
      printf ("ERROR: no fonts found.\n");
      exit (1);
    }

  fontcount = 1;
  while ((tmp = strrchr (buf, ' ')))
    {
      *tmp = '\0';
      fontcount++;
    }

  fonts = (struct fontinfo *) malloc (sizeof (struct fontinfo) * fontcount);
  if (fonts == NULL)
    {
      printf ("ERROR: Failed to initilize fonts.\n");
      exit (1);
    }

  for (tmp = buf, i = 0; i < fontcount && *tmp; tmp += strlen (tmp)+1, i++)
    add_font (tmp, i);

  qsort (fonts, fontcount, sizeof (struct fontinfo), cmp_fonts);

  fclose (langfile);
  langfile = NULL;
}

void
free_langconf (void)
{
  if (langs)
    free (langs);
  if (fonts)
    free (fonts);

  langcount = 0;
  fontcount = 0;
  deflang = NULL;
  langs = NULL;
  fonts = NULL;
}

int
write_lang_to_disk (HWND hwnd, geexbox_options_t *opts,
                    char *subfont_dir, char *menufont_dir)
{
  char *menu_font, *sub_font, *filelist[2], buf[128];
  int l, f, i;
  FILE *fp;

  l = find_language (opts->lang);

  menu_font = langs[l].bitmapmenu ? langs[l].font : "";
  sub_font = strcmp (opts->subfont, SUBFONT_AS_LANGUAGE)
    ? opts->subfont : opts->lang;

  if ((f = find_language (sub_font)) >= 0)
    sub_font = langs[f].font;

  sprintf (buf, "font/%s/font.desc", sub_font);
  filelist[0] = buf;
  sprintf (subfont_dir, "font/%s/font.desc", menu_font);
  filelist[1] = *menu_font ? subfont_dir : NULL;

  for (i = 0; i < 2; i++)
    if (filelist[i] && FileExists (filelist[i]) == FALSE)
      {
        sprintf (menufont_dir, "%s font is missing.\nPlease visit the README" \
                 " - EXTRA SUBTITLE FONTS section", sub_font);
        printf ("*** %s ***\n", menufont_dir);
        MessageBox (hwnd, menufont_dir, "ERROR", MB_OK | MB_ICONERROR);
        return -1;
      }

  sprintf (buf, "language/help_%s.txt", langs[l].shortname);
  filelist[0] = buf;
  sprintf (subfont_dir, "language/menu_%s.conf", langs[l].shortname);
  filelist[1] = subfont_dir;

  for (i = 0; i < 2; i++)
    if (filelist[i] && FileExists(filelist[i]) == FALSE)
      {
        sprintf (menufont_dir, "%s language file is missing", filelist[i]);
        printf ("*** %s ***\n", menufont_dir);
        MessageBox (hwnd, menufont_dir, "ERROR", MB_OK | MB_ICONERROR);
        return -1;
      }

  fp = fopen ("iso/GEEXBOX/etc/lang", "wb");
  fprintf (fp, "%s", langs[l].shortname);
  fclose (fp);

  CreateDirectory ("ziso", NULL);
  sprintf (buf, "language/help_%s.txt", langs[l].shortname);
  sprintf (subfont_dir,
           "iso/GEEXBOX/usr/share/mplayer/help_%s.txt", langs[l].shortname);
  CopyFile (buf, subfont_dir, FALSE);

  sprintf (buf, "language/menu_%s.conf", langs[l].shortname);
  sprintf (subfont_dir,
           "iso/GEEXBOX/etc/mplayer/menu_%s.conf", langs[l].shortname);
  CopyFile (buf, subfont_dir, FALSE);

  sprintf (buf, "language/lang.conf");
  sprintf (subfont_dir, "iso/GEEXBOX/etc/lang.conf");
  CopyFile (buf, subfont_dir, FALSE);

  fp = fopen ("iso/GEEXBOX/etc/subfont", "wb");
  fprintf (fp, "%s", sub_font);
  fclose (fp);

  sprintf (buf, "font/%s/", sub_font);
  sprintf (subfont_dir, "iso/GEEXBOX/usr/share/mplayer/font/%s/", sub_font);
  CreateDirectory (subfont_dir, NULL);
  MultipleFileCopy ("*", buf, subfont_dir, "", FALSE);

  if (strcmp (menu_font, "") && strcmp (menu_font, sub_font))
    {
      sprintf (buf, "font/%s/", menu_font);
      sprintf (menufont_dir,
               "iso/GEEXBOX/usr/share/mplayer/font/%s/", menu_font);
      CreateDirectory (menufont_dir, NULL);
      MultipleFileCopy ("*", buf, menufont_dir, "", FALSE);
    }
  else
    menufont_dir[0] = '\0';

  return l;
}
