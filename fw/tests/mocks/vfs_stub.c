#include "vfs.h"

/* Minimal stub: all drives disabled, no drivers */

bool vfs_drive_enabled(vfs_drive_t drive) {
    (void)drive;
    return false;
}

vfs_driver_t *vfs_get_driver(vfs_drive_t drive) {
    (void)drive;
    return NULL;
}
