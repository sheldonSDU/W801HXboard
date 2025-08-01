/*
 * nvs.c
 *
 * NVS (Non-Volatile Storage)功能实现文件
 * 类似于ESP32-IDF中的NVS功能
 *
 * Copyright (c) 2023 Winner Micro Electronic Design Co., Ltd.
 * All rights reserved.
 */

#include "nvs.h"
#include <string.h>

// 存储NVS条目的数组
static nvs_entry_t nvs_entries[NVS_MAX_ENTRIES] = {0};
// 已使用的NVS条目数量
static uint8_t nvs_used_entries = 0;
// NVS是否已初始化
static bool nvs_initialized = false;

/**
 * @brief 从Flash加载NVS数据
 * @return 无
 */
static void nvs_load_from_flash(void)
{
    // 从Flash读取NVS数据
    tls_fls_read(NVS_FLASH_BASE_ADDR, (u8*)nvs_entries, sizeof(nvs_entries));

    // 计算已使用的条目数量
    nvs_used_entries = 0;
    for (uint8_t i = 0; i < NVS_MAX_ENTRIES; i++) {
        if (nvs_entries[i].key[0] != 0) {
            nvs_used_entries++;
        } else {
            break;  // 遇到空条目，后面的都是未使用的
        }
    }
}

/**
 * @brief 将NVS数据保存到Flash
 * @return 无
 */
static void nvs_save_to_flash(void)
{
    // 写入整个NVS数组到Flash
    tls_fls_write(NVS_FLASH_BASE_ADDR, (u8*)nvs_entries, sizeof(nvs_entries));
}

/**
 * @brief 初始化NVS
 * @return 无
 */
void nvs_init(void)
{
    if (nvs_initialized) {
        return;
    }

    // 初始化Flash驱动
    tls_fls_init();

    // 从Flash加载NVS数据
    nvs_load_from_flash();

    nvs_initialized = true;
    printf("NVS initialized, used entries: %d\n", nvs_used_entries);
}

/**
 * @brief 存储键值对
 * @param key 键名
 * @param value 键值
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_set(const char *key, const char *value)
{
    if (!nvs_initialized) {
        return WM_FAILED;
    }

    if (key == NULL || value == NULL) {
        return WM_FAILED;
    }

    // 查找是否已存在该键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 更新已有键的值
            strncpy(nvs_entries[i].value, value, NVS_VALUE_SIZE - 1);
            nvs_save_to_flash();
            return WM_SUCCESS;
        }
    }

    // 检查是否还有空间
    if (nvs_used_entries >= NVS_MAX_ENTRIES) {
        printf("NVS is full\n");
        return WM_FAILED;
    }

    // 添加新的键值对
    strncpy(nvs_entries[nvs_used_entries].key, key, NVS_KEY_SIZE - 1);
    strncpy(nvs_entries[nvs_used_entries].value, value, NVS_VALUE_SIZE - 1);
    nvs_used_entries++;

    // 保存到Flash
    nvs_save_to_flash();

    return WM_SUCCESS;
}

/**
 * @brief 读取键值对
 * @param key 键名
 * @param value 存储读取值的缓冲区
 * @param value_len 缓冲区大小
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get(const char *key, char *value, size_t value_len)
{
    if (!nvs_initialized || key == NULL || value == NULL) {
        return WM_FAILED;
    }

    // 查找键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 找到键，复制值
            strncpy(value, nvs_entries[i].value, value_len - 1);
            value[value_len - 1] = '\0';  // 确保字符串结束
            return WM_SUCCESS;
        }
    }

    // 未找到键
    printf("Key not found: %s\n", key);
    return WM_FAILED;
}

/**
 * @brief 删除键值对
 * @param key 键名
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_delete(const char *key)
{
    if (!nvs_initialized || key == NULL) {
        return WM_FAILED;
    }

    // 查找键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 找到键，删除它
            // 将后面的条目向前移动
            for (uint8_t j = i; j < nvs_used_entries - 1; j++) {
                memcpy(&nvs_entries[j], &nvs_entries[j + 1], sizeof(nvs_entry_t));
            }

            // 清空最后一个条目
            memset(&nvs_entries[nvs_used_entries - 1], 0, sizeof(nvs_entry_t));
            nvs_used_entries--;

            // 保存到Flash
            nvs_save_to_flash();

            return WM_SUCCESS;
        }
    }

    // 未找到键
    printf("Key not found: %s\n", key);
    return WM_FAILED;
}

/**
 * @brief 清除所有NVS数据
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_erase_all(void)
{
    if (!nvs_initialized) {
        return WM_FAILED;
    }

    // 清空所有条目
    memset(nvs_entries, 0, sizeof(nvs_entries));
    nvs_used_entries = 0;

    // 保存到Flash
    nvs_save_to_flash();

    printf("NVS erased all data\n");
    return WM_SUCCESS;
}