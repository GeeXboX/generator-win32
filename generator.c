#include <windows.h>
#include <stdio.h>
#include "resource.h" 

char lang[50] = "", remote[50] = "", receiver[50] = "";
char *path = "DOCS/README_";

void associate() {
  if (!strcmp(lang, ""))
    strcpy(lang, "en");
  if (!strcmp(remote, ""))
    strcpy(remote, "pctv");
  if (!strcmp(receiver, ""))
    strcpy(receiver, "pctv");
}

char *GetVersionNumber () {
  FILE *fd;
  char *version;

  version = (char *) malloc (100*sizeof(char));
  fd = fopen("VERSION", "r");
  fgets(version, 100, fd);
  if (version[strlen(version) - 1] == '\n')
    version[strlen(version) - 1] = '\0';

  return version;
}

void ListLanguages (HWND hwnd) {
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char buf[50];
  char *lang;

  lang = (char *) malloc (10*sizeof(char));
  hSearch = FindFirstFile("language/help_*.txt", &FileData);
  while (!finished) {
    lang = &FileData.cFileName[5];
    strncpy(buf, lang, strlen(lang));
    if (buf[2] == '.')
      buf[2] = '\0';
    else
      buf[3] = '\0';
    SendDlgItemMessage(hwnd, LANG_LIST, CB_ADDSTRING, 0, (LPARAM)buf);
    if (!FindNextFile(hSearch, &FileData))
      if (GetLastError() == ERROR_NO_MORE_FILES)
        finished = TRUE;
  }
  FindClose(hSearch);
}

void ListRemotes (HWND hwnd) {
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char remote[50];

  hSearch = FindFirstFile("lirc/lircrc_*", &FileData);
  while (!finished) {
    strcpy(remote, &FileData.cFileName[7]);
    SendDlgItemMessage(hwnd, REMOTE_LIST, CB_ADDSTRING, 0, (LPARAM)remote);
    if (!FindNextFile(hSearch, &FileData))
      if (GetLastError() == ERROR_NO_MORE_FILES)
        finished = TRUE;
  }
  FindClose(hSearch);
}

void ListReceivers (HWND hwnd) {
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char receiver[50];

  hSearch = FindFirstFile("lirc/lircd_*", &FileData);
  while (!finished) {
    strcpy(receiver, &FileData.cFileName[6]);
    if (strcmp (&FileData.cFileName[strlen(FileData.cFileName)-5], ".conf"))
      SendDlgItemMessage(hwnd, RECEIVER_LIST, CB_ADDSTRING,0,(LPARAM)receiver);
    if (!FindNextFile(hSearch, &FileData))
      if (GetLastError() == ERROR_NO_MORE_FILES)
        finished = TRUE;
  }
  FindClose(hSearch);
}

void MultipleFileDelete (char *token, char *src, BOOL recursive) {
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char file[100];
  char search[100];

  lstrcpy(search, src);
  lstrcat(search, token);
  hSearch = FindFirstFile(search, &FileData);

  while (!finished) {
    lstrcpy(file, src);
    lstrcat(file, FileData.cFileName);
    if (strcmp(FileData.cFileName, ".") && strcmp(FileData.cFileName, "..")) {
      if (GetFileAttributes(file) == FILE_ATTRIBUTE_DIRECTORY) {
        /* File descriptor is a directory */
        if (recursive) {
          lstrcat(file, "/");
          MultipleFileDelete(token, file, TRUE);
          printf("Removing directory : %s\n", file);
          RemoveDirectory(file);
        }
      }
      else {
        /* File descriptor is a file */
        printf("Removing file %s\n", file);
        DeleteFile(file);
      }
    }
    if (!FindNextFile(hSearch, &FileData))
      if (GetLastError() == ERROR_NO_MORE_FILES)
        finished = TRUE;
  }
  FindClose(hSearch);
}

