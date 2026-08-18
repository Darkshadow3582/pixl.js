#ifndef VFS_MOCK_H_
#define VFS_MOCK_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define VFS_MAX_META_LEN 128

typedef enum {
    VFS_DRIVE_INT = 0,
    VFS_DRIVE_EXT = 1,
} vfs_drive_t;

typedef struct {
    int32_t (*read_file_data)(const char *filename, uint8_t *data, size_t size);
    int32_t (*write_file_data)(const char *filename, uint8_t *data, size_t size);
} vfs_driver_t;

bool vfs_drive_enabled(vfs_drive_t drive);
vfs_driver_t *vfs_get_driver(vfs_drive_t drive);

#endif
