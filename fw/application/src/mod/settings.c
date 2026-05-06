#include "settings.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "vfs.h"
#include "vfs_meta.h"

#include "tag_helper.h"
#include "ble_amiibolink.h"

#define SETTINGS_FILE_NAME "/settings.bin"
#define SETTINGS_FILE_MAGIC 0x31544753u // "STG1"
#define SETTINGS_FILE_VERSION 1

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t payload_size;
} settings_file_header_t;

#define SETTINGS_SERIALIZED_MAX_SIZE (sizeof(settings_file_header_t) + sizeof(settings_data_t))

#ifdef OLED_SCREEN
// Though OLED doesn't necessarily imply rechargeable battery, it's usually the case.
#define DEFAULT_BAT_MODE 1
#else // !OLED_SCREEN
#define DEFAULT_BAT_MODE 0
#endif // OLED_SCREEN

const settings_data_t def_settings_data = {.backlight = 0,
                                           .auto_gen_amiibo = 0,
                                           .auto_gen_amiibolink = 0,
                                           .sleep_timeout_sec = 30,
                                           .skip_driver_select = 0,
                                           .bat_mode = DEFAULT_BAT_MODE,
                                           .amiibo_link_ver = BLE_AMIIBOLINK_VER_V1,
                                           .language = LANGUAGE_EN_US,
                                           .hibernate_enabled = false,
                                           .show_mem_usage = false,
                                           .lcd_backlight = 0,
                                           .oled_contrast = 80,
                                           .anim_enabled = false,
                                           .amiidb_data_slot_num = 20,
                                           .qrcode_enabled = true,
                                           .chameleon_default_slot_index = INVALID_SLOT_INDEX,
                                            .app_enable_bits = 0xFFFF,
                                            .amiidb_sort_column = 0,
                                           .chameleon_slot_num = 8,
                                           .amiibolink_mode = 0, // 0 = not set, use default (manual)
                                           .display_flip = false};

settings_data_t m_settings_data = {0};

#define BOOL_VALIDATE(expr, default_val)                                                                               \
    if ((expr) != 0 && (expr) != 1) {                                                                                  \
        (expr) = (default_val);                                                                                        \
    }
#define INT8_VALIDATE(expr, min, max, default_val)                                                                     \
    if ((expr) < (min) || (expr) > (max)) {                                                                            \
        (expr) = (default_val);                                                                                        \
    }

static void validate_settings() {
    if (m_settings_data.sleep_timeout_sec > 180) {
        m_settings_data.sleep_timeout_sec = 30;
    }

    if (m_settings_data.amiibo_link_ver != BLE_AMIIBOLINK_VER_V1 &&
        m_settings_data.amiibo_link_ver != BLE_AMIIBOLINK_VER_V2 && m_settings_data.amiibo_link_ver != BLE_AMILOOP) {
        m_settings_data.amiibo_link_ver = BLE_AMIIBOLINK_VER_V1;
    }

    BOOL_VALIDATE(m_settings_data.skip_driver_select, 0);
    BOOL_VALIDATE(m_settings_data.show_mem_usage, 0);
    BOOL_VALIDATE(m_settings_data.hibernate_enabled, 0);
    BOOL_VALIDATE(m_settings_data.bat_mode, 0);
    BOOL_VALIDATE(m_settings_data.skip_driver_select, 0);
    BOOL_VALIDATE(m_settings_data.auto_gen_amiibo, 0);
    BOOL_VALIDATE(m_settings_data.auto_gen_amiibolink, 0);
    BOOL_VALIDATE(m_settings_data.backlight, 0);
    INT8_VALIDATE(m_settings_data.lcd_backlight, 0, 100, 0);
    INT8_VALIDATE(m_settings_data.oled_contrast, 0, 100, 80);
    BOOL_VALIDATE(m_settings_data.anim_enabled, 0);
    BOOL_VALIDATE(m_settings_data.qrcode_enabled, 0);
    BOOL_VALIDATE(m_settings_data.display_flip, 0);
    INT8_VALIDATE(m_settings_data.language, 0, LANGUAGE_COUNT - 1, LANGUAGE_EN_US);
    INT8_VALIDATE(m_settings_data.amiidb_data_slot_num, 1, 100, 20);
    INT8_VALIDATE(m_settings_data.chameleon_slot_num, 8, 50, 8);
    INT8_VALIDATE(m_settings_data.chameleon_default_slot_index, 0, m_settings_data.chameleon_slot_num, INVALID_SLOT_INDEX);
    
    // Validate amiibolink_mode: 0 = not set, 1-4 are valid modes
    if (m_settings_data.amiibolink_mode != 0 && 
        m_settings_data.amiibolink_mode != BLE_AMIIBOLINK_MODE_RANDOM &&
        m_settings_data.amiibolink_mode != BLE_AMIIBOLINK_MODE_CYCLE &&
        m_settings_data.amiibolink_mode != BLE_AMIIBOLINK_MODE_NTAG &&
        m_settings_data.amiibolink_mode != BLE_AMIIBOLINK_MODE_RANDOM_AUTO_GEN) {
        m_settings_data.amiibolink_mode = 0; // Reset to "not set" if invalid
    }
}

