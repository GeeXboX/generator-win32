/*
 *  generator.c : GeeXboX Win32 generator main file.
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

#include "resource.h"
#include "langconf.h"
#include "options.h"

#define DOCS_PATH "DOCS/README_"

static geexbox_options_t *opts;

static char *
GetVersionNumber (void)
{
  FILE *fd;
  char *version;

  version = (char *) malloc (100 * sizeof (char));
  fd = fopen ("VERSION", "r");
  fgets (version, 100, fd);
  if (version[strlen(version) - 1] == '\n')
    version[strlen(version) - 1] = '\0';

  return version;
}

static void
ListSubFonts (HWND hwnd)
{
  int i;

  for (i = 0; i < fontcount; i++)
    SendDlgItemMessage (hwnd, SUBFONT_LIST,
                        CB_ADDSTRING, 0, (LPARAM) fonts[i].font);

  SendDlgItemMessage (hwnd, SUBFONT_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SUBFONT_AS_LANGUAGE);
}

static void
ListLanguages (HWND hwnd)
{
  char *name;
  int i;

  for (i = 0; i < langcount; i++)
    {
      name = langs[i].name;
      SendDlgItemMessage (hwnd, LANG_LIST, CB_ADDSTRING, 0, (LPARAM) name);
      SendDlgItemMessage (hwnd, SUBFONT_LIST, CB_ADDSTRING, 0, (LPARAM) name);
    }
}

static void
ListRemotes (HWND hwnd)
{
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char remote[50];

  hSearch = FindFirstFile ("lirc/lircrc_*", &FileData);
  if (hSearch == INVALID_HANDLE_VALUE)
    return;

  while (!finished)
    {
      strcpy (remote, &FileData.cFileName[7]);
      SendDlgItemMessage (hwnd, REMOTE_LIST, CB_ADDSTRING, 0, (LPARAM) remote);
      if (!FindNextFile (hSearch, &FileData))
        if (GetLastError () == ERROR_NO_MORE_FILES)
          finished = TRUE;
    }
  FindClose (hSearch);
}

static void
ListReceivers (HWND hwnd)
{
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char receiver[50];

  hSearch = FindFirstFile ("lirc/lircd_*", &FileData);
  if (hSearch == INVALID_HANDLE_VALUE)
    return;

  while (!finished)
    {
      strcpy (receiver, &FileData.cFileName[6]);
      if (strcmp (&FileData.cFileName[strlen(FileData.cFileName)-5], ".conf"))
        SendDlgItemMessage (hwnd, RECEIVER_LIST,
                            CB_ADDSTRING, 0, (LPARAM) receiver);
      if (!FindNextFile (hSearch, &FileData))
        if (GetLastError () == ERROR_NO_MORE_FILES)
          finished = TRUE;
    }
  FindClose (hSearch);
}

static BOOL
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

static void
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

static void
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

static void
Execute (char *cmdline)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory (&si, sizeof (si));
  si.cb = sizeof (si);
  CreateProcess (NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
  WaitForSingleObject (pi.hProcess, INFINITE);
  CloseHandle (pi.hProcess);
  CloseHandle (pi.hThread);
}

static void
ExecuteToFile (char *cmdline, char *file)
{
  HANDLE saveStdOutPut, output;

  saveStdOutPut = GetStdHandle (STD_OUTPUT_HANDLE);
  output = CreateFile (file, GENERIC_WRITE, FILE_SHARE_WRITE,
                       NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  SetStdHandle (STD_OUTPUT_HANDLE, output);

  if ((saveStdOutPut == INVALID_HANDLE_VALUE)
      || (output == INVALID_HANDLE_VALUE))
    return; /* Can't get output */

  Execute (cmdline);

  /* Restore Standard Output */
  SetStdHandle (STD_OUTPUT_HANDLE, saveStdOutPut);
  CloseHandle (output);
}

