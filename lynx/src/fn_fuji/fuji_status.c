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


bool fuji_status(FNStatus *status)
{
    uint16_t len;

    _net_cmd[0] = FUJICMD_STATUS;
    if (!_fnio_send_cmd_recv(FUJI_DEVICEID_FUJINET, _net_cmd, 1, (char *) status, &len))
    	return false;

    return true;
}
