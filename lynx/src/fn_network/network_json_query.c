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
 * @brief  Perform JSON query
 * @param  devicespec pointer to device specification, e.g. "N1:HTTPS://fujinet.online/"
 * @param  query pointer to string containing json path to query, e.g. "/path/field". No need to add device drive.
 * @param  s pointer to receiving string, nul terminated, if no data was retrieved, sets it to an empty string
 * @return Bytes read, or negative values represent fujinet-network error code (See FN_ERR_* values)
 * 
 * Assumes an open and parsed json.
 */
int16_t network_json_query(const char *devicespec, const char *query, char *s)
{
    uint8_t  dev = _net_dev(devicespec);
    uint16_t qlen = strlen(query);
    uint16_t rlen;

    _net_cmd[0] = FUJICMD_JSON_QUERY;
    memcpy(&_net_cmd[1], query, qlen + 1);

    if (!_fnio_send_cmd_recv(dev, _net_cmd, (qlen + 2), s, &rlen)) {
        s[0] = '\0';
        return -fn_error(fnio_error());
    }

    return rlen;
}