void MultipleFileCopy (char *token, char *src, char *dest, 
                       char *exclude, BOOL recursive) {
  WIN32_FIND_DATA FileData;
  HANDLE hSearch;
  BOOL finished = FALSE;
  char newFile[100];
  char oldFile[100];
  char search[100];

  lstrcpy(search, src);
  lstrcat(search, token);
  hSearch = FindFirstFile(search, &FileData);

  while (!finished) {
    lstrcpy(oldFile, src);
    lstrcat(oldFile, FileData.cFileName);
    lstrcpy(newFile, dest);
    lstrcat(newFile, FileData.cFileName);
    if (strcmp(FileData.cFileName, ".") && strcmp(FileData.cFileName, "..")
        && strcmp (FileData.cFileName, exclude)) {
      if (GetFileAttributes(oldFile) == FILE_ATTRIBUTE_DIRECTORY) {
        /* File descriptor is a directory */
        if (recursive) {
          CreateDirectory(newFile, NULL);
          printf("Creating new directory : %s\n", newFile);
          lstrcat(oldFile, "/");
          lstrcat(newFile, "/");
          MultipleFileCopy(token, oldFile, newFile, exclude, TRUE);
        }
      }
      else {
        /* File descriptor is a file */
        printf("Copying file %s to %s\n", oldFile, newFile);
        CopyFile(oldFile, newFile, FALSE);
      }
    }
    if (!FindNextFile(hSearch, &FileData))
      if (GetLastError() == ERROR_NO_MORE_FILES)
        finished = TRUE;
  }
  FindClose(hSearch);
}

