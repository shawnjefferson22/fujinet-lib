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
 * @brief  Device specific direct control commands
 * @param  cmd Command byte to send
 * @param  aux1 Auxiliary byte 1
 * @param  aux2 Auxiliary byte 2
 * @param  devicespec pointer to device specification, e.g. "N1:HTTPS://fujinet.online/"
 * @param  ... varargs - Device specific additional parameters to pass to the network device
 * @return fujinet-network error code (See FN_ERR_* values)
 */
uint8_t network_ioctl(uint8_t cmd, uint8_t aux1, uint8_t aux2, const char* devicespec, ...)
{
    uint8_t dev = _net_dev(devicespec);

    _net_cmd[0] = (char)cmd;

    // Ignore the aux1 and aux2 bytes on Lynx
    // and ignore the varags, not used

	(void) aux1;
	(void) aux2;

    if (!_fnio_send_cmd(dev, _net_cmd, 1))
        return fn_error(fnio_error());

    return FN_ERR_OK;
}
