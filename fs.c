/*
 *  fs.c : GeeXboX Win32 generator operations on files.
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

#include <windows.h>
#include <stdio.h>

BOOL
FileExists (char *file)
{
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;

  hSearch = FindFirstFile (file, &FileData);
  if (hSearch != INVALID_HANDLE_VALUE)
    {
      FindClose (hSearch);
      return TRUE;
    }
  else
    return FALSE;
}

void
MultipleFileDelete (char *token, char *src, BOOL recursive)
{
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char file[100], search[100];

  lstrcpy (search, src);
  lstrcat (search, token);
  hSearch = FindFirstFile (search, &FileData);
  if (hSearch == INVALID_HANDLE_VALUE)
    return;

  while (!finished)
    {
      lstrcpy (file, src);
      lstrcat (file, FileData.cFileName);

      if (strcmp (FileData.cFileName, ".")
          && strcmp (FileData.cFileName, ".."))
        {
          if (GetFileAttributes (file) == FILE_ATTRIBUTE_DIRECTORY)
            {
              /* File descriptor is a directory */
              if (recursive)
                {
                  lstrcat (file, "/");
                  MultipleFileDelete (token, file, TRUE);
                  printf ("Removing directory : %s\n", file);
                  RemoveDirectory (file);
                }
            }
          else
            {
              /* File descriptor is a file */
              printf ("Removing file %s\n", file);
              DeleteFile (file);
            }
        }

      if (!FindNextFile (hSearch, &FileData))
        if (GetLastError () == ERROR_NO_MORE_FILES)
          finished = TRUE;
    }
  FindClose (hSearch);
}

void
MultipleFileCopy (char *token, char *src,
                  char *dest, char *exclude, BOOL recursive)
{
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char newFile[100], oldFile[100], search[100];

  lstrcpy (search, src);
  lstrcat (search, token);
  hSearch = FindFirstFile (search, &FileData);
  if (hSearch == INVALID_HANDLE_VALUE)
    return;

  while (!finished)
    {
      lstrcpy (oldFile, src);
      lstrcat (oldFile, FileData.cFileName);
      lstrcpy (newFile, dest);
      lstrcat (newFile, FileData.cFileName);

      if (strcmp (FileData.cFileName, ".")
          && strcmp (FileData.cFileName, "..")
          && strcmp (FileData.cFileName, exclude))
        {
          if (GetFileAttributes (oldFile) == FILE_ATTRIBUTE_DIRECTORY)
            {
              /* File descriptor is a directory */
              if (recursive)
                {
                  CreateDirectory (newFile, NULL);
                  printf ("Creating new directory : %s\n", newFile);
                  lstrcat (oldFile, "/");
                  lstrcat (newFile, "/");
                  MultipleFileCopy (token, oldFile, newFile, exclude, TRUE);
                }
            }
          else
            {
              /* File descriptor is a file */
              printf ("Copying file %s to %s\n", oldFile, newFile);
              CopyFile (oldFile, newFile, FALSE);
            }
        }

      if (!FindNextFile (hSearch, &FileData))
        if (GetLastError () == ERROR_NO_MORE_FILES)
          finished = TRUE;
    }
  FindClose (hSearch);
}