static void
GenerateISO (HWND hwnd)
{
  char buf[128], buf2[128], buf3[128], version[128];
  char *menu_font, *sub_font, *filelist[2];
  int i, l, f;
  FILE *fp;

  printf ("*** Generating ISO image ***\n");
  display_options_to_console (hwnd, opts);

  l = find_language (opts->lang);

  menu_font = langs[l].bitmapmenu ? langs[l].font : "";
  sub_font = strcmp (opts->subfont, SUBFONT_AS_LANGUAGE)
    ? opts->subfont : opts->lang;

  if ((f = find_language (sub_font)) >= 0)
    sub_font = langs[f].font;

  sprintf (buf, "font/%s/font.desc", sub_font);
  filelist[0] = buf;
  sprintf (buf2, "font/%s/font.desc", menu_font);
  filelist[1] = *menu_font ? buf2 : NULL;

  for (i = 0; i < 2; i++)
    if (filelist[i] && FileExists (filelist[i]) == FALSE)
      {
        sprintf (buf3, "%s font is missing.\nPlease visit the README - " \
                 "EXTRA SUBTITLE FONTS section", sub_font);
        printf ("*** %s ***\n", buf3);
        MessageBox (hwnd, buf3, "ERROR", MB_OK | MB_ICONERROR);
        return;
      }

  sprintf (buf, "language/help_%s.txt", langs[l].shortname);
  filelist[0] = buf;
  sprintf (buf2, "language/menu_%s.conf", langs[l].shortname);
  filelist[1] = buf2;

  for (i = 0; i < 2; i++)
    if (filelist[i] && FileExists(filelist[i]) == FALSE)
      {
        sprintf (buf3, "%s language file is missing", filelist[i]);
        printf ("*** %s ***\n", buf3);
        MessageBox (hwnd, buf3, "ERROR", MB_OK | MB_ICONERROR);
        return;
      }

  fp = fopen ("iso/GEEXBOX/etc/lang", "wb");
  fprintf (fp, "%s", langs[l].shortname);
  fclose (fp);

  CreateDirectory ("ziso", NULL);
  sprintf (buf, "language/help_%s.txt", langs[l].shortname);
  sprintf (buf2,
           "iso/GEEXBOX/usr/share/mplayer/help_%s.txt", langs[l].shortname);
  CopyFile (buf, buf2, FALSE);

  sprintf (buf, "language/menu_%s.conf", langs[l].shortname);
  sprintf (buf2, "iso/GEEXBOX/etc/mplayer/menu_%s.conf", langs[l].shortname);
  CopyFile (buf, buf2, FALSE);

  sprintf (buf, "language/lang.conf");
  sprintf (buf2, "iso/GEEXBOX/etc/lang.conf");
  CopyFile (buf, buf2, FALSE);

  fp = fopen ("iso/GEEXBOX/etc/subfont", "wb");
  fprintf (fp, "%s", sub_font);
  fclose (fp);

  sprintf (buf, "font/%s/", sub_font);
  sprintf (buf2, "iso/GEEXBOX/usr/share/mplayer/font/%s/", sub_font);
  CreateDirectory (buf2, NULL);
  MultipleFileCopy ("*", buf, buf2, "", FALSE);

  if (strcmp (menu_font, "") && strcmp (menu_font, sub_font))
    {
      sprintf (buf, "font/%s/", menu_font);
      sprintf (buf3, "iso/GEEXBOX/usr/share/mplayer/font/%s/", menu_font);
      CreateDirectory (buf3, NULL);
      MultipleFileCopy ("*", buf, buf3, "", FALSE);
    }
  else
    buf3[0] = '\0';

  fp = fopen ("iso/GEEXBOX/etc/audio", "wb");
  fprintf (fp, "SPDIF=%s\n", !strcmp (opts->audio, "spdif") ? "yes" : "no");
  fclose (fp);
  if (!strcmp (opts->vidix, "no"))
    {
      fp = fopen ("iso/GEEXBOX/etc/mplayer/no_nvidia_vidix", "ab");
      fclose (fp);
    }
  else
    DeleteFile ("iso/GEEXBOX/etc/mplayer/no_nvidia_vidix");

  fp = fopen ("iso/GEEXBOX/etc/view_img_timeout", "wb");
  fprintf (fp, "%s", opts->image_tempo);
  fclose (fp);

  fp = fopen ("iso/GEEXBOX/etc/network", "wb");
  fprintf (fp, "PHY_TYPE=\"%s\"\n", opts->net->type);
  fprintf (fp, "WIFI_MODE=\"%s\"\n", opts->net->wifi->mode);
  fprintf (fp, "WIFI_WEP=\"%s\"\n", opts->net->wifi->wep);
  fprintf (fp, "WIFI_ESSID=\"%s\"\n", opts->net->wifi->essid);
  fprintf (fp, "HOST=\"%s\"\n", opts->net->host_ip);
  fprintf (fp, "GATEWAY=\"%s\"\n", opts->net->gateway_ip);
  fprintf (fp, "DNS_SERVER=\"%s\"\n", opts->net->dns);
  fprintf (fp, "SMB_USER=\"%s\"\n", opts->net->smb->username);
  fprintf (fp, "SMB_PWD=\"%s\"\n", opts->net->smb->password);
  fprintf (fp, "TELNET_SERVER=\"%s\"\n",
           IsDlgButtonChecked (hwnd, TELNET_SERVER) ? "yes" : "no");
  fprintf (fp, "FTP_SERVER=\"%s\"\n",
           IsDlgButtonChecked (hwnd, FTP_SERVER) ? "yes" : "no");

  fclose (fp);

  sprintf (buf, "lirc/lircrc_%s", opts->remote);
  CopyFile (buf, "iso/GEEXBOX/etc/lircrc", FALSE);
  sprintf (buf, "lirc/lircd_%s", opts->receiver);
  CopyFile (buf, "iso/GEEXBOX/etc/lircd", FALSE);
  sprintf (buf, "lirc/lircd_%s.conf", opts->remote);
  CopyFile (buf, "iso/GEEXBOX/etc/lircd.conf", FALSE);

  Execute ("win32/mkzftree.exe iso/GEEXBOX ziso/GEEXBOX");

  DeleteFile ("iso/GEEXBOX/usr/share/mplayer/help.txt");
  DeleteFile ("iso/GEEXBOX/etc/mplayer/menu.conf");

  MultipleFileDelete ("*", buf2, FALSE); /* remove subfont directory */
  RemoveDirectory (buf2);
  if (buf3[0] != '\0')
    { /* remove menufont directory */
      MultipleFileDelete ("*", buf3, FALSE);
      RemoveDirectory (buf3);
    }
  MultipleFileDelete ("lirc*", "iso/GEEXBOX/etc/", FALSE);

  CreateDirectory ("ziso/GEEXBOX/boot", NULL);
  MultipleFileCopy ("*", "iso/GEEXBOX/boot/", "ziso/GEEXBOX/boot/", "", FALSE);
  MultipleFileCopy ("*", "iso/", "ziso/", "GEEXBOX", TRUE);

  sprintf (version, "geexbox-%s-%s.iso",
           GetVersionNumber(), langs[l].shortname);
  ExecuteToFile ("win32/mkisofs -quiet -no-pad -V GEEXBOX -volset GEEXBOX -P \"The GeeXboX team (www.geexbox.org)\" -p \"The GeeXboX team (www.geexbox.org)\" -A \"MKISOFS ISO 9660/HFS FILESYSTEM BUILDER\" -z -f -D -r -J -b GEEXBOX/boot/isolinux.bin -c GEEXBOX/boot/boot.catalog -sort sort -no-emul-boot -boot-load-size 4 -boot-info-table ziso", version);

  MultipleFileDelete ("*", "ziso/", TRUE);
  RemoveDirectory ("ziso");

  sprintf (buf, "Your customized GeeXboX ISO is ready");
  printf ("*** %s ***\n", buf);
  MessageBox (hwnd, buf, "DONE", MB_OK);
}

