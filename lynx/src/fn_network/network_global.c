/**
 * @brief FujiNet Fuji device for Atari Lynx
 * @platform Atari Lynx
 *
 * Implements the fujinet-fuji.h API using the Lynx ComLynx serial
 * transport (lynxfnio).
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "lynxfnio.h"
#include "fujinet-network-lynx.h"
#include "fujinet-network.h"


/* Command / send buffer */
char _net_cmd[512];

/*
 * Return the device ID for a given unit number (1-based).
 * Unit 1 → 0x71, unit 2 → 0x72, …
 */
uint8_t _net_dev(const char *devicespec)
{
    return (uint8_t) (FUJI_DEVICEID_NETWORK + network_unit(devicespec) - 1);
}
