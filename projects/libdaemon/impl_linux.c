#include <stdlib.h>
#include "api.h"

hbl_service_config_t  *gHblConf = 0;

hbl_service_config_t* hbl_service_create_config()
{
  hbl_service_config_t *c = malloc(sizeof(hbl_service_config_t));
  c->start = 0;
  c->stop = 0;
  return c;
}

void hbl_service_free_config(hbl_service_config_t *c)
{
  if (c)
    free(c);
}

int hbl_service_run(hbl_service_config_t *conf)
{
  if (!conf)
    return 1;
  gHblConf = conf;
  return 0;
}
