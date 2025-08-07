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
uint8_t nvs_used_entries = 0;
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
            
        }
        // 不提前中断，确保计数所有非空条目
    }
    
    // 检查是否接近容量限制
    if (nvs_used_entries >= NVS_MAX_ENTRIES * 0.9) {
        printf("警告: NVS存储空间即将用完，已使用 %d/%d 条目\n", nvs_used_entries, NVS_MAX_ENTRIES);
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
 * @brief 存储字符串键值对
 * @param key 键名
 * @param value 字符串值
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

    size_t value_len = strlen(value);
    if (value_len >= NVS_VALUE_SIZE) {
        printf("Value too long for NVS\n");
        return WM_FAILED;
    }

    // 查找是否已存在该键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 更新已有键的值
            nvs_entries[i].type = NVS_TYPE_STRING;
            nvs_entries[i].length = (uint16_t)value_len;
            strncpy(nvs_entries[i].value, value, NVS_VALUE_SIZE - 1);
            nvs_save_to_flash();
            return WM_SUCCESS;
        }
    }

    // 检查是否需要自动清理
    if (nvs_used_entries >= (NVS_MAX_ENTRIES * NVS_AUTO_CLEAN_THRESHOLD) / 100) {
        nvs_auto_clean();
    }

    // 检查是否需要自动清理
    if (nvs_used_entries >= (NVS_MAX_ENTRIES * NVS_AUTO_CLEAN_THRESHOLD) / 100) {
        nvs_auto_clean();
    }

    // 检查是否还有空间
    if (nvs_used_entries >= NVS_MAX_ENTRIES) {
        printf("NVS已满 (已使用 %d/%d 条目), 尝试释放空间...\n", nvs_used_entries, NVS_MAX_ENTRIES);
        // 简单的LRU策略: 删除最早添加的条目(第一个条目)
        printf("删除最旧的条目: %s\n", nvs_entries[0].key);
        // 将所有条目向前移动一位
        for (uint8_t j = 0; j < nvs_used_entries - 1; j++) {
            memcpy(&nvs_entries[j], &nvs_entries[j + 1], sizeof(nvs_entry_t));
        }
        nvs_used_entries--;
        // 清空最后一个条目
        memset(&nvs_entries[nvs_used_entries], 0, sizeof(nvs_entry_t));
    } else if (nvs_used_entries >= NVS_MAX_ENTRIES * 0.9) {
        printf("警告: NVS存储空间即将用完，已使用 %d/%d 条目\n", nvs_used_entries, NVS_MAX_ENTRIES);
    }

    // 添加新的键值对
    strncpy(nvs_entries[nvs_used_entries].key, key, NVS_KEY_SIZE - 1);
    nvs_entries[nvs_used_entries].type = NVS_TYPE_STRING;
    nvs_entries[nvs_used_entries].length = (uint16_t)value_len;
    strncpy(nvs_entries[nvs_used_entries].value, value, NVS_VALUE_SIZE - 1);
    nvs_used_entries++;

    // 保存到Flash
    nvs_save_to_flash();

    return WM_SUCCESS;
}

