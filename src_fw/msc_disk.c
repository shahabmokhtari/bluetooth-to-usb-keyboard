#include <string.h>

#include "tusb.h"
#include "msc_disk_image.h"

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8],
                        uint8_t product_id[16], uint8_t product_rev[4])
{
    (void)lun;
    memcpy(vendor_id, "PICO    ", 8);
    memcpy(product_id, "PAIRING UI      ", 16);
    memcpy(product_rev, "1.0 ", 4);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void)lun;
    return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void)lun;
    *block_count = MSC_DISK_BLOCK_COUNT;
    *block_size = MSC_DISK_BLOCK_SIZE;
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    (void)lun;
    return false;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition,
                           bool start, bool load_eject)
{
    (void)lun;
    (void)power_condition;
    (void)start;
    (void)load_eject;
    return true;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                          void *buffer, uint32_t buffer_size)
{
    (void)lun;
    uint32_t byte_offset = lba * MSC_DISK_BLOCK_SIZE + offset;
    if (byte_offset > sizeof(msc_disk_image) ||
        buffer_size > sizeof(msc_disk_image) - byte_offset) {
        return -1;
    }

    memcpy(buffer, msc_disk_image + byte_offset, buffer_size);
    return (int32_t)buffer_size;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                           uint8_t *buffer, uint32_t buffer_size)
{
    (void)lba;
    (void)offset;
    (void)buffer;
    (void)buffer_size;
    tud_msc_set_sense(lun, SCSI_SENSE_DATA_PROTECT, 0x27, 0x00);
    return -1;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16],
                        void *buffer, uint16_t buffer_size)
{
    (void)scsi_cmd;
    (void)buffer;
    (void)buffer_size;
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
    return -1;
}
