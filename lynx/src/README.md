# Atari Lynx fujinet-lib Port

This directory (`lynx/`) contains the Atari Lynx implementation of the
[fujinet-lib](https://github.com/FujiNetWIFI/fujinet-lib) common library.

## Architecture

```
lynx/
├── Makefile                  # Platform build config (cc65 lynx target)
├── src/
│   ├── lynxfnio.h            # ComLynx serial I/O API (transport layer)
│   ├── lynxfnio.c            # ComLynx serial I/O implementation
│   ├── fuji/
│   │   └── fuji.c            # Implements fujinet-fuji.h
│   └── network/
│       └── network.c         # Implements fujinet-network.h
```

The port is split into two layers:

### Transport layer (`lynxfnio.c / .h`)

Handles all low-level ComLynx framing. The Lynx FujiNet adapter connects
via the ComLynx serial port at **62500 baud, 8 data bits, odd parity**
using the cc65 `lynx_comlynx_ser` driver.

The protocol is a simple framed packet:

| Field     | Size   | Notes |
|-----------|--------|-------|
| Device ID | 1 byte | e.g. 0x70 for FUJINET, 0x71–0x78 for network units |
| Length    | 2 bytes| Big-endian payload length |
| Payload   | N bytes| Command byte(s) + data |
| Checksum  | 1 byte | XOR of all payload bytes |

Because ComLynx is half-duplex, every transmitted byte is immediately
echoed back; the implementation discards those reflected bytes. After
sending, the device responds with ACK (0x06) or NAK (0x15).

For received data frames the format is `[len_hi][len_lo][payload][checksum]`.

### Device layers

- **`fuji/fuji.c`** — implements all functions declared in `fujinet-fuji.h`
  (WiFi management, host/device slots, directory browsing, appkeys, hashing,
  base64, etc.).

- **`network/network.c`** — implements all functions declared in
  `fujinet-network.h` (open, close, read, write, status, JSON, HTTP
  helpers, filesystem operations).

## Building

Add `lynx` to the `TARGETS` variable in the root `Makefile` (see
`Makefile.patch`), then:

```sh
make TARGETS=lynx release
```

This requires the [cc65](https://cc65.github.io/) toolchain installed and
on your `PATH`.  The Lynx target uses `cl65` as the compiler driver.

## Using in your application

Link `fujinet-lib-lynx-<version>.lib` into your Lynx project and call
`network_init()` once at startup before any network or fuji functions.

```c
#include "fujinet-network.h"
#include "fujinet-fuji.h"

int main(void)
{
    if (network_init() != FN_ERR_OK) {
        /* FujiNet not found or ComLynx error */
        return 1;
    }

    /* Open a TCP connection */
    network_open("N1:TCP://myserver.example:1234/", OPEN_MODE_RW, OPEN_TRANS_NONE);

    /* ... read / write ... */

    network_close("N1:TCP://myserver.example:1234/");
    return 0;
}
```

## Relationship to fujinet-lynx-config

This port extracts and generalises the transport layer from
[fujinet-lynx-config](https://github.com/shawnjefferson22/fujinet-lynx-config)
so that any Lynx application can use the common fujinet-lib API. The
`lynxfnio.c` / `lynxfnio.h` files are a direct adaptation of the files
by the same name in that repository.

## Notes and limitations

- The `NewDisk` structure is **not defined** for the Lynx target in
  `fujinet-fuji.h`, so `fuji_create_new()` always returns `false`.
- `fuji_enable_udpstream()` is not implemented for the Lynx and returns
  `false`.
- The maximum single-transaction payload is 1024 bytes (`LYNX_FN_LEN_MAX`).
  Larger reads are handled by the `network_read()` loop.
- The clock device (`fujinet-clock.h`) implementation is not included in
  this initial port but can be added following the same pattern.
