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
#include "options.h"

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
