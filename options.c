/*
 *  options.c : GeeXboX Win32 generator options management.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <windows.h>

#include "resource.h"
#include "options.h"
#include "langconf.h"

geexbox_options_t *
init_options (void)
{
  geexbox_options_t *opts;

  opts = (geexbox_options_t *) malloc (sizeof (geexbox_options_t));
  opts->net = (network_options_t *) malloc (sizeof (network_options_t));
  opts->net->wifi = (wifi_options_t *) malloc (sizeof (wifi_options_t));
  opts->net->smb = (samba_options_t *) malloc (sizeof (samba_options_t));
  opts->snd = (sound_options_t *) malloc (sizeof (sound_options_t));

  *opts->lang = *opts->subfont = *opts->remote = *opts->receiver = '\0';
  *opts->vidix = '\0';
  *opts->net->type = *opts->net->host_ip = '\0';
  *opts->net->gateway_ip = *opts->net->dns = '\0';
  *opts->net->wifi->mode = *opts->net->wifi->wep = '\0';
  *opts->net->wifi->essid = '\0';
  *opts->net->smb->username = *opts->net->smb->password = '\0';
  opts->snd->card_id = 0;
  opts->snd->mode = SOUND_MODE_ANALOG;
  opts->snd->channels = 2;

  return opts;
}

void
free_options (geexbox_options_t *opts)
{
  free (opts->net->wifi);
  free (opts->net->smb);
  free (opts->net);
  free (opts->snd);
  free (opts);
}

void
set_default_options_value (geexbox_options_t *opts)
{
  if (!strcmp (opts->lang, ""))
    strcpy (opts->lang, deflang->name);
  if (!strcmp (opts->subfont, ""))
    strcpy (opts->subfont, SUBFONT_AS_LANGUAGE);
  if (!strcmp (opts->remote, ""))
    strcpy (opts->remote, "atiusb");
  if (!strcmp (opts->receiver, ""))
    strcpy (opts->receiver, "atiusb");
  if (!strcmp (opts->vidix, ""))
    strcpy (opts->vidix, "no");
  if (!strcmp (opts->net->type, ""))
    strcpy (opts->net->type, "auto");
  if (!strcmp (opts->net->wifi->mode, ""))
    strcpy (opts->net->wifi->mode, "managed");
}

void
display_options_to_console (HWND hwnd, geexbox_options_t *opts)
{
  printf ("Remote : %s\n", opts->remote);
  printf ("Receiver : %s\n", opts->receiver);
  printf ("Language : %s\n", opts->lang);
  printf ("Subfont : %s\n", opts->subfont);
  printf ("NVidia vidix : %s\n", opts->vidix);
  printf ("Image tempo : %ss\n", opts->image_tempo);
  printf ("Network type : %s\n", opts->net->type);
  printf ("Wifi mode : %s\n", opts->net->wifi->mode);
  printf ("Wifi WEP key : %s\n", opts->net->wifi->wep);
  printf ("Geexbox IP : %s\n", opts->net->host_ip);
  printf ("Gateway IP : %s\n", opts->net->gateway_ip);
  printf ("DNS Server IP : %s\n", opts->net->dns);
  printf ("User login : %s\n", opts->net->smb->username);
  printf ("User Password : %s\n", opts->net->smb->password);
  printf ("Enable Telnet Server : %s\n",
          IsDlgButtonChecked (hwnd, TELNET_SERVER) ? "yes" : "no");
  printf ("Enable FTP Server : %s\n",
          IsDlgButtonChecked (hwnd, FTP_SERVER) ? "yes" : "no");
  printf ("Audio Card's ID : %d\n", opts->snd->card_id);
  printf ("Audio Mode : %s\n",
          (opts->snd->mode == SOUND_MODE_SPDIF) ? "spdif" : "analog");
  printf ("Nr. of Channels : %d\n", opts->snd->channels);
  printf ("AC3 Decoding : %s\n",
          IsDlgButtonChecked (hwnd, AUDIO_HWAC3) ? "hardware" : "software");
}

int
write_options_to_disk (HWND hwnd, geexbox_options_t *opts,
                       char *subfont_dir, char *menufont_dir)
{
  FILE *fp;
  char buf[128];
  int res;

  res = write_lang_to_disk (hwnd, opts, subfont_dir, menufont_dir);
  if (res < 0)
    return res;

  fp = fopen ("iso/GEEXBOX/etc/audio", "wb");
  fprintf (fp, "ALSA_CARD=\"%d\"\n", opts->snd->card_id);
  fprintf (fp, "SOUNDCARD_MODE=\"%s\"\n",
           (opts->snd->mode == SOUND_MODE_SPDIF) ? "SPDIF" : "analog");
  fprintf (fp, "AC3_DECODER=\"%s\"\n",
           IsDlgButtonChecked (hwnd, AUDIO_HWAC3) ? "hardware" : "software");
  fprintf (fp, "CHANNELS=\"%d\"\n", opts->snd->channels);
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

  return res;
}

static void
get_config_value (FILE *fp, const char *var, char *dst)
{
  static char buf[256];
  char *line, *value, *end;

  rewind (fp);
  while ((line = fgets (buf, sizeof (buf), fp)))
    {
      while (isspace (*line))
        line++;
      while (isspace (line[strlen (line) - 1]))
        line[strlen (line) - 1] = '\0';

      if (*line == '#' || (value = strchr (line, '=')) == NULL)
        continue;

      *value++ = '\0';

      if (strcmp (line, var))
        continue;

      while (*value != '"')
        *value++;
      *value++;
      
      end = strchr (value, '"');
      value[strlen (value) - strlen (end)] = '\0';
      strcpy (dst, value);
      return;
    }

  *dst = '\0';
}

void
read_options_from_disk (HWND hwnd, geexbox_options_t *opts)
{
  FILE *fp;

  fp = fopen ("iso/GEEXBOX/etc/audio", "r");
  if (fp)
    {
      char tmp[50];
      get_config_value (fp, "ALSA_CARD", tmp);
      opts->snd->card_id = atoi (tmp);

      get_config_value (fp, "SOUNDCARD_MODE", tmp);
      if (!strcmp (tmp, "SPDIF"))
        opts->snd->mode = SOUND_MODE_SPDIF;
      else
        opts->snd->mode = SOUND_MODE_ANALOG;

      get_config_value (fp, "CHANNELS", tmp);
      opts->snd->channels = atoi (tmp);

      get_config_value (fp, "AC3_DECODER", tmp);
      if (!strcmp (tmp, "software"))
        CheckDlgButton (hwnd, AUDIO_HWAC3, BST_UNCHECKED);
      else
        CheckDlgButton (hwnd, AUDIO_HWAC3, BST_CHECKED);
      fclose (fp);         
    }

  fp = fopen ("iso/GEEXBOX/etc/mplayer/no_nvidia_vidix", "r");
  if (fp)
    {
      strcpy (opts->vidix, "no");
      fclose (fp);
    }
  else
    strcpy (opts->vidix, "yes");

  fp = fopen ("iso/GEEXBOX/etc/view_img_timeout", "r");
  if (fp)
    {
      fscanf (fp, "%s", opts->image_tempo);
      fclose (fp);
    }

  fp = fopen ("iso/GEEXBOX/etc/network", "r");
  if (fp)
    {
      char tmp[50];
      get_config_value (fp, "PHY_TYPE", opts->net->type);
      get_config_value (fp, "WIFI_MODE", opts->net->wifi->mode);
      get_config_value (fp, "WIFI_WEP", opts->net->wifi->wep);
      get_config_value (fp, "WIFI_ESSID", opts->net->wifi->essid);
      get_config_value (fp, "HOST", opts->net->host_ip);
      get_config_value (fp, "GATEWAY", opts->net->gateway_ip);
      get_config_value (fp, "DNS_SERVER", opts->net->dns);
      get_config_value (fp, "SMB_USER", opts->net->smb->username);
      get_config_value (fp, "SMB_PWD", opts->net->smb->password);

      get_config_value (fp, "TELNET_SERVER", tmp);
      if (!strcmp (tmp, "yes"))
        CheckDlgButton (hwnd, TELNET_SERVER, BST_CHECKED);
      else
        CheckDlgButton (hwnd, TELNET_SERVER, BST_UNCHECKED);

      get_config_value (fp, "FTP_SERVER", tmp);
      if (!strcmp (tmp, "yes"))
        CheckDlgButton (hwnd, FTP_SERVER, BST_CHECKED);
      else
        CheckDlgButton (hwnd, FTP_SERVER, BST_UNCHECKED);
      fclose (fp);
    }
}