static size_t settings_serialize_current(uint8_t *buffer, size_t buffer_size) {
    if (buffer_size < SETTINGS_SERIALIZED_MAX_SIZE) {
        return 0;
    }

    settings_file_header_t header = {.magic = SETTINGS_FILE_MAGIC,
                                     .version = SETTINGS_FILE_VERSION,
                                     .payload_size = sizeof(settings_data_t)};
    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer + sizeof(header), &m_settings_data, sizeof(settings_data_t));
    return SETTINGS_SERIALIZED_MAX_SIZE;
}

static bool settings_try_deserialize_versioned(const uint8_t *buffer, size_t buffer_size, settings_data_t *out) {
    if (buffer_size < sizeof(settings_file_header_t)) {
        return false;
    }

    const settings_file_header_t *header = (const settings_file_header_t *)buffer;
    if (header->magic != SETTINGS_FILE_MAGIC || header->version != SETTINGS_FILE_VERSION) {
        return false;
    }

    if (header->payload_size != sizeof(settings_data_t)) {
        return false;
    }

    size_t expected_size = sizeof(settings_file_header_t) + header->payload_size;
    if (buffer_size < expected_size) {
        return false;
    }

    memcpy(out, buffer + sizeof(settings_file_header_t), sizeof(settings_data_t));
    return true;
}

static bool settings_deserialize_legacy(const uint8_t *buffer, size_t buffer_size, settings_data_t *out) {
    if (buffer_size == 0 || buffer_size > sizeof(settings_data_t)) {
        return false;
    }
    memcpy(out, buffer, buffer_size);
    return true;
}

