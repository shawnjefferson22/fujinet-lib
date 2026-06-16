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
 * @brief  Get Network Device Status byte
 * @param  devicespec pointer to device specification of form: N:PROTO://[HOSTNAME]:PORT/PATH/.../
 * @param  bw pointer to where to put bytes waiting
 * @param  c pointer to where to put connection status
 * @param  err to where to put network error byte.
 * @return fujinet-network status/error code (See FN_ERR_* values)
 */
uint8_t network_status(const char *devicespec, uint16_t *bw, uint8_t *c, uint8_t *err)
{
    uint8_t  dev = _net_dev(devicespec);
    uint16_t rlen;
    uint8_t  status[4];

    _net_cmd[0] = FUJICMD_STATUS;
    if (!_fnio_send_cmd_recv(dev, _net_cmd, 1, (char *)status, &rlen)) {
        return fn_error(fnio_error());
    }

    *bw  = (uint16_t)status[0] | ((uint16_t)status[1] << 8);
    *c   = status[2];
    *err = status[3];

    fn_network_bw    = *bw;
    fn_network_conn  = *c;
    fn_network_error = *err;

    return FN_ERR_OK;
}
