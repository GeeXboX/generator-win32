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
#include "fs.h"

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
  char subfont_dir[128], menufont_dir[128], version[128];
  int l;

  printf ("*** Generating ISO image ***\n");
  display_options_to_console (hwnd, opts);
  l = write_options_to_disk (hwnd, opts, subfont_dir, menufont_dir);
  if (l < 0)
    return;

  Execute ("win32/mkzftree.exe iso/GEEXBOX ziso/GEEXBOX");

  DeleteFile ("iso/GEEXBOX/usr/share/mplayer/help.txt");
  DeleteFile ("iso/GEEXBOX/etc/mplayer/menu.conf");

  MultipleFileDelete ("*", subfont_dir, FALSE); /* remove subfont directory */
  RemoveDirectory (subfont_dir);
  if (menufont_dir[0] != '\0')
    { /* remove menufont directory */
      MultipleFileDelete ("*", menufont_dir, FALSE);
      RemoveDirectory (menufont_dir);
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

  printf ("*** Your customized GeeXboX ISO is ready ***\n");
  MessageBox (hwnd, "Your customized GeeXboX ISO is ready", "DONE", MB_OK);
}

static void
set_items_values (HWND hwnd)
{
  char tmp[50];

  /* set main title */
  sprintf (tmp, "GeeXboX Generator %s", GetVersionNumber ());
  SetWindowText (hwnd, tmp);

  /* list languages */
  ListLanguages (hwnd);
  SendDlgItemMessage (hwnd, LANG_LIST,
                      CB_SELECTSTRING, 0, (LPARAM) deflang->name);

  /* list subfonts */
  ListSubFonts (hwnd);
  SendDlgItemMessage (hwnd, SUBFONT_LIST,
                      CB_SELECTSTRING, 0, (LPARAM) SUBFONT_AS_LANGUAGE);

  /* list remotes */
  ListRemotes (hwnd);
  SendDlgItemMessage (hwnd, REMOTE_LIST,
                      CB_SELECTSTRING, 0, (LPARAM) "atiusb");

  /* list receivers */
  ListReceivers (hwnd);
  SendDlgItemMessage (hwnd, RECEIVER_LIST,
                      CB_SELECTSTRING, 0, (LPARAM) "atiusb");

  /* set nvidia vidix item */
  SendDlgItemMessage(hwnd, NVIDIA_LIST, CB_ADDSTRING, 0, (LPARAM) "no");
  SendDlgItemMessage (hwnd, NVIDIA_LIST, CB_ADDSTRING, 0, (LPARAM) "yes");
  SendDlgItemMessage (hwnd, NVIDIA_LIST, CB_SELECTSTRING, 0,
                      (LPARAM) ((opts->vidix) ? opts->vidix : "no"));

  /* set image timeout value */
  SetWindowText (GetDlgItem (hwnd, TEMPOIMG),
                 (opts->image_tempo) ? opts->image_tempo : "10");

  /* audio card's ID */
  sprintf (tmp, "%d", (opts->snd->card_id) ? opts->snd->card_id : 0);
  SetWindowText (GetDlgItem (hwnd, AUDIO_CARD), tmp);

  /* audio mode */
  SendDlgItemMessage (hwnd, AUDIO_MODE_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SOUND_MODE_ANALOG_STR);
  SendDlgItemMessage (hwnd, AUDIO_MODE_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SOUND_MODE_SPDIF_STR);
  if (opts->snd->mode == SOUND_MODE_SPDIF)
    SendDlgItemMessage (hwnd, AUDIO_MODE_LIST,
                        CB_SELECTSTRING, 0, (LPARAM) SOUND_MODE_SPDIF_STR);
  else
    SendDlgItemMessage (hwnd, AUDIO_MODE_LIST,
                        CB_SELECTSTRING, 0, (LPARAM) SOUND_MODE_ANALOG_STR);

  /* audio channels number */
  SendDlgItemMessage (hwnd, AUDIO_CHANNELS_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SOUND_CHANNELS_2);
  SendDlgItemMessage (hwnd, AUDIO_CHANNELS_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SOUND_CHANNELS_4);
  SendDlgItemMessage (hwnd, AUDIO_CHANNELS_LIST,
                      CB_ADDSTRING, 0, (LPARAM) SOUND_CHANNELS_6);
  if (opts->snd->channels == 6)
    strcpy (tmp, SOUND_CHANNELS_6);
  else if (opts->snd->channels == 4)
    strcpy (tmp, SOUND_CHANNELS_4);
  else
    strcpy (tmp, SOUND_CHANNELS_2);
  SendDlgItemMessage (hwnd, AUDIO_CHANNELS_LIST,
                      CB_SELECTSTRING, 0, (LPARAM) tmp);

  /* network type */
  SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0, (LPARAM) "auto");
  SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0, (LPARAM) "wifi");
  SendDlgItemMessage (hwnd, PHY_LIST, CB_ADDSTRING, 0, (LPARAM) "ethernet");
  SendDlgItemMessage (hwnd, PHY_LIST, CB_SELECTSTRING, 0,
                      (LPARAM) (opts->net->type ? opts->net->type : "auto"));

  /* wifi mode */
  SendDlgItemMessage (hwnd, WIMO_LIST, CB_ADDSTRING, 0, (LPARAM) "managed");
  SendDlgItemMessage (hwnd, WIMO_LIST, CB_ADDSTRING, 0, (LPARAM) "ad-hoc");
  SendDlgItemMessage (hwnd, WIMO_LIST, CB_SELECTSTRING, 0,
                      (LPARAM) (opts->net->wifi->mode
                                ? opts->net->wifi->mode : "managed"));

  /* wifi wep key */
  SetWindowText (GetDlgItem (hwnd, WIFIWEP),
                 (opts->net->wifi->wep ? opts->net->wifi->wep : ""));

  /* wifi ssid */
  SetWindowText (GetDlgItem (hwnd, WIFIESSID),
                 (opts->net->wifi->essid ? opts->net->wifi->essid : "any"));

  /* network host IP */
  SetWindowText (GetDlgItem (hwnd, IPGEEX),
                 (opts->net->host_ip ? opts->net->host_ip : ""));

  /* network gateway IP */
  SetWindowText (GetDlgItem (hwnd, IPGAT),
                 (opts->net->gateway_ip ? opts->net->gateway_ip : ""));

  /* network DNS server IP */
  SetWindowText (GetDlgItem (hwnd, IPDNS),
                 (opts->net->dns ? opts->net->dns : ""));

  /* samba username */
  SetWindowText (GetDlgItem (hwnd, SMBUSER),
                 (opts->net->smb->username ? opts->net->smb->username : ""));

  /* samba password */
  SetWindowText (GetDlgItem (hwnd, SMBPWD),
                 (opts->net->smb->password ? opts->net->smb->password : ""));
}

