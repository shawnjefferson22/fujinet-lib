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

#include "../../fujinet-fuji.h"
#include "lynxfnio.h"

/* ------------------------------------------------------------------ */
/* Module-private state                                                 */
/* ------------------------------------------------------------------ */

/* Command / scratch buffer shared by all fuji functions */
static char _fn_cmd[512];
static uint16_t _fn_recv_len;

/* Appkey context set via fuji_set_appkey_details() */
static uint16_t _appkey_creator_id = 0;
static uint8_t  _appkey_app_id     = 0;
static uint8_t  _appkey_keysize    = 0; /* 0 = DEFAULT (64 bytes) */

/* ------------------------------------------------------------------ */
/* Public globals required by fujinet-fuji.h                            */
/* ------------------------------------------------------------------ */

FNStatus _fuji_status;
uint8_t  fn_default_timeout = 0x0F;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * Send a Fuji-device command from _fn_cmd and wait for ACK.
 */
static uint8_t _fuji_cmd(uint16_t len)
{
    return _fnio_send_cmd(FUJI_DEVICEID_FUJINET, _fn_cmd, len);
}

/*
 * Send a Fuji-device command from _fn_cmd and receive a data response
 * into `out`.  Returns 1 on success; `out_len` set to bytes received.
 */
static uint8_t _fuji_cmd_recv(uint16_t len, char *out, uint16_t *out_len)
{
    return _fnio_send_cmd_recv(FUJI_DEVICEID_FUJINET, _fn_cmd, len,
                               out, out_len);
}

/* ------------------------------------------------------------------ */
/* fuji_error                                                           */
/* ------------------------------------------------------------------ */

bool fuji_error(void)
{
    return (fnio_error() != FNIO_ERR_NONE);
}

/* ------------------------------------------------------------------ */
/* WiFi / adapter                                                       */
/* ------------------------------------------------------------------ */

bool fuji_get_wifi_status(uint8_t *status)
{
    char s;
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_WIFISTATUS;
    if (!_fuji_cmd_recv(1, &s, &len)) return false;

    *status = (uint8_t)s;
    return true;
}

bool fuji_get_wifi_enabled(void)
{
    char s;
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_WIFI_ENABLED;
    if (!_fuji_cmd_recv(1, &s, &len)) return false;

    return (s != 0);
}

bool fuji_scan_for_networks(uint8_t *count)
{
    char n;
    uint16_t len;

    _fn_cmd[0] = FUJICMD_SCAN_NETWORKS;
    if (!_fuji_cmd_recv(1, &n, &len)) return false;

    *count = (uint8_t)n;
    return true;
}

bool fuji_get_scan_result(uint8_t n, SSIDInfo *ssid_info)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_SCAN_RESULT;
    _fn_cmd[1] = (char)n;
    if (!_fuji_cmd_recv(2, (char *)ssid_info, &len)) return false;

    return true;
}

bool fuji_get_ssid(NetConfig *net_config)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_SSID;
    if (!_fuji_cmd_recv(1, (char *)net_config, &len)) return false;

    return true;
}

bool fuji_set_ssid(NetConfig *nc)
{
    _fn_cmd[0] = FUJICMD_SET_SSID;
    _fn_cmd[1] = 1; /* save flag */
    memcpy(&_fn_cmd[2], nc, sizeof(NetConfig));
    return (_fuji_cmd((uint16_t)(sizeof(NetConfig) + 2)) != 0);
}

bool fuji_get_adapter_config(AdapterConfig *ac)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_ADAPTERCONFIG;
    if (!_fuji_cmd_recv(1, (char *)ac, &len)) return false;

    return true;
}

bool fuji_get_adapter_config_extended(AdapterConfigExtended *ac)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_ADAPTERCONFIG_EXTENDED;
    if (!_fuji_cmd_recv(1, (char *)ac, &len)) return false;

    return true;
}

/* ------------------------------------------------------------------ */
/* Host / device slots                                                  */
/* ------------------------------------------------------------------ */

bool fuji_get_host_slots(HostSlot *h, size_t size)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_READ_HOST_SLOTS;
    if (!_fuji_cmd_recv(1, (char *)h, &len)) return false;

    /* sanity: check the received length matches what caller expects */
    if (len != (uint16_t)(size * sizeof(HostSlot))) return false;

    return true;
}

