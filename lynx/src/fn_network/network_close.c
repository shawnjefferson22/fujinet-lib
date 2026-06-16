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
#include "fujinet-network.h"



/**
 * @brief  Close Connection
 * @param  devicespec pointer to device specification of form: N:PROTO://[HOSTNAME]:PORT/PATH/.../
 * @return fujinet-network error code (See FN_ERR_* values)
 */
uint8_t network_close(const char* devicespec)
{
   uint8_t dev = _net_dev(devicespec);

    _net_cmd[0] = FUJICMD_CLOSE;
    if (!_fnio_send_cmd(dev, _net_cmd, 1))
        return fn_error(fnio_error());

    return FN_ERR_OK;
}