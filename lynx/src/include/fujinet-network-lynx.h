#ifndef FUJINET_NETWORK_LYNX
#define FUJINET_NETWORK_LYNX

#include <stdint.h>


extern char _net_cmd[512];

uint8_t _net_dev(const char *devicespec);
int16_t network_read_lynx(const char* devicespec, uint8_t *buf, uint16_t len);

#endif