/*
 * nvs.h
 *
 * NVS (Non-Volatile Storage)功能头文件
 * 类似于ESP32-IDF中的NVS功能
 *
 * Copyright (c) 2023 Winner Micro Electronic Design Co., Ltd.
 * All rights reserved.
 */

#ifndef __NVS_H__
#define __NVS_H__

#include "wm_include.h"
#include "wm_internal_flash.h"

// NVS功能宏定义
#define NVS_FLASH_BASE_ADDR 0x1F0303
#define NVS_KEY_SIZE 32
#define NVS_VALUE_SIZE 256
#define NVS_MAX_ENTRIES 100  // 设置NVS条数为100条

// NVS条目结构体
typedef struct {
    char key[NVS_KEY_SIZE];
    char value[NVS_VALUE_SIZE];
} nvs_entry_t;

// NVS功能函数声明
/**
 * @brief 初始化NVS
 * @return 无
 */
void nvs_init(void);

/**
 * @brief 存储键值对
 * @param key 键名
 * @param value 键值
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_set(const char *key, const char *value);

/**
 * @brief 读取键值对
 * @param key 键名
 * @param value 存储读取值的缓冲区
 * @param value_len 缓冲区大小
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get(const char *key, char *value, size_t value_len);

/**
 * @brief 删除键值对
 * @param key 键名
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_delete(const char *key);

/**
 * @brief 清除所有NVS数据
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_erase_all(void);

#endif /* __NVS_H__ */