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