void Execute (char *cmdline) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void ExecuteToFile (char *cmdline, char *file) {
  HANDLE saveStdOutPut, output;

  saveStdOutPut = GetStdHandle(STD_OUTPUT_HANDLE);
  output = CreateFile(file, GENERIC_WRITE, FILE_SHARE_WRITE, 
                      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  SetStdHandle(STD_OUTPUT_HANDLE, output);

  if ((saveStdOutPut == INVALID_HANDLE_VALUE) 
      || (output == INVALID_HANDLE_VALUE))
    return; /* Can't get output */

  Execute(cmdline);

  /* Restore Standard Output */
  SetStdHandle(STD_OUTPUT_HANDLE, saveStdOutPut);
  CloseHandle(output);
}

void GenerateISO () {
  char buf[128], buf2[128], version[128], font[128];
  FILE *fp;

  printf("*** Generating ISO image ***\n");
  printf("Remote : %s\n", remote);
  printf("Receiver : %s\n", receiver);
  printf("Langage : %s\n", lang);

  fp = fopen ("iso/GEEXBOX/etc/lang", "w");
  fprintf (fp, "%s", lang);
  fclose (fp);

  CreateDirectory("ziso", NULL);
  sprintf(buf, "language/help_%s.txt", lang);  
  sprintf(buf2, "iso/GEEXBOX/usr/share/mplayer/help_%s.txt", lang);  
  CopyFile(buf, buf2, FALSE);
  sprintf(buf, "language/menu_%s.conf", lang);  
  sprintf(buf2, "iso/GEEXBOX/etc/mplayer/menu_%s.conf", lang);  
  CopyFile(buf, buf2, FALSE);

  if (!strcmp(lang, "cz") || !strcmp(lang, "hu") || !strcmp(lang, "pl")
      || !strcmp(lang, "sk"))
    strcpy(font, "iso-8859-2");
  else if (!strcmp(lang, "bg"))
    strcpy(font, "cp1251");
  else if (!strcmp(lang, "ru"))
    strcpy(font, "koi8r");
  else
    strcpy(font, "iso-8859-1");

  sprintf (buf, "font/%s/", font);
  sprintf (buf2, "iso/GEEXBOX/usr/share/mplayer/font/%s/", font);
  CreateDirectory(buf2, NULL);
  MultipleFileCopy("*", buf, buf2, "", FALSE);

  sprintf(buf, "lirc/lircrc_%s", remote);
  CopyFile(buf, "iso/GEEXBOX/etc/lircrc", FALSE);
  sprintf(buf, "lirc/lircd_%s", receiver);
  CopyFile(buf, "iso/GEEXBOX/etc/lircd", FALSE);
  sprintf(buf, "lirc/lircd_%s.conf", remote);
  CopyFile(buf, "iso/GEEXBOX/etc/lircd.conf", FALSE);

  Execute("win32/mkzftree.exe iso/GEEXBOX ziso/GEEXBOX");

  DeleteFile("iso/GEEXBOX/usr/share/mplayer/help.txt");
  DeleteFile("iso/GEEXBOX/etc/mplayer/menu.conf");
  MultipleFileDelete("*", buf2, FALSE);
  RemoveDirectory(buf2);
  DeleteFile("iso/GEEXBOX/usr/share/mplayer/font/font.desc");
  MultipleFileDelete("lirc*", "iso/GEEXBOX/etc/", FALSE);

  CreateDirectory("ziso/GEEXBOX/boot", NULL);
  MultipleFileCopy("*", "iso/GEEXBOX/boot/", "ziso/GEEXBOX/boot/", "", FALSE);

  MultipleFileCopy("*", "iso/", "ziso/", "GEEXBOX", TRUE);

  sprintf(version, "geexbox-%s.iso", GetVersionNumber());
  ExecuteToFile("win32/mkisofs -quiet -no-pad -V GEEXBOX -volset GEEXBOX -P \"The GeeXboX team (www.geexbox.org)\" -p \"The GeeXboX team (www.geexbox.org)\" -A \"MKISOFS ISO 9660/HFS FILESYSTEM BUILDER\" -z -f -D -r -J -b GEEXBOX/boot/isolinux.bin -c GEEXBOX/boot/boot.catalog -sort sort -no-emul-boot -boot-load-size 4 -boot-info-table ziso", version);

  MultipleFileDelete("*", "ziso/", TRUE);
  RemoveDirectory("ziso");
  
  printf("*** Your customized GeeXboX ISO is ready ***\n");
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
  char *caption;
  caption = (char *) malloc (100*sizeof(char));

  switch (Message) {

  case WM_INITDIALOG:
    ListLanguages(hwnd);
    SendDlgItemMessage(hwnd, LANG_LIST, CB_SELECTSTRING, 0, (LPARAM)"en");
    ListRemotes(hwnd);
    SendDlgItemMessage(hwnd, REMOTE_LIST, CB_SELECTSTRING, 0, (LPARAM)"pctv");
    ListReceivers(hwnd);
    SendDlgItemMessage(hwnd, RECEIVER_LIST, CB_SELECTSTRING, 0,(LPARAM)"pctv");
    sprintf(caption, "GeeXboX Generator %s", GetVersionNumber());
    SetWindowText(hwnd, caption);
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDC_OK:
      associate();
      GenerateISO();
      break;
    case IDC_HLP: {
      char load[30], tmp[30];
      associate();
      sprintf(tmp, "%s%s.txt", path, lang);
      if (GetFileAttributes(tmp) == -1)
        // File doesn't exist.
        sprintf(tmp, "%sen.txt", path);
      sprintf (load, "notepad %s", tmp);
      WinExec(load, 1);
      break;
    }
    case REMOTE_LIST:
      switch (HIWORD(wParam)) {
      case LBN_SELCHANGE:
        GetDlgItemText(hwnd, REMOTE_LIST, remote, 50);
        break;
      }
      break;   
    case RECEIVER_LIST:
      switch (HIWORD(wParam)) {
      case LBN_SELCHANGE:
        GetDlgItemText(hwnd, RECEIVER_LIST, receiver, 50);
        break;
      }
      break;   
    case LANG_LIST:
      switch (HIWORD(wParam)) {
      case LBN_SELCHANGE:
        GetDlgItemText(hwnd, LANG_LIST, lang, 50);
        break;
      }
      break;
    }
    break;

  case WM_CLOSE:
    EndDialog(hwnd, 0);
    break;

  default:
    return FALSE;
  }
  return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
  return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
}
