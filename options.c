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
  return opts;
}

void
free_options (geexbox_options_t *opts)
{
  free (opts->net->wifi);
  free (opts->net->smb);
  free (opts->net);
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
  if (!strcmp (opts->audio, ""))
    strcpy (opts->audio, "analog");
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
  printf ("Audio output : %s\n", opts->audio);
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
}
