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
#include "fujinet_network.h"



/**
 * @brief  Read from channel
 * 
 * The read will block until it has read all the bytes requested from the device, or the EOF is hit.
 * This will block waiting for as much data as it can, so that the client does not need to handle counting.
 * Errors are returned as the negative value of the error. fn_network_error contains the device specific error code. fn_bytes_read will contain the count of bytes read before error occurred.
 * 
 * @param  devicespec pointer to device specification, e.g. "N1:HTTPS://fujinet.online/"
 * @param  buf Buffer
 * @param  len length
 * @return Bytes read, or negative value of fujinet-network error code (See FN_ERR_* values) with fn_network_error containing real error code
 */
int16_t network_read_lynx(const char* devicespec, uint8_t *buf, uint16_t len)
{
    uint8_t dev = _net_dev(devicespec);
    uint16_t rlen;

    _net_cmd[0] = FUJICMD_READ;
    if (!_fnio_send_cmd_recv(dev, _net_cmd, 1, (char *) buf, &rlen))
        return -fn_error(fnio_error());

    return rlen;
}
