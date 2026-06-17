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


/*
 * Pointer to the protocol + path portion of a devicespec, e.g.
 * "N1:TCP://host:port/" → "TCP://host:port/"
 */
static const char *_net_url(const char *devicespec)
{
    const char *p = devicespec;
    /* skip "Nx:" prefix */
    while (*p && *p != ':') p++;
    if (*p == ':') p++;
    return p;
}


/**
 * @brief  Open Connection
 * @param  devicespec pointer to device specification of form: N:PROTO://[HOSTNAME]:PORT/PATH/.../
 * @param  mode (4=read, 8=write, 12=read/write, 13=POST, etc.)
 * @param  trans translation mode (CR/LF to other line endings; 0=none, 1=CR, 2=LF, 3=CRLF, 4=Pet)
 * @return fujinet-network error code (See FN_ERR_* values)
 */
uint8_t network_open(const char* devicespec, uint8_t mode, uint8_t trans)
{
    uint8_t dev = _net_dev(devicespec);
    const char *url = _net_url(devicespec);
    uint16_t ulen = strlen(url);

    _net_cmd[0] = NETCMD_OPEN;
    _net_cmd[1] = mode;
    _net_cmd[2] = trans;
    memcpy(&_net_cmd[3], url, ulen + 1); /* include NUL */

    if (!_fnio_send_cmd(dev, _net_cmd, (uint16_t)(ulen + 4)))
        return fn_error(fnio_error());

    return FN_ERR_OK;
}