static BOOL CALLBACK
DlgProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch (Message)
    {
    case WM_INITDIALOG:
      read_options_from_disk (hwnd, opts);
      set_items_values (hwnd);
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
        case AUDIO_CARD:
          switch (HIWORD (wParam))
            {
              char tmp[50];
            case EN_CHANGE:
              GetDlgItemText (hwnd, AUDIO_CARD, tmp, 50);
              opts->snd->card_id = atoi (tmp);
              break;
            }
          break;
        case AUDIO_MODE_LIST:
          switch (HIWORD (wParam))
            {
              char tmp[50];
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, AUDIO_MODE_LIST, tmp, 50);
              if (!strcmp (tmp, SOUND_MODE_SPDIF_STR))
                opts->snd->mode = SOUND_MODE_SPDIF;
              else
                opts->snd->mode = SOUND_MODE_ANALOG;
              break;
            }
          break;
        case AUDIO_CHANNELS_LIST:
          switch (HIWORD (wParam))
            {
              char tmp[50];
            case LBN_SELCHANGE:
              GetDlgItemText (hwnd, AUDIO_CHANNELS_LIST, tmp, 50);
              if (!strcmp (tmp, SOUND_CHANNELS_6))
                opts->snd->channels = 6;
              else if (!strcmp (tmp, SOUND_CHANNELS_4))
                opts->snd->channels = 4;
              else
                opts->snd->channels = 2;
              break;
            }
          break;
        case AUDIO_HWAC3:
          switch (HIWORD (wParam))
            {
            case BN_CLICKED:
              if (IsDlgButtonChecked (hwnd, AUDIO_HWAC3)) 
                CheckDlgButton (hwnd, AUDIO_HWAC3, BST_UNCHECKED);
              else
                CheckDlgButton (hwnd, AUDIO_HWAC3, BST_CHECKED);
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
