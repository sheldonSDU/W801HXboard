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
/*Refer to 《W800 SDK指南》《》定义了使用者期望存储自定义的参数或者运行日志。
* QFLASH布局用户可用的空间为240KB，
* 地址范围为：0x81E0000-0x81FBFFF
*/
#define NVS_FLASH_BASE_ADDR 0x81E0000  //NVS的存储区域
#define NVS_KEY_SIZE 32
#define NVS_VALUE_SIZE 256
#define NVS_MAX_ENTRIES 100  // 设置NVS条数为100条

// 自动清理阈值定义 (占总容量的百分比)
#define NVS_AUTO_CLEAN_THRESHOLD 80  // 当使用空间达到80%时触发自动清理

// 数据类型枚举
typedef enum {
    NVS_TYPE_STRING,  // 字符串类型
    NVS_TYPE_BINARY   // 二进制类型
} nvs_data_type_t;

// NVS条目结构体
typedef struct {
    char key[NVS_KEY_SIZE];
    nvs_data_type_t type;  // 数据类型
    uint16_t length;       // 数据长度
    char value[NVS_VALUE_SIZE];  // 数据值
} nvs_entry_t;

// 外部变量声明
extern uint8_t nvs_used_entries;

// NVS功能函数声明
/**
 * @brief 初始化NVS
 * @return 无
 */
void nvs_init(void);

/**
 * @brief 存储字符串键值对
 * @param key 键名
 * @param value 字符串值
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_set(const char *key, const char *value);

/**
 * @brief 存储二进制键值对
 * @param key 键名
 * @param data 二进制数据
 * @param length 数据长度
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_set_binary(const char *key, const void *data, size_t length);

/**
 * @brief 读取键值对
 * @param key 键名
 * @param value 存储读取值的缓冲区
 * @param value_len 缓冲区大小
 * @param type 数据类型（可以为NULL，表示自动检测）
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get(const char *key, void *value, size_t value_len, nvs_data_type_t *type);

/**
 * @brief 读取字符串键值对（兼容旧接口）
 * @param key 键名
 * @param value 存储读取值的缓冲区
 * @param value_len 缓冲区大小
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get_string(const char *key, char *value, size_t value_len);

/**
 * @brief 读取二进制键值对
 * @param key 键名
 * @param data 存储读取数据的缓冲区
 * @param data_len 缓冲区大小
 * @param actual_len 实际读取的数据长度（可以为NULL）
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_get_binary(const char *key, void *data, size_t data_len, size_t *actual_len);

/**
 * @brief 删除键值对
 * @param key 键名
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_delete(const char *key);

/**
 * @brief 自动清理NVS空间
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_auto_clean(void);

/**
 * @brief 清除所有NVS数据
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int nvs_erase_all(void);

#endif /* __NVS_H__ */