/**
 * @brief 存储二进制键值对
 * @param key 键名
 * @param data 二进制数据
 * @param length 数据长度
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_set_binary(const char *key, const void *data, size_t length)
{
    if (!nvs_initialized) {
        return WM_FAILED;
    }

    if (key == NULL || data == NULL || length == 0) {
        return WM_FAILED;
    }

    if (length >= NVS_VALUE_SIZE) {
        printf("Binary data too long for NVS\n");
        return WM_FAILED;
    }

    // 查找是否已存在该键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 更新已有键的值
            nvs_entries[i].type = NVS_TYPE_BINARY;
            nvs_entries[i].length = (uint16_t)length;
            memcpy(nvs_entries[i].value, data, length);
            nvs_save_to_flash();
            return WM_SUCCESS;
        }
    }

    // 检查是否还有空间
    if (nvs_used_entries >= NVS_MAX_ENTRIES) {
        printf("NVS已满 (已使用 %d/%d 条目), 尝试释放空间...\n", nvs_used_entries, NVS_MAX_ENTRIES);
        // 简单的LRU策略: 删除最早添加的条目(第一个条目)
        printf("删除最旧的条目: %s\n", nvs_entries[0].key);
        // 将所有条目向前移动一位
        for (uint8_t j = 0; j < nvs_used_entries - 1; j++) {
            memcpy(&nvs_entries[j], &nvs_entries[j + 1], sizeof(nvs_entry_t));
        }
        nvs_used_entries--;
        // 清空最后一个条目
        memset(&nvs_entries[nvs_used_entries], 0, sizeof(nvs_entry_t));
    } else if (nvs_used_entries >= NVS_MAX_ENTRIES * 0.9) {
        printf("警告: NVS存储空间即将用完，已使用 %d/%d 条目\n", nvs_used_entries, NVS_MAX_ENTRIES);
    }

    // 添加新的键值对
    strncpy(nvs_entries[nvs_used_entries].key, key, NVS_KEY_SIZE - 1);
    nvs_entries[nvs_used_entries].type = NVS_TYPE_BINARY;
    nvs_entries[nvs_used_entries].length = (uint16_t)length;
    memcpy(nvs_entries[nvs_used_entries].value, data, length);
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
 * @param type 数据类型（可以为NULL，表示自动检测）
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get(const char *key, void *value, size_t value_len, nvs_data_type_t *type)
{
    if (!nvs_initialized || key == NULL || value == NULL) {
        return WM_FAILED;
    }

    // 查找键
    for (uint8_t i = 0; i < nvs_used_entries; i++) {
        if (strcmp(nvs_entries[i].key, key) == 0) {
            // 检查缓冲区大小
            if (value_len < nvs_entries[i].length) {
                printf("Buffer too small for NVS value\n");
                return WM_FAILED;
            }

            // 复制值
            memcpy(value, nvs_entries[i].value, nvs_entries[i].length);

            // 如果是字符串类型，确保以null结尾
            if (nvs_entries[i].type == NVS_TYPE_STRING) {
                ((char *)value)[nvs_entries[i].length] = '\0';
            }

            // 如果需要，返回数据类型
            if (type != NULL) {
                *type = nvs_entries[i].type;
            }

            return WM_SUCCESS;
        }
    }

    // 未找到键
    return WM_FAILED;  // 不打印错误信息，仅返回失败
}

/**
 * @brief 读取字符串键值对（兼容旧接口）
 * @param key 键名
 * @param value 存储读取值的缓冲区
 * @param value_len 缓冲区大小
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get_string(const char *key, char *value, size_t value_len)
{
    nvs_data_type_t type;
    int ret = nvs_get(key, value, value_len, &type);
    if (ret == WM_SUCCESS && type != NVS_TYPE_STRING) {
        printf("Requested key is not a string type\n");
        return WM_FAILED;
    }
    return ret;
}

/**
 * @brief 读取二进制键值对
 * @param key 键名
 * @param data 存储读取数据的缓冲区
 * @param data_len 缓冲区大小
 * @param actual_len 实际读取的数据长度（可以为NULL）
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get_binary(const char *key, void *data, size_t data_len, size_t *actual_len)
{
    nvs_data_type_t type;
    int ret = nvs_get(key, data, data_len, &type);
    if (ret == WM_SUCCESS) {
        if (type != NVS_TYPE_BINARY) {
            printf("Requested key is not a binary type\n");
            return WM_FAILED;
        }
        if (actual_len != NULL) {
            // 查找键以获取实际长度
            for (uint8_t i = 0; i < nvs_used_entries; i++) {
                if (strcmp(nvs_entries[i].key, key) == 0) {
                    *actual_len = nvs_entries[i].length;
                    break;
                }
            }
        }
    }
    return ret;
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
    return WM_FAILED;  // 不打印错误信息，仅返回失败
}

/**
 * @brief 自动清理NVS空间
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_auto_clean(void)
{
    if (!nvs_initialized) {
        return WM_FAILED;
    }

    // 计算需要清理的条目数量
    uint8_t threshold_entries = (NVS_MAX_ENTRIES * NVS_AUTO_CLEAN_THRESHOLD) / 100;
    if (nvs_used_entries <= threshold_entries) {
        // 未达到清理阈值
        return WM_SUCCESS;
    }

    // 需要清理的条目数量
    uint8_t entries_to_clean = nvs_used_entries - threshold_entries;
    printf("NVS自动清理: 使用量 %d/%d 条目，超过阈值 %d%%，清理 %d 个最旧条目...\n", 
           nvs_used_entries, NVS_MAX_ENTRIES, NVS_AUTO_CLEAN_THRESHOLD, entries_to_clean);

    // 删除最旧的条目
    for (uint8_t i = 0; i < entries_to_clean; i++) {
        printf("删除最旧的条目: %s\n", nvs_entries[0].key);
        // 将所有条目向前移动一位
        for (uint8_t j = 0; j < nvs_used_entries - 1; j++) {
            memcpy(&nvs_entries[j], &nvs_entries[j + 1], sizeof(nvs_entry_t));
        }
        nvs_used_entries--;
        // 清空最后一个条目
        memset(&nvs_entries[nvs_used_entries], 0, sizeof(nvs_entry_t));
    }

    // 保存到Flash
    nvs_save_to_flash();
    printf("NVS自动清理完成，当前使用: %d/%d 条目\n", nvs_used_entries, NVS_MAX_ENTRIES);

    return WM_SUCCESS;
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