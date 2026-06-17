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

extern uint16_t ak_creator_id;
extern uint8_t ak_app_id;

// open appkey needs
// creator (2 bytes)
// app (1)
// key
// mode
// reserved



bool fuji_write_appkey(uint8_t key_id, uint16_t count, uint8_t *data)
{
	// Open AppKey
	_net_cmd[0] = FUJICMD_OPEN_APPKEY;
 	_net_cmd[1] = (char)(ak_creator_id & 0xFF);
    _net_cmd[2] = (char)(ak_creator_id >> 8);
    _net_cmd[3] = (char) ak_app_id;
    _net_cmd[4] = (char) key_id;
	_net_cmd[5] = 0;		// mode
	_net_cmd[6] = 0;		// reserved

	if (!_fnio_send_cmd(FUJI_DEVICEID_FUJINET, _net_cmd, 7))
		return false;

    // Read AppKey
    _net_cmd[0] = FUJICMD_WRITE_APPKEY;
    memcpy(&_net_cmd[1], data, count);
    if (!_fnio_send_cmd(FUJI_DEVICEID_FUJINET, _net_cmd, count+1))
    	return false;

	// Close AppKey (check return value?)
	_net_cmd[0] = FUJICMD_CLOSE_APPKEY;
	_fnio_send_cmd(FUJI_DEVICEID_FUJINET, _net_cmd, 1);

    return true;
}
