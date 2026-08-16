#include "cache.h"

#include <stdint.h>

#include "ntag_emu.h"

#include "nrf_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "vfs.h"
#include "vfs_meta.h"

#include "nrf_pwr_mgmt.h"
#include "settings.h"
#include "crc32.h"

// nRF52 RAM retention is configured per 8 KiB block, split into two 4 KiB sections.
#define NRF52_RAM_BASE_ADDR 0x20000000UL
#define NRF52_RAM_POWER_BLOCK_SIZE 0x2000UL
#define NRF52_RAM_POWER_SECTION_SIZE 0x1000UL
#define NRF52_RAM_POWER_SECTIONS_PER_BLOCK (NRF52_RAM_POWER_BLOCK_SIZE / NRF52_RAM_POWER_SECTION_SIZE)


extern uint8_t __start_noinit;
extern uint8_t __stop_noinit;

static __attribute__((section(".noinit"))) cache_data_t m_cache_data;
static __attribute__((section(".noinit"))) int32_t m_cache_crc32;

static size_t cache_noinit_size() {
    return (size_t)(&__stop_noinit - &__start_noinit);
}

static uint32_t cache_ram_retention_mask(uint32_t section) {
    return POWER_RAM_POWER_S0RETENTION_On << (POWER_RAM_POWER_S0RETENTION_Pos + section);
}

static ret_code_t cache_enable_noinit_retention() {
    uintptr_t start = (uintptr_t)&__start_noinit;
    uintptr_t end = (uintptr_t)&__stop_noinit;

    if (end <= start) {
        return NRF_SUCCESS;
    }

    uintptr_t first_offset = start - NRF52_RAM_BASE_ADDR;
    uintptr_t last_offset = end - NRF52_RAM_BASE_ADDR - 1;
    uint32_t first_section = first_offset / NRF52_RAM_POWER_SECTION_SIZE;
    uint32_t last_section = last_offset / NRF52_RAM_POWER_SECTION_SIZE;

    for (uint32_t section = first_section; section <= last_section; section++) {
        uint8_t ram_index = section / NRF52_RAM_POWER_SECTIONS_PER_BLOCK;
        uint32_t ram_section = section % NRF52_RAM_POWER_SECTIONS_PER_BLOCK;
        uint32_t ram_power = 0;
        uint32_t retention_mask = cache_ram_retention_mask(ram_section);

        ret_code_t err_code = sd_power_ram_power_get(ram_index, &ram_power);
        if (err_code != NRF_SUCCESS) {
            return err_code;
        }

        NRF_LOG_INFO("RAM%d power: 0x%X, retaining S%d", ram_index, ram_power, ram_section);
        err_code = sd_power_ram_power_set(ram_index, retention_mask);
        if (err_code != NRF_SUCCESS) {
            return err_code;
        }
    }

    return NRF_SUCCESS;
}

bool cache_valid(){
    NRF_LOG_INFO("noinit area: [0x%X, 0x%X], %u bytes",  &__start_noinit, &__stop_noinit,
                 (uint32_t)cache_noinit_size());
    NRF_LOG_INFO("m_cache_data address: 0x%X", &m_cache_data);
    return m_cache_crc32 == crc32_compute((const int8_t *)&m_cache_data, sizeof(cache_data_t), NULL);
}

int32_t cache_clean() {
    NRF_LOG_INFO("Cleaning cache...")
    // 重置一下noinit ram区域
    uint8_t *noinit_addr = &__start_noinit;
    size_t noinit_size = cache_noinit_size();
    memset(noinit_addr, 0x0, noinit_size);
    m_cache_crc32 = crc32_compute(&m_cache_data, sizeof(cache_data_t), NULL);

    NRF_LOG_INFO("Reset noinit ram done.");
    return NRF_SUCCESS;
}

int32_t cache_save() {
    NRF_LOG_INFO("Saving cache...");
    m_cache_crc32 = crc32_compute(&m_cache_data, sizeof(cache_data_t), NULL);
    NRF_LOG_INFO("Cache data: enabled = %d, id = %d", m_cache_data.enabled, m_cache_data.id);

    ret_code_t err_code = cache_enable_noinit_retention();
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    return NRF_SUCCESS;
}

cache_data_t *cache_get_data() {
    NRF_LOG_INFO("Cache data: enabled = %d, id = %d", m_cache_data.enabled, m_cache_data.id);
    return &m_cache_data;
}
