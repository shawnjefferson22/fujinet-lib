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



int16_t network_read_lynx(const char* devicespec, uint8_t *buf, uint16_t len)
{
    uint8_t dev = _net_dev(devicespec);
    uint16_t rlen;

	// might need this in the future, not sure
	(void) len;

    _net_cmd[0] = NETCMD_READ;
    if (!_fnio_send_cmd_recv(dev, _net_cmd, 1, (char *) buf, &rlen))
        return -fn_error(fnio_error());

    return rlen;
}
