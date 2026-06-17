/**
 * @brief FujiNet Lynx serial I/O layer
 * @platform Atari Lynx
 *
 * Low-level ComLynx serial communication routines used by all
 * Lynx FujiNet device implementations (fuji, network, clock).
 *
 * Adapted from fujinet-lynx-config by Thom Cherryhomes / Shawn Jefferson.
 */

#ifndef LYNXFNIO_H
#define LYNXFNIO_H

#include <stdint.h>

/* Maximum payload size for a single send/recv transaction */
#define LYNX_FN_LEN_MAX 1024
#define LYNX_FN_RECV_MAX 256

/* Receive timeout in seconds */
#define LYNX_FN_RECV_TIMEOUT 10

/* Number of command retries before giving up */
#define LYNX_FN_RETRIES 3

/* ------------------------------------------------------------------ */
/* Device IDs                                                           */
/* ------------------------------------------------------------------ */

#define FUJI_DEVICEID_FUJINET       0x70
#define FUJI_DEVICEID_NETWORK       0x71
#define FUJI_DEVICEID_NETWORK_LAST  0x78
#define FUJI_DEVICEID_CLOCK         0x45
#define FUJI_DEVICEID_DISK          0x31

/* ------------------------------------------------------------------ */
/* Command bytes (shared subset used by all Lynx device modules)       */
/* ------------------------------------------------------------------ */

#define NETCMD_ACK          0x06
#define NETCMD_NAK          0x15

/* Network device commands */
#define NETCMD_OPEN         0x4F  /* O */
#define NETCMD_CLOSE        0x43  /* C */
#define NETCMD_READ         0x52  /* R */
#define NETCMD_WRITE        0x57  /* W */
#define NETCMD_STATUS       0x53  /* S */
#define NETCMD_QUERY        0x51  /* Q */
#define NETCMD_CONTROL      0x41  /* A */
#define NETCMD_PARSE        0x50  /* P */
#define NETCMD_TRANSLATION  0x54  /* T */
#define NETCMD_GET_ERROR    0x45  /* E */
#define NETCMD_CHANNEL_MODE	0xFC
#define NETCMD_JSON_QUERY   0x81
#define NETCMD_JSON_PARSE   0x80

/* ------------------------------------------------------------------ */
/* I/O error codes                                                    */
/* ------------------------------------------------------------------ */

typedef enum {
    FNIO_ERR_NONE,      /**< Success */
    FNIO_ERR_TIMEOUT,   /**< Timed out waiting for data */
    FNIO_ERR_SEND_CHK,  /**< Checksum on sent data was rejected */
    FNIO_ERR_RECV_CHK,  /**< Checksum on received data was bad */
    FNIO_ERR_GENERAL    /**< Unspecified error */
} FNIO_ERROR_T;

extern uint8_t _fn_error;  /**< Most-recent I/O error code */


/**
 * @brief send a command buffer and wait for ACK (no data back).
 * Retries up to LYNX_FN_RETRIES times.
 * @param dev  Device ID
 * @param buf  Command buffer
 * @param len  Length of command buffer
 * @return 1 on success, 0 on failure
 */
uint8_t _fnio_send_cmd(uint8_t dev, char *buf, uint16_t len);

/**
 * @brief send a command buffer and receive a data response.
 * Retries up to LYNX_FN_RETRIES times.
 * @param dev      Device ID
 * @param buf      Command buffer (input) / data buffer (output) – caller must ensure
 *                 the buffer is large enough for the expected response.
 * @param cmd_len  Length of the command portion in buf
 * @param out      Destination buffer for the received data
 * @param out_len  Set to the number of bytes received
 * @return 1 on success, 0 on failure
 */
uint8_t _fnio_send_cmd_recv(uint8_t dev, char *buf, uint16_t cmd_len, char *out, uint16_t *out_len);

/**
 * @brief Initialise the ComLynx serial port.
 * Must be called once before any fnio_* function.
 * @return 0 on success, cc65 ser_* error code on failure.
 */
uint8_t fnio_init(void);

/**
 * @brief Tear down the ComLynx serial port.
 * @return 0 on success, cc65 ser_* error code on failure.
 */
uint8_t fnio_done(void);

/* ------------------------------------------------------------------ */
/* Low Level I/O Functions                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Send a framed packet to a FujiNet device and wait for ACK/NAK.
 * @param dev    Device ID (e.g. FUJI_DEVICEID_FUJINET, FUJI_DEVICEID_NETWORK+unit)
 * @param buf    Payload buffer (first byte is typically the command)
 * @param len    Length of payload
 * @return 1 on ACK, 0 on NAK or timeout (check fnio_error())
 */
uint8_t fnio_send_buf(uint8_t dev, char *buf, uint16_t len);

/**
 * @brief Receive a framed data response from FujiNet.
 * @param buf    Destination buffer (must be at least LYNX_FN_LEN_MAX bytes)
 * @param len    Set to the number of bytes actually received
 * @return 1 on success, 0 on error (check fnio_error())
 */
uint8_t fnio_recv_buf(char *buf, uint16_t *len);

/**
 * @brief Receive a bare ACK/NAK from FujiNet (for commands with no data response).
 * @return 1 on ACK, 0 on NAK or timeout
 */
uint8_t fnio_recv_ack(void);

/**
 * @brief Flush any pending bytes from the receive buffer.
 */
void fnio_flush_recv(void);

/**
 * @brief Return the most recent I/O error code.
 * @return FNIO_ERROR_T value
 */
uint8_t fnio_error(void);

/* ------------------------------------------------------------------ */
/* Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief Compute XOR checksum of a buffer
 */
uint8_t _checksum(char *b, uint16_t len);

/**
 * @brief Block-wait for one byte from the serial port with timeout.
 * On success the byte is in the global _r variable.
 * @return 1 on success, 0 on timeout
 */
uint8_t _serial_get_loop(char *b);


#endif /* LYNXFNIO_H */