bool fuji_put_host_slots(HostSlot *h, size_t size)
{
    uint16_t payload = (uint16_t)(size * sizeof(HostSlot));

    _fn_cmd[0] = FUJICMD_WRITE_HOST_SLOTS;
    memcpy(&_fn_cmd[1], h, payload);
    return (_fuji_cmd((uint16_t)(payload + 1)) != 0);
}

bool fuji_mount_host_slot(uint8_t hs)
{
    _fn_cmd[0] = FUJICMD_MOUNT_HOST;
    _fn_cmd[1] = (char)hs;
    return (_fuji_cmd(2) != 0);
}

bool fuji_unmount_host_slot(uint8_t hs)
{
    _fn_cmd[0] = FUJICMD_UNMOUNT_HOST;
    _fn_cmd[1] = (char)hs;
    return (_fuji_cmd(2) != 0);
}

bool fuji_get_host_prefix(uint8_t hs, char *prefix)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_HOST_PREFIX;
    _fn_cmd[1] = (char)hs;
    if (!_fuji_cmd_recv(2, prefix, &len)) return false;

    return true;
}

bool fuji_set_host_prefix(uint8_t hs, char *prefix)
{
    uint16_t slen = (uint16_t)strlen(prefix);

    _fn_cmd[0] = FUJICMD_SET_HOST_PREFIX;
    _fn_cmd[1] = (char)hs;
    memcpy(&_fn_cmd[2], prefix, slen + 1); /* include NUL */
    return (_fuji_cmd((uint16_t)(slen + 3)) != 0);
}

bool fuji_get_device_slots(DeviceSlot *d, size_t size)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_READ_DEVICE_SLOTS;
    if (!_fuji_cmd_recv(1, (char *)d, &len)) return false;

    if (len != (uint16_t)(size * sizeof(DeviceSlot))) return false;

    return true;
}

bool fuji_put_device_slots(DeviceSlot *d, size_t size)
{
    uint16_t payload = (uint16_t)(size * sizeof(DeviceSlot));

    _fn_cmd[0] = FUJICMD_WRITE_DEVICE_SLOTS;
    memcpy(&_fn_cmd[1], d, payload);
    return (_fuji_cmd((uint16_t)(payload + 1)) != 0);
}

bool fuji_get_device_filename(uint8_t ds, char *buffer)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_DEVICE_FULLPATH;
    _fn_cmd[1] = (char)ds;
    if (!_fuji_cmd_recv(2, buffer, &len)) return false;

    return true;
}

bool fuji_set_device_filename(uint8_t mode, uint8_t hs, uint8_t ds,
                               char *buffer)
{
    uint16_t slen = (uint16_t)strlen(buffer);

    _fn_cmd[0] = FUJICMD_SET_DEVICE_FULLPATH;
    _fn_cmd[1] = (char)ds;
    _fn_cmd[2] = (char)hs;
    _fn_cmd[3] = (char)mode;
    memcpy(&_fn_cmd[4], buffer, slen + 1);
    return (_fuji_cmd((uint16_t)(slen + 5)) != 0);
}

bool fuji_get_device_enabled_status(uint8_t d)
{
    /* Not properly implemented in firmware; always return true */
    (void)d;
    return true;
}

bool fuji_mount_disk_image(uint8_t ds, uint8_t mode)
{
    _fn_cmd[0] = FUJICMD_MOUNT_IMAGE;
    _fn_cmd[1] = (char)ds;
    _fn_cmd[2] = (char)mode;
    return (_fuji_cmd(3) != 0);
}

bool fuji_unmount_disk_image(uint8_t ds)
{
    _fn_cmd[0] = FUJICMD_UNMOUNT_IMAGE;
    _fn_cmd[1] = (char)ds;
    return (_fuji_cmd(2) != 0);
}

bool fuji_mount_all(void)
{
    _fn_cmd[0] = FUJICMD_MOUNT_ALL;
    return (_fuji_cmd(1) != 0);
}

/* ------------------------------------------------------------------ */
/* Directory operations                                                 */
/* ------------------------------------------------------------------ */

bool fuji_open_directory(uint8_t hs, char *path_filter)
{
    /* path_filter is a 256-byte buffer with path + \0 + filter */
    _fn_cmd[0] = FUJICMD_OPEN_DIRECTORY;
    _fn_cmd[1] = (char)hs;
    memcpy(&_fn_cmd[2], path_filter, 256);
    return (_fuji_cmd(258) != 0);
}

