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
 * @brief  Write to network
 * @param  devicespec pointer to device specification, e.g. "N1:HTTPS://fujinet.online/"
 * @param  buf Buffer
 * @param  len length
 * @return fujinet-network error code (See FN_ERR_* values)
 */
uint8_t network_write(const char* devicespec, const uint8_t *buf, uint16_t len)
{
    uint8_t dev = _net_dev(devicespec);

    _net_cmd[0] = NETCMD_WRITE;
    _net_cmd[1] = (char)(len & 0xFF);
    _net_cmd[2] = (char)(len >> 8);
    memcpy(&_net_cmd[3], buf, len);

    if (!_fnio_send_cmd(dev, _net_cmd, len + 3))
        return fn_error(fnio_error());

    return FN_ERR_OK;
}