int32_t settings_init() {
    memcpy(&m_settings_data, &def_settings_data, sizeof(settings_data_t));
    vfs_driver_t *p_driver = vfs_get_default_driver();
    if (p_driver == NULL) {
        return NRF_ERROR_NOT_SUPPORTED;
    }
    int32_t err = 0;
    if (!p_driver->mounted()) {
        err = p_driver->mount();
    }

    if (!p_driver->mounted()) {
        return NRF_ERROR_INVALID_STATE;
    }

    vfs_obj_t settings_obj = {0};
    err = p_driver->stat_file(SETTINGS_FILE_NAME, &settings_obj);
    if (err == VFS_ERR_NOOBJ) {
        validate_settings();
        NRF_LOG_INFO("settings not found, using defaults");
        return NRF_SUCCESS;
    }
    if (err < 0) {
        return NRF_ERROR_INVALID_STATE;
    }

    if (settings_obj.size == 0) {
        validate_settings();
        NRF_LOG_INFO("settings empty, using defaults");
        return NRF_SUCCESS;
    }

    uint8_t settings_raw[SETTINGS_SERIALIZED_MAX_SIZE];
    size_t read_size_target = settings_obj.size;
    if (read_size_target > sizeof(settings_raw)) {
        read_size_target = sizeof(settings_raw);
    }

    err = p_driver->read_file_data(SETTINGS_FILE_NAME, settings_raw, read_size_target);
    if (err < 0) {
        return NRF_ERROR_INVALID_STATE;
    }

    size_t read_size = (size_t)err;
    bool loaded = settings_try_deserialize_versioned(settings_raw, read_size, &m_settings_data);
    if (!loaded) {
        loaded = settings_deserialize_legacy(settings_raw, read_size, &m_settings_data);
        if (loaded) {
            NRF_LOG_INFO("settings migrated from legacy layout");
        }
    }

    if (!loaded) {
        NRF_LOG_WARNING("settings invalid, using defaults");
    }

    validate_settings();

    NRF_LOG_INFO("settings loaded!");
    return NRF_SUCCESS;
}

int32_t settings_save() {
    vfs_driver_t *p_driver = vfs_get_default_driver();
    int32_t err;

    if (p_driver == NULL) {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    uint8_t old_settings_raw[SETTINGS_SERIALIZED_MAX_SIZE];
    size_t old_settings_size = 0;
    vfs_obj_t settings_obj = {0};
    err = p_driver->stat_file(SETTINGS_FILE_NAME, &settings_obj);
    bool not_found = false;
    if (err == VFS_ERR_NOOBJ) {
        not_found = true;
    } else if (err < 0) {
        return NRF_ERROR_INVALID_STATE;
    } else if (settings_obj.size > 0) {
        old_settings_size = settings_obj.size;
        if (old_settings_size > sizeof(old_settings_raw)) {
            old_settings_size = sizeof(old_settings_raw);
        }

        err = p_driver->read_file_data(SETTINGS_FILE_NAME, old_settings_raw, old_settings_size);
        if (err < 0) {
            return NRF_ERROR_INVALID_STATE;
        }
        old_settings_size = (size_t)err;
    }

    uint8_t new_settings_raw[SETTINGS_SERIALIZED_MAX_SIZE];
    size_t new_settings_size = settings_serialize_current(new_settings_raw, sizeof(new_settings_raw));
    if (new_settings_size == 0) {
        return NRF_ERROR_INVALID_STATE;
    }

    bool needs_write = not_found || old_settings_size != new_settings_size ||
                       memcmp(new_settings_raw, old_settings_raw, new_settings_size) != 0;

    if (needs_write) {
        err = p_driver->write_file_data(SETTINGS_FILE_NAME, new_settings_raw, new_settings_size);
        if (err < 0 || (size_t)err != new_settings_size) {
            return NRF_ERROR_INVALID_STATE;
        }

        if (not_found) {
            vfs_meta_t meta;
            memset(&meta, 0, sizeof(meta));
            meta.has_flags = true;
            meta.flags = VFS_OBJ_FLAG_HIDDEN;

            uint8_t meta_data[VFS_MAX_META_LEN];
            vfs_meta_encode(meta_data, sizeof(meta_data), &meta);
            err = p_driver->update_file_meta(SETTINGS_FILE_NAME, &meta_data, sizeof(meta_data));
            NRF_LOG_INFO("Settings file meta updated!: %d", err);
        }

        NRF_LOG_INFO("settings saved!");
    }

    return NRF_SUCCESS;
}

settings_data_t *settings_get_data() { return &m_settings_data; }

int32_t settings_reset() {
    memcpy(&m_settings_data, &def_settings_data, sizeof(settings_data_t));
    vfs_driver_t *p_driver = vfs_get_default_driver();
    return p_driver->remove_file(SETTINGS_FILE_NAME);
}
