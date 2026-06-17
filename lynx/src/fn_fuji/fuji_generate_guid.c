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
#include "fujinet-fuji.h"



bool fuji_generate_guid(char *buffer)
{
    uint16_t len;

    _net_cmd[0] = FUJICMD_GENERATE_GUID;
    if (!_fnio_send_cmd_recv(FUJI_DEVICEID_FUJINET, _net_cmd, 1, buffer, &len))
    	return false;

    return true;
}