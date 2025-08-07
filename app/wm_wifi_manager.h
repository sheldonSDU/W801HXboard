/*
 * wm_wifi_manager.h
 *
 * WiFi管理功能头文件
 *
 * Copyright (c) 2023 Winner Micro Electronic Design Co., Ltd.
 * All rights reserved.
 */

#ifndef __WM_WIFI_MANAGER_H__
#define __WM_WIFI_MANAGER_H__

#include "wm_include.h"
#include "wm_wifi.h"
#include "wm_wifi_oneshot.h"
#include "wm_param.h"
#include "wm_cmdp.h"
#include "nvs.h"

// WiFi配置参数
#define WIFI_SSID_DEFAULT "MMSysLab"
#define WIFI_PASSWORD_DEFAULT "123459876"
#define CONNECT_TIMEOUT 10000  // 连接超时时间(ms)
#define SCAN_MODE 0  // 扫描模式: 0-常规扫描, 1-快速扫描
#define PCI_EN 0  // 是否允许连接开放/WEP网络: 0-不允许, 1-允许

// WiFi连接状态枚举
typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_ERROR
} wifi_state_t;

// WiFi配置结构体
typedef struct {
    char ssid[32];
    char password[64];
    bool use_dhcp;
    u8 scan_mode;
    u8 pci_en;
    u32 connect_timeout;
} wifi_config_t;

// WiFi功能函数声明
/**
 * @brief 初始化WiFi管理器
 * @return 无
 */
void wifi_manager_init(void);

/**
 * @brief 连接到WiFi网络
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return 成功返回WM_SUCCESS，失败返回错误码
 */
int wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief 断开WiFi连接
 * @return 无
 */
void wifi_manager_disconnect(void);

/**
 * @brief 获取当前WiFi连接状态
 * @return 返回wifi_state_t枚举值
 */
wifi_state_t wifi_manager_get_state(void);

/**
 * @brief 从NVS加载WiFi配置
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_load_config(void);

/**
 * @brief 保存WiFi配置到NVS
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_save_config(const char *ssid, const char *password);

/**
 * @brief 获取IP信息
 * @param ip_info 存储IP信息的结构体指针
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_get_ip_info(struct tls_cmd_ip_params_t *ip_info);

/**
 * @brief 获取当前连接的RSSI值
 * @param rssi 存储RSSI值的指针
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_get_rssi(int8_t *rssi);

#endif /* __WM_WIFI_MANAGER_H__ */