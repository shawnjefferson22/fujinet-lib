/**
 * @brief FujiNet Lynx serial I/O layer
 * @platform Atari Lynx
 *
 * Implements low-level ComLynx framing used by all Lynx FujiNet device
 * modules (fuji, network, clock).
 *
 * Protocol overview
 * -----------------
 * Send frame:  [dev 1B][len_hi 1B][len_lo 1B][payload N B][checksum 1B]
 *   The Lynx ComLynx bus is half-duplex, so every transmitted byte is
 *   immediately echoed back; those reflected bytes are discarded in the
 *   send path.
 *   After the full frame is sent, FujiNet replies with ACK (0x06) or
 *   NAK (0x15).
 *
 * Recv frame:  [len_hi 1B][len_lo 1B][payload N B][checksum 1B]
 *   The receiver reads the length, payload, and checksum.  If the
 *   checksum matches it replies ACK, otherwise NAK.
 *
  */

#include <6502.h>
#include <lynx.h>
#include <serial.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "lynxfnio.h"

/* ------------------------------------------------------------------ */
/* Globals                                                              */
/* ------------------------------------------------------------------ */

//uint8_t _ck;       /* checksum accumulator */
//char    _r;        /* last received byte   */
uint8_t _fn_error;   /* last error code      */

/* ------------------------------------------------------------------ */
/* Serial port init / teardown                                          */
/* ------------------------------------------------------------------ */

uint8_t fnio_init(void)
{
    struct ser_params params = {
        SER_BAUD_62500,
        SER_BITS_8,
        SER_STOP_1,
        SER_PAR_ODD,
        SER_HS_NONE
    };

    ser_install(lynx_comlynx_ser); /* activates the ComLynx port */
    CLI();
    return ser_open(&params);
}

uint8_t fnio_done(void)
{
    return ser_close();
}

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

uint8_t _checksum(char *b, uint16_t len)
{
    uint16_t i;
    uint8_t ck;

    ck = 0;
    for (i = 0; i < len; ++i)
        ck ^= (uint8_t)b[i];

    return(ck);
}

uint8_t _serial_get_loop(char *b)
{
    clock_t start, now;   
    start = clock();

    while (ser_get(b) == SER_ERR_NO_DATA) {
        now = clock();
        if (((now - start) / CLOCKS_PER_SEC) > LYNX_FN_RECV_TIMEOUT) {
            _fn_error = FNIO_ERR_TIMEOUT;
            return 0;
        }
    }
    return 1;
}

void fnio_flush_recv(void)
{
    char b;

    while (ser_get(&b) != SER_ERR_NO_DATA)
        ;
}

uint8_t fnio_error(void)
{
    return _fn_error;
}

/* ------------------------------------------------------------------ */
/* Framed send                                                          */
/* ------------------------------------------------------------------ */

uint8_t fnio_send_buf(uint8_t dev, char *buf, uint16_t len)
{
    uint16_t i;
    uint8_t ck;
    char b;

    _fn_error = FNIO_ERR_NONE;

    /* compute checksum over payload */
    ck = _checksum(buf, len);

    /* send header: device ID + 16-bit big-endian length */
    ser_put((char)dev);
    ser_put((char)(len >> 8));
    ser_put((char)(len & 0xFF));

    /* discard the three echoed header bytes */
    ser_get(&b);
    ser_get(&b);
    ser_get(&b);

    /* send payload, discarding each reflected byte */
    for (i = 0; i < len; ++i) {
        ser_put(buf[i]);
        ser_get(&b);
    }

    /* send checksum, discard echo */
    ser_put((char) ck);
    ser_get(&b);

    /* wait for ACK / NAK */
    if (!_serial_get_loop(&b))
        return 0;

    if ((uint8_t) b == FUJICMD_ACK)
        return 1;

    _fn_error = FNIO_ERR_SEND_CHK;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Framed receive                                                     */
/* ------------------------------------------------------------------ */

uint8_t fnio_recv_buf(char *buf, uint16_t *len)
{
    uint16_t i;
    uint8_t ck;
    char b;

    _fn_error = FNIO_ERR_NONE;

    /* receive high byte of length */
    if (!_serial_get_loop(&b)) return 0;
    *len = (uint16_t)((uint8_t) b) << 8;

    /* receive low byte of length */
    if (!_serial_get_loop(&b)) return 0;
    *len |= (uint8_t) b;

    if (*len > LYNX_FN_LEN_MAX)
        *len = LYNX_FN_LEN_MAX;

    /* receive payload */
    for (i = 0; i < *len; ++i) {
        if (!_serial_get_loop(&b)) return 0;
        buf[i] = b;
    }

    /* receive checksum byte */
    if (!_serial_get_loop(&b)) return 0;

    /* verify */
    ck = _checksum(buf, *len);
    if ((uint8_t) b == ck) {
        ser_put((char)FUJICMD_ACK);
        ser_get(&b); /* discard echo */
        return 1;
    } else {
        _fn_error = FNIO_ERR_RECV_CHK;
        ser_put((char) FUJICMD_NAK);
        ser_get(&b); /* discard echo */
        *len = 0;
        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Receive a bare ACK/NAK (for commands with no data response)          */
/* ------------------------------------------------------------------ */

uint8_t fnio_recv_ack(void)
{
    char b;

    _fn_error = FNIO_ERR_NONE;

    if (!_serial_get_loop(&b)) return 0;

    if (b == FUJICMD_ACK)
        return 1;

    _fn_error = FNIO_ERR_GENERAL;
    fnio_flush_recv();
    return 0;
}

/* ------------------------------------------------------------------ */
/* High-level helpers used by device modules                            */
/* ------------------------------------------------------------------ */

/*
 * Send cmd buf to `dev`, wait for completion ACK.
 * Retries up to LYNX_FN_RETRIES times on send failure.
 */
uint8_t _fnio_send_cmd(uint8_t dev, char *buf, uint16_t len)
{
    uint8_t r, i;

    for (i = 0; i < LYNX_FN_RETRIES; ++i) {
        r = fnio_send_buf(dev, buf, len);
        if (r) break;
    }
    if (!r) return 0;

    return fnio_recv_ack();
}

/*
 * Send cmd buf to `dev`, then receive a data frame into `out`.
 * Retries up to LYNX_FN_RETRIES times on send failure.
 */
uint8_t _fnio_send_cmd_recv(uint8_t dev, char *buf, uint16_t cmd_len,
                             char *out, uint16_t *out_len)
{
    uint8_t r, i;

    for (i = 0; i < LYNX_FN_RETRIES; ++i) {
        r = fnio_send_buf(dev, buf, cmd_len);
        if (r) break;
    }
    if (!r) return 0;

    r = fnio_recv_buf(out, out_len);
    return (r && (*out_len != 0)) ? 1 : 0;
}
