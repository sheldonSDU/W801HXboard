/*
 * wm_wifi_manager.c
 *
 * WiFi管理功能实现文件
 *
 * Copyright (c) 2023 Winner Micro Electronic Design Co., Ltd.
 * All rights reserved.
 */

#include "wm_wifi_manager.h"
#include <string.h>
#include "lwip/ip_addr.h"  // 为了确保ip_addr相关定义可用
#include "wm_netif.h"     // 为了确保tls_netif_get_ethif函数声明可用

// 函数声明  
// Note: 如果函数在调用前没有声明，编译器会默认它是非静态的！

// 全局变量
static wifi_state_t g_wifi_state = WIFI_STATE_DISCONNECTED;
static wifi_config_t g_wifi_config;
static bool g_wifi_initialized = false;


/**
 * @brief 打印IP地址
 * @param ip IP地址结构体指针
 * @return 无
 */
 void print_ipaddr_user(ip_addr_t *ip)
{
    if (!IP_IS_V6(ip))  // 如果不是IPv6，则是IPv4
    {
        printf("IPv4地址: %d.%d.%d.%d\n",
               ip4_addr1(ip_2_ip4(ip)),
               ip4_addr2(ip_2_ip4(ip)),
               ip4_addr3(ip_2_ip4(ip)),
               ip4_addr4(ip_2_ip4(ip)));
    }
#if TLS_CONFIG_IPV6
    else  // IPv6地址
    {
        printf("IPv6地址: %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
               HTONS(((ip6_addr_t *)ip)->addr[0]),
               HTONS(((ip6_addr_t *)ip)->addr[1]),
               HTONS(((ip6_addr_t *)ip)->addr[2]),
               HTONS(((ip6_addr_t *)ip)->addr[3]),
               HTONS(((ip6_addr_t *)ip)->addr[4]),
               HTONS(((ip6_addr_t *)ip)->addr[5]),
               HTONS(((ip6_addr_t *)ip)->addr[6]),
               HTONS(((ip6_addr_t *)ip)->addr[7]));
    }
#endif
}



// 网络状态变化回调函数
static void con_net_status_changed_event(u8 status )
{
    struct tls_cmd_ip_params_t ip_info;
    switch(status)
    {
    case NETIF_WIFI_JOIN_SUCCESS:
        printf("NETIF_WIFI_JOIN_SUCCESS: WiFi连接成功\n");
        g_wifi_state = WIFI_STATE_CONNECTED;
        break;
    case NETIF_WIFI_JOIN_FAILED:
        printf("NETIF_WIFI_JOIN_FAILED: WiFi连接失败\n");
        g_wifi_state = WIFI_STATE_ERROR;
        break;
    case NETIF_WIFI_DISCONNECTED:
        printf("NETIF_WIFI_DISCONNECTED: WiFi已断开\n");
        g_wifi_state = WIFI_STATE_DISCONNECTED;
        break;
    case NETIF_IP_NET_UP:
        printf("NETIF_IP_NET_UP: 网络已启动\n");
        // 直接从网络接口获取并打印IP信息
        struct tls_ethif *tmpethif = tls_netif_get_ethif();
        if (tmpethif != NULL)
        {
            print_ipaddr_user(&tmpethif->ip_addr);
#if TLS_CONFIG_IPV6
            print_ipaddr_user(&tmpethif->ip6_addr[0]);
            print_ipaddr_user(&tmpethif->ip6_addr[1]);
            print_ipaddr_user(&tmpethif->ip6_addr[2]);
#endif
        }
        break;
    default:
        printf("未知网络状态: %d\n", status);
        break;
    }
}

/**
 * @brief 初始化WiFi配置
 * @return 无
 */
