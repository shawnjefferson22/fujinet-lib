#include <cmoc.h>
#include <coco.h>
#include "fujinet-fuji.h"
#include <dw.h>
#include <fujinet-fuji-coco.h>

bool fuji_mount_disk_image(uint8_t ds, uint8_t mode)
{
    struct _mdi
    {
        uint8_t opcode;
        uint8_t cmd;
        uint8_t ds;
        uint8_t mode;
    } mdi;

    mdi.opcode = OP_FUJI;
    mdi.cmd = FUJICMD_MOUNT_IMAGE;
    mdi.ds = ds;
    mdi.mode = mode;
    
    bus_ready();
    dwwrite((uint8_t *)&mdi, sizeof(mdi));

#ifdef DRAGON
    ds++;
    uint8_t link = ds+4;
    uint8_t* pUnit = (uint8_t*)0x00f8; 
    *pUnit = ds;
    asm
    {
        pshs x,y,b
        ldb :link

        jsr [0xe6aa]
        puls b,y,x
    }
#endif
    
    return !fuji_get_error();
}
