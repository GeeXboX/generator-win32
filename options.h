#ifndef OPTIONS_H_
#define OPTIONS_H_

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
  char lang[50];
  char subfont[50];
  char remote[50];
  char receiver[50];
  char vidix[50];
  char audio[50];
  char image_tempo[50];
  network_options_t *net;
} geexbox_options_t;

geexbox_options_t * init_options (void);
void free_options (geexbox_options_t *opts);

#endif
