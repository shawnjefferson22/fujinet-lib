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
 * Convert device specific error in code to FujiNet Network library error, agnostic of device.
 * Library code calls this when it encounters an error to return value applications should use.
 */
uint8_t fn_error(uint8_t code) 
{
    fn_device_error = code;    
    switch(code) {
        case FNIO_ERR_NONE:
            return (FN_ERR_OK);
        case FNIO_ERR_GENERAL:
        case FNIO_ERR_TIMEOUT:
            return(FN_ERR_UNKNOWN);
        case FNIO_ERR_SEND_CHK:
        case FNIO_ERR_RECV_CHK:
            return(FN_ERR_IO_ERROR);
    }

    return(FN_ERR_UNKNOWN);
}