bool fuji_open_directory2(uint8_t hs, char *path, char *filter)
{
    uint16_t plen = (uint16_t)strlen(path);

    _fn_cmd[0] = FUJICMD_OPEN_DIRECTORY;
    _fn_cmd[1] = (char)hs;
    memcpy(&_fn_cmd[2], path, plen + 1); /* path + NUL */

    if (filter) {
        uint16_t flen = (uint16_t)strlen(filter);
        memcpy(&_fn_cmd[2 + plen + 1], filter, flen + 1);
        return (_fuji_cmd((uint16_t)(plen + flen + 4)) != 0);
    }

    return (_fuji_cmd((uint16_t)(plen + 3)) != 0);
}

bool fuji_close_directory(void)
{
    _fn_cmd[0] = FUJICMD_CLOSE_DIRECTORY;
    return (_fuji_cmd(1) != 0);
}

bool fuji_read_directory(uint8_t maxlen, uint8_t aux2, char *buffer)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_READ_DIR_ENTRY;
    _fn_cmd[1] = (char)maxlen;
    _fn_cmd[2] = (char)aux2;
    memset(buffer, 0, maxlen);
    if (!_fuji_cmd_recv(3, buffer, &len)) return false;

    return true;
}

bool fuji_read_directory_block(uint8_t ram_pages, uint8_t group_size,
                                void *buffer)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_READ_DIR_ENTRY;
    _fn_cmd[1] = (char)ram_pages;
    _fn_cmd[2] = (char)group_size;
    if (!_fuji_cmd_recv(3, (char *)buffer, &len)) return false;

    return true;
}

bool fuji_get_directory_position(uint16_t *pos)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GET_DIRECTORY_POSITION;
    if (!_fuji_cmd_recv(1, (char *)pos, &len)) return false;

    return true;
}

bool fuji_set_directory_position(uint16_t pos)
{
    _fn_cmd[0] = FUJICMD_SET_DIRECTORY_POSITION;
    _fn_cmd[1] = (char)(pos & 0xFF);
    _fn_cmd[2] = (char)(pos >> 8);
    return (_fuji_cmd(3) != 0);
}

/* ------------------------------------------------------------------ */
/* Device management                                                    */
/* ------------------------------------------------------------------ */

bool fuji_enable_device(uint8_t d)
{
    _fn_cmd[0] = FUJICMD_ENABLE_DEVICE;
    _fn_cmd[1] = (char)d;
    return (_fuji_cmd(2) != 0);
}

bool fuji_disable_device(uint8_t d)
{
    _fn_cmd[0] = FUJICMD_DISABLE_DEVICE;
    _fn_cmd[1] = (char)d;
    return (_fuji_cmd(2) != 0);
}

bool fuji_reset(void)
{
    _fn_cmd[0] = FUJICMD_RESET;
    return (_fuji_cmd(1) != 0);
}

bool fuji_set_boot_config(uint8_t toggle)
{
    _fn_cmd[0] = FUJICMD_CONFIG_BOOT;
    _fn_cmd[1] = (char)toggle;
    return (_fuji_cmd(2) != 0);
}

bool fuji_set_boot_mode(uint8_t mode)
{
    _fn_cmd[0] = FUJICMD_SET_BOOT_MODE;
    _fn_cmd[1] = (char)mode;
    return (_fuji_cmd(2) != 0);
}

bool fuji_copy_file(uint8_t src_slot, uint8_t dst_slot, char *copy_spec)
{
    uint16_t slen = (uint16_t)strlen(copy_spec);

    _fn_cmd[0] = FUJICMD_COPY_FILE;
    _fn_cmd[1] = (char)src_slot;
    _fn_cmd[2] = (char)dst_slot;
    memcpy(&_fn_cmd[3], copy_spec, slen + 1);
    return (_fuji_cmd((uint16_t)(slen + 4)) != 0);
}

bool fuji_create_new(NewDisk *new_disk)
{
    (void)new_disk;
    /* NewDisk not defined for Lynx in fujinet-fuji.h */
    return false;
}

bool fuji_enable_udpstream(uint16_t port, char *host)
{
    (void)port;
    (void)host;
    /* Not implemented for Lynx */
    return false;
}