static BOOL CALLBACK
DlgProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  char caption[100];

  switch (Message)
    {
    case WM_INITDIALOG:
      ListSubFonts (hwnd);
      ListLanguages (hwnd);
      SendDlgItemMessage (hwnd, LANG_LIST,
                          CB_SELECTSTRING, 0, (LPARAM) deflang->name);
      SendDlgItemMessage (hwnd, SUBFONT_LIST,
                          CB_SELECTSTRING, 0, (LPARAM) SUBFONT_AS_LANGUAGE);
      ListRemotes (hwnd);
      SendDlgItemMessage (hwnd, REMOTE_LIST,
                          CB_SELECTSTRING, 0, (LPARAM) "atiusb");
      ListReceivers (hwnd);
      SendDlgItemMessage (hwnd, RECEIVER_LIST,
                          CB_SELECTSTRING, 0,(LPARAM) "atiusb");
      SendDlgItemMessage(hwnd, NVIDIA_LIST, CB_ADDSTRING, 0,(LPARAM) "no");
      SendDlgItemMessage (hwnd, NVIDIA_LIST, CB_ADDSTRING, 0,(LPARAM) "yes");
      SendDlgItemMessage (hwnd, NVIDIA_LIST, CB_SELECTSTRING, 0,(LPARAM) "no");
      SendDlgItemMessage (hwnd, AUDIO_LIST, CB_ADDSTRING, 0,(LPARAM) "analog");
      SendDlgItemMessage (hwnd, AUDIO_LIST, CB_ADDSTRING, 0,(LPARAM) "spdif");
      SendDlgItemMessage (hwnd, AUDIO_LIST,
                          CB_SELECTSTRING, 0,(LPARAM) "analog");
      SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0,(LPARAM) "auto");
      SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0,(LPARAM) "wifi");
      SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0,(LPARAM) "ethernet");
      SendDlgItemMessage (hwnd, PHY_LIST, CB_SELECTSTRING, 0,(LPARAM) "auto");
      SendDlgItemMessage (hwnd, WIMO_LIST, CB_ADDSTRING, 0,(LPARAM) "managed");
      SendDlgItemMessage (hwnd, WIMO_LIST, CB_ADDSTRING, 0,(LPARAM) "ad-hoc");
      SendDlgItemMessage (hwnd, WIMO_LIST,
                          CB_SELECTSTRING, 0,(LPARAM) "managed");
      sprintf (caption, "GeeXboX Generator %s", GetVersionNumber ());

      SetWindowText (hwnd, caption);
      SetWindowText (GetDlgItem (hwnd, TEMPOIMG), "10");
      SetWindowText (GetDlgItem (hwnd, WIFIESSID), "any");
      SetWindowText (GetDlgItem (hwnd, SMBUSER), "SHARE");
      break;
    case WM_COMMAND:
      switch (LOWORD (wParam))
        {
        case IDC_OK:
          set_default_options_value (opts);
          GenerateISO (hwnd);
          break;
        case IDC_HLP:
          {
            char load[30], tmp[30];
            set_default_options_value (opts);
            sprintf (tmp, "%s%s.txt", DOCS_PATH, opts->lang);
            if (GetFileAttributes (tmp) == -1) /* File doesn't exist. */
              sprintf (tmp, "%sen.txt", DOCS_PATH);
            sprintf (load, "notepad %s", tmp);
            WinExec (load, 1);
            break;
          }
        case REMOTE_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, REMOTE_LIST, opts->remote, 50);
              break;
            }
          break;
        case RECEIVER_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, RECEIVER_LIST, opts->receiver, 50);
              break;
            }
          break;
        case NVIDIA_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText(hwnd, NVIDIA_LIST, opts->vidix, 50);
              break;
            }
          break;
        case AUDIO_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, AUDIO_LIST, opts->audio, 50);
              break;
            }
          break;
        case PHY_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, PHY_LIST, opts->net->type, 50);
              break;
            }
          break;
        case WIMO_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, WIMO_LIST, opts->net->wifi->mode, 50);
              break;
            }
          break;
        case TEMPOIMG:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, TEMPOIMG, opts->image_tempo, 50);
              break;
            }
          break;
        case WIFIWEP:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, WIFIWEP, opts->net->wifi->wep, 50);
              break;
            }
          break;
        case WIFIESSID:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, WIFIESSID, opts->net->wifi->essid, 50);
              break;
            }
          break;
        case IPGEEX:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, IPGEEX, opts->net->host_ip, 50);
              break;
            }
          break;
        case IPGAT:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, IPGAT, opts->net->gateway_ip, 50);
              break;
            }
          break;
        case IPDNS:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, IPDNS, opts->net->dns, 50);
              break;
            }
          break;
        case SMBUSER:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, SMBUSER, opts->net->smb->username, 50);
              break;
            }
          break;
        case SMBPWD:
          switch (HIWORD (wParam))
            {
            case EN_CHANGE:
              GetDlgItemText (hwnd, SMBPWD, opts->net->smb->password, 50);
              break;
            }
          break;
        case LANG_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, LANG_LIST, opts->lang, 50);
              break;
            }
          break;
        case SUBFONT_LIST:
          switch (HIWORD (wParam))
            {
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, SUBFONT_LIST, opts->subfont, 50);
              break;
            }
          break;
        case TELNET_SERVER:
          switch (HIWORD (wParam))
            {
            case BN_CLICKED:
              if (IsDlgButtonChecked (hwnd, TELNET_SERVER)) 
                CheckDlgButton (hwnd, TELNET_SERVER, BST_UNCHECKED);
              else
                CheckDlgButton (hwnd, TELNET_SERVER, BST_CHECKED);
              break; 
            }
          break;
        case FTP_SERVER:
          switch (HIWORD(wParam))
            {
            case BN_CLICKED:
              if (IsDlgButtonChecked (hwnd, FTP_SERVER))
                CheckDlgButton (hwnd, FTP_SERVER, BST_UNCHECKED);
              else
                CheckDlgButton (hwnd, FTP_SERVER, BST_CHECKED);
              break; 
            }
          break;
        }
      break;
    case WM_CLOSE:
      free_langconf ();
      free_options (opts);
      EndDialog (hwnd, 0);
      break;
    default:
      return FALSE;
    }
  return TRUE;
}

int WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
         LPSTR lpCmdLine, int nCmdShow)
{
  init_langconf ();
  opts = init_options ();
  return DialogBox (hInstance, MAKEINTRESOURCE (IDD_MAIN), NULL, DlgProc);
}
