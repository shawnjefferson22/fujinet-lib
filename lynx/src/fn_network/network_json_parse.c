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



/**
 * @brief  Parse the currently open JSON location
 * @param  devicespec pointer to device specification, e.g. "N1:HTTPS://fujinet.online/"
 * @return fujinet-network error code (See FN_ERR_* values)
 *
 * This will set the channel mode to JSON, which will be unset in the close.
 */
uint8_t network_json_parse(const char *devicespec)
{
   uint8_t dev = _net_dev(devicespec);

    _net_cmd[0] = NETCMD_JSON_PARSE;
    if (!_fnio_send_cmd(dev, _net_cmd, 1))
        return fn_error(fnio_error());

    return FN_ERR_OK;
}