/* ------------------------------------------------------------------ */
/* Status                                                               */
/* ------------------------------------------------------------------ */

bool fuji_status(FNStatus *status)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_STATUS;
    if (!_fuji_cmd_recv(1, (char *)status, &len)) return false;

    return true;
}

/* ------------------------------------------------------------------ */
/* App Keys                                                             */
/* ------------------------------------------------------------------ */

void fuji_set_appkey_details(uint16_t creator_id, uint8_t app_id,
                              enum AppKeySize keysize)
{
    _appkey_creator_id = creator_id;
    _appkey_app_id     = app_id;
    _appkey_keysize    = (uint8_t)keysize;
}

bool fuji_read_appkey(uint8_t key_id, uint16_t *count, uint8_t *data)
{
    uint16_t len;

    /* OPEN_APPKEY */
    _fn_cmd[0] = FUJICMD_OPEN_APPKEY;
    _fn_cmd[1] = (char)(_appkey_creator_id & 0xFF);
    _fn_cmd[2] = (char)(_appkey_creator_id >> 8);
    _fn_cmd[3] = (char)_appkey_app_id;
    _fn_cmd[4] = (char)_appkey_keysize;
    _fn_cmd[5] = (char)key_id;
    if (!_fuji_cmd(6)) return false;

    /* READ_APPKEY */
    _fn_cmd[0] = FUJICMD_READ_APPKEY;
    if (!_fuji_cmd_recv(1, (char *)data, &len)) return false;

    *count = len;

    /* CLOSE_APPKEY */
    _fn_cmd[0] = FUJICMD_CLOSE_APPKEY;
    _fuji_cmd(1); /* best-effort; ignore result */

    return true;
}

bool fuji_write_appkey(uint8_t key_id, uint16_t count, uint8_t *data)
{
    /* OPEN_APPKEY */
    _fn_cmd[0] = FUJICMD_OPEN_APPKEY;
    _fn_cmd[1] = (char)(_appkey_creator_id & 0xFF);
    _fn_cmd[2] = (char)(_appkey_creator_id >> 8);
    _fn_cmd[3] = (char)_appkey_app_id;
    _fn_cmd[4] = (char)_appkey_keysize;
    _fn_cmd[5] = (char)key_id;
    if (!_fuji_cmd(6)) return false;

    /* WRITE_APPKEY */
    _fn_cmd[0] = FUJICMD_WRITE_APPKEY;
    memcpy(&_fn_cmd[1], data, count);
    if (!_fuji_cmd((uint16_t)(count + 1))) return false;

    /* CLOSE_APPKEY */
    _fn_cmd[0] = FUJICMD_CLOSE_APPKEY;
    _fuji_cmd(1);

    return true;
}

/* ------------------------------------------------------------------ */
/* GUID                                                                 */
/* ------------------------------------------------------------------ */

bool fuji_generate_guid(char *buffer)
{
    uint16_t len;

    _fn_cmd[0] = FUJICMD_GENERATE_GUID;
    if (!_fuji_cmd_recv(1, buffer, &len)) return false;

    return true;
}

/* ------------------------------------------------------------------ */
/* Base64                                                               */
/* ------------------------------------------------------------------ */

bool fuji_base64_encode_input(char *s, uint16_t len)
{
    _fn_cmd[0] = FUJICMD_BASE64_ENCODE_INPUT;
    memcpy(&_fn_cmd[1], s, len);
    return (_fuji_cmd((uint16_t)(len + 1)) != 0);
}

bool fuji_base64_encode_compute(void)
{
    _fn_cmd[0] = FUJICMD_BASE64_ENCODE_COMPUTE;
    return (_fuji_cmd(1) != 0);
}

bool fuji_base64_encode_length(unsigned long *len)
{
    uint16_t rlen;
    uint16_t v = 0;

    _fn_cmd[0] = FUJICMD_BASE64_ENCODE_LENGTH;
    if (!_fuji_cmd_recv(1, (char *)&v, &rlen)) return false;

    *len = v;
    return true;
}

bool fuji_base64_encode_output(char *s, uint16_t len)
{
    uint16_t rlen;

    _fn_cmd[0] = FUJICMD_BASE64_ENCODE_OUTPUT;
    if (!_fuji_cmd_recv(1, s, &rlen)) return false;

    (void)len;
    return true;
}

