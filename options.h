/*
 *  options.h : GeeXboX Win32 generator options management.
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

#ifndef OPTIONS_H_
#define OPTIONS_H_

#define SOUND_MODE_ANALOG 0
#define SOUND_MODE_SPDIF 1
#define SOUND_MODE_ANALOG_STR "Analog"
#define SOUND_MODE_SPDIF_STR "S/PDIF"

#define SOUND_CHANNELS_2 "Stereo (2)"
#define SOUND_CHANNELS_4 "Surround (4)"
#define SOUND_CHANNELS_6 "5.1 Surround (6)"

typedef struct {
  char mode[50];
  char wep[50];
  char essid[50];
} wifi_options_t;

typedef struct {
  char username[50];
  char password[50];
} samba_options_t;

typedef struct {
  char type[50];
  char host_ip[50];
  char gateway_ip[50];
  char dns[50];
  wifi_options_t *wifi;
  samba_options_t *smb;
} network_options_t;

typedef struct {
  int card_id;
  int mode;
  int channels;
} sound_options_t;

typedef struct {
  char lang[50];
  char subfont[50];
  char remote[50];
  char receiver[50];
  char vidix[50];
  char image_tempo[50];
  sound_options_t *snd;
  network_options_t *net;
} geexbox_options_t;

geexbox_options_t * init_options (void);
void free_options (geexbox_options_t *opts);
void set_default_options_value (geexbox_options_t *opts);
void display_options_to_console (HWND hwnd, geexbox_options_t *opts);
int write_options_to_disk (HWND hwnd, geexbox_options_t *opts,
                           char *subfont_dir, char *menufont_dir);
void read_options_from_disk (HWND hwnd, geexbox_options_t *opts);

#endif