static void init_wifi_config(void)
{
    u8 wireless_protocol = 0;
    struct tls_param_ip *ip_param = NULL;
    struct tls_param_quick_connect quick_connect;

    tls_wifi_disconnect();
    tls_wifi_softap_destroy();
    tls_wifi_set_oneshot_flag(0);

    // 清除快速连接信息
    quick_connect.quick_connect_en = FALSE;
    quick_connect.chanId = 255;
    tls_param_set(TLS_PARAM_ID_QUICK_CONNECT, &quick_connect, TRUE);

    // 设置为基础架构模式
    tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, TRUE);
    if (TLS_PARAM_IEEE80211_INFRA != wireless_protocol)
    {
        wireless_protocol = TLS_PARAM_IEEE80211_INFRA;
        tls_param_set(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, FALSE);
    }

    // 配置DHCP
    ip_param = tls_mem_alloc(sizeof(struct tls_param_ip));
    if (ip_param)
    {
        tls_param_get(TLS_PARAM_ID_IP, ip_param, FALSE);
        ip_param->dhcp_enable = g_wifi_config.use_dhcp;
        tls_param_set(TLS_PARAM_ID_IP, ip_param, FALSE);
        tls_mem_free(ip_param);
    }

    // 配置连接参数
    tls_wifi_cfg_connect_pci(g_wifi_config.pci_en);
    tls_wifi_cfg_connect_timeout(g_wifi_config.connect_timeout);
    tls_wifi_cfg_connect_scan_mode(g_wifi_config.scan_mode);

    // 注册网络状态事件回调
    tls_netif_add_status_event(con_net_status_changed_event);
}

/**
 * @brief 初始化WiFi管理器
 * @return 无
 */
void wifi_manager_init(void)
{
    if (g_wifi_initialized)
    {
        return;
    }

    // 设置默认配置
    strncpy(g_wifi_config.ssid, WIFI_SSID_DEFAULT, sizeof(g_wifi_config.ssid) - 1);
    strncpy(g_wifi_config.password, WIFI_PASSWORD_DEFAULT, sizeof(g_wifi_config.password) - 1);
    g_wifi_config.use_dhcp = TRUE;
    g_wifi_config.scan_mode = SCAN_MODE;
    g_wifi_config.pci_en = PCI_EN;
    g_wifi_config.connect_timeout = CONNECT_TIMEOUT;

    // 初始化WiFi配置
    init_wifi_config();

    g_wifi_initialized = true;
    g_wifi_state = WIFI_STATE_DISCONNECTED;
    printf("WiFi管理器已初始化\n");
}

/**
 * @brief 连接到WiFi网络
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return 成功返回WM_SUCCESS，失败返回错误码
 */
int wifi_manager_connect(const char *ssid, const char *password)
{
    int ret;

    if (!g_wifi_initialized)
    {
        return WM_FAILED;
    }

    if (ssid != NULL)
    {
        strncpy(g_wifi_config.ssid, ssid, sizeof(g_wifi_config.ssid) - 1);
    }

    if (password != NULL)
    {
        strncpy(g_wifi_config.password, password, sizeof(g_wifi_config.password) - 1);
    }

    printf("\n开始连接WiFi: %s\n", g_wifi_config.ssid);
    printf("密码: %s\n", g_wifi_config.password);

    g_wifi_state = WIFI_STATE_CONNECTING;

    // 执行连接
    ret = tls_wifi_connect((u8 *)g_wifi_config.ssid, strlen(g_wifi_config.ssid), 
                          (u8 *)g_wifi_config.password, strlen(g_wifi_config.password));
    if (WM_SUCCESS == ret)
    {
        printf("连接请求已发送，请等待连接结果...\n");
    }
    else
    {
        printf("连接请求发送失败\n");
        g_wifi_state = WIFI_STATE_ERROR;
    }

    return ret;
}

/**
 * @brief 断开WiFi连接
 * @return 无
 */
void wifi_manager_disconnect(void)
{
    if (!g_wifi_initialized)
    {
        return;
    }

    tls_wifi_disconnect();
    g_wifi_state = WIFI_STATE_DISCONNECTED;
    printf("已断开WiFi连接\n");
}

/**
 * @brief 获取当前WiFi连接状态
 * @return 返回wifi_state_t枚举值
 */
wifi_state_t wifi_manager_get_state(void)
{
    return g_wifi_state;
}