bool fuji_base64_decode_input(char *s, uint16_t len)
{
    _fn_cmd[0] = FUJICMD_BASE64_DECODE_INPUT;
    memcpy(&_fn_cmd[1], s, len);
    return (_fuji_cmd((uint16_t)(len + 1)) != 0);
}

bool fuji_base64_decode_compute(void)
{
    _fn_cmd[0] = FUJICMD_BASE64_DECODE_COMPUTE;
    return (_fuji_cmd(1) != 0);
}

bool fuji_base64_decode_length(unsigned long *len)
{
    uint16_t rlen;
    uint16_t v = 0;

    _fn_cmd[0] = FUJICMD_BASE64_DECODE_LENGTH;
    if (!_fuji_cmd_recv(1, (char *)&v, &rlen)) return false;

    *len = v;
    return true;
}

bool fuji_base64_decode_output(char *s, uint16_t len)
{
    uint16_t rlen;

    _fn_cmd[0] = FUJICMD_BASE64_DECODE_OUTPUT;
    if (!_fuji_cmd_recv(1, s, &rlen)) return false;

    (void)len;
    return true;
}

/* ------------------------------------------------------------------ */
/* Hashing                                                              */
/* ------------------------------------------------------------------ */

bool fuji_hash_input(char *s, uint16_t len)
{
    _fn_cmd[0] = FUJICMD_HASH_INPUT;
    memcpy(&_fn_cmd[1], s, len);
    return (_fuji_cmd((uint16_t)(len + 1)) != 0);
}

bool fuji_hash_compute(uint8_t type)
{
    _fn_cmd[0] = FUJICMD_HASH_COMPUTE;
    _fn_cmd[1] = (char)type;
    return (_fuji_cmd(2) != 0);
}

bool fuji_hash_compute_no_clear(uint8_t type)
{
    _fn_cmd[0] = FUJICMD_HASH_COMPUTE_NO_CLEAR;
    _fn_cmd[1] = (char)type;
    return (_fuji_cmd(2) != 0);
}

bool fuji_hash_length(uint8_t mode)
{
    _fn_cmd[0] = FUJICMD_HASH_LENGTH;
    _fn_cmd[1] = (char)mode;
    return (_fuji_cmd(2) != 0);
}

bool fuji_hash_output(uint8_t output_type, char *s, uint16_t len)
{
    uint16_t rlen;

    _fn_cmd[0] = FUJICMD_HASH_OUTPUT;
    _fn_cmd[1] = (char)output_type;
    if (!_fuji_cmd_recv(2, s, &rlen)) return false;

    (void)len;
    return true;
}

bool fuji_hash_clear(void)
{
    _fn_cmd[0] = FUJICMD_HASH_CLEAR;
    return (_fuji_cmd(1) != 0);
}

bool fuji_hash_add(uint8_t *data, uint16_t length)
{
    _fn_cmd[0] = FUJICMD_HASH_INPUT;
    memcpy(&_fn_cmd[1], data, length);
    return (_fuji_cmd((uint16_t)(length + 1)) != 0);
}

uint16_t fuji_hash_size(hash_alg_t hash_type, bool as_hex)
{
    static const uint8_t sizes[] = { 16, 20, 32, 64 }; /* MD5,SHA1,SHA256,SHA512 */
    uint16_t sz = sizes[(uint8_t)hash_type];
    return as_hex ? (uint16_t)(sz * 2) : sz;
}

bool fuji_hash_calculate(hash_alg_t hash_type, bool as_hex,
                          bool discard_data, uint8_t *output)
{
    uint8_t type_byte = (uint8_t)hash_type | (as_hex ? 0x80 : 0x00)
                                           | (discard_data ? 0x40 : 0x00);
    uint16_t rlen;

    _fn_cmd[0] = FUJICMD_HASH_COMPUTE;
    _fn_cmd[1] = (char)type_byte;

    if (!_fuji_cmd_recv(2, (char *)output, &rlen)) return false;

    return true;
}

bool fuji_hash_data(hash_alg_t hash_type, uint8_t *input, uint16_t length,
                    bool as_hex, uint8_t *output)
{
    if (!fuji_hash_clear())                   return false;
    if (!fuji_hash_add(input, length))        return false;
    return fuji_hash_calculate(hash_type, as_hex, true, output);
}