/**
 * @brief 从NVS加载WiFi配置
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_load_config(void)
{
    if (!g_wifi_initialized)
    {
        return WM_FAILED;
    }

    // 从NVS加载配置
    if (nvs_get("wifi_ssid", g_wifi_config.ssid, sizeof(g_wifi_config.ssid)) != WM_SUCCESS)
    {
        printf("从NVS加载SSID失败，使用默认值\n");
        strncpy(g_wifi_config.ssid, WIFI_SSID_DEFAULT, sizeof(g_wifi_config.ssid) - 1);
    }

    if (nvs_get("wifi_password", g_wifi_config.password, sizeof(g_wifi_config.password)) != WM_SUCCESS)
    {
        printf("从NVS加载密码失败，使用默认值\n");
        strncpy(g_wifi_config.password, WIFI_PASSWORD_DEFAULT, sizeof(g_wifi_config.password) - 1);
    }

    printf("从NVS加载WiFi配置成功: SSID=%s\n", g_wifi_config.ssid);
    return WM_SUCCESS;
}

/**
 * @brief 保存WiFi配置到NVS
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_save_config(const char *ssid, const char *password)
{
    if (!g_wifi_initialized)
    {
        return WM_FAILED;
    }

    if (ssid != NULL)
    {
        strncpy(g_wifi_config.ssid, ssid, sizeof(g_wifi_config.ssid) - 1);
    }

    if (password != NULL)
    {
        strncpy(g_wifi_config.password, password, sizeof(g_wifi_config.password) - 1);
    }

    // 保存到NVS
    if (nvs_set("wifi_ssid", g_wifi_config.ssid) != WM_SUCCESS)
    {
        return WM_FAILED;
    }

    if (nvs_set("wifi_password", g_wifi_config.password) != WM_SUCCESS)
    {
        return WM_FAILED;
    }

    printf("WiFi配置已保存到NVS: SSID=%s\n", g_wifi_config.ssid);
    return WM_SUCCESS;
}

/**
 * @brief 获取IP信息
 * @param ip_info 存储IP信息的结构体指针
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_get_ip_info(struct tls_cmd_ip_params_t *ip_info)
{
    struct tls_ethif *ethif;

    if (!g_wifi_initialized || ip_info == NULL)
    {
        return WM_FAILED;
    }

    if (g_wifi_state != WIFI_STATE_CONNECTED)
    {
        printf("WiFi未连接，无法获取IP信息\n");
        return WM_FAILED;
    }

    // 尝试直接从网络接口获取IP信息
    ethif = tls_netif_get_ethif();
    if (ethif == NULL)
    {
        printf("获取网络接口失败\n");
        //  fallback到原有的方式
        return tls_cmd_get_ip_info(ip_info);
    }

    // 填充IP信息
    MEMCPY(ip_info->ip_addr, (char *)ip_2_ip4(&ethif->ip_addr), 4);
    MEMCPY(ip_info->netmask, (char *)ip_2_ip4(&ethif->netmask), 4);
    MEMCPY(ip_info->gateway, (char *)ip_2_ip4(&ethif->gw), 4);
    MEMCPY(ip_info->dns, (char *)ip_2_ip4(&ethif->dns1), 4);

    // 设置IP获取方式
    struct tls_param_ip ip_param;
    tls_param_get(TLS_PARAM_ID_IP, &ip_param, FALSE);
    ip_info->type = ip_param.dhcp_enable ? 0 : 1;

    return WM_SUCCESS;
}



/**
 * @brief 获取当前连接的RSSI值
 * @param rssi 存储RSSI值的指针
 * @return 成功返回WM_SUCCESS，失败返回WM_FAILED
 */
int wifi_manager_get_rssi(int8_t *rssi)
{
    struct tls_curr_bss_t bss;

    if (!g_wifi_initialized || rssi == NULL)
    {
        return WM_FAILED;
    }

    if (g_wifi_state != WIFI_STATE_CONNECTED)
    {
        printf("WiFi未连接，无法获取RSSI值\n");
        return WM_FAILED;
    }

    tls_wifi_get_current_bss(&bss);
    *rssi = (int8_t)bss.rssi;

    return WM_SUCCESS;
}