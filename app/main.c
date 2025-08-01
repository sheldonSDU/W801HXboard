/***************************************************************************** 
* 
* File Name : main.c
* 
* Description: main 
* 
* Copyright (c) 2014 Winner Micro Electronic Design Co., Ltd. 
* All rights reserved. 
* 
* Author : dave
* 
* Date : 2014-6-14
*****************************************************************************/ 
#include "wm_include.h"
#include "wm_netif.h"
#include "wm_demo.h"
#include "wm_sockets.h"
#include "nvs.h"

// WiFi配置参数
#define WIFI_SSID "lovexiaona"
#define WIFI_PASSWORD "13406903498"

// WiFi连接函数
static void init_wifi_config(void)
{
    u8 wireless_protocol = 0;
    struct tls_param_ip *ip_param = NULL;

    tls_wifi_disconnect();
    tls_wifi_softap_destroy();
    tls_wifi_set_oneshot_flag(0);

    tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, TRUE);
    if (TLS_PARAM_IEEE80211_INFRA != wireless_protocol)
    {
        wireless_protocol = TLS_PARAM_IEEE80211_INFRA;
        tls_param_set(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, FALSE);
    }

    ip_param = tls_mem_alloc(sizeof(struct tls_param_ip));
    if (ip_param)
    {
        tls_param_get(TLS_PARAM_ID_IP, ip_param, FALSE);
        ip_param->dhcp_enable = TRUE;
        tls_param_set(TLS_PARAM_ID_IP, ip_param, FALSE);
        tls_mem_free(ip_param);
    }
}

static int connect_wifi(void)
{
    int ret;
    ret = tls_wifi_connect((u8 *)WIFI_SSID, strlen(WIFI_SSID), (u8 *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));
    if (WM_SUCCESS == ret)
        printf("\nConnecting to WiFi...\n");
    else
        printf("\nWiFi connection failed\n");
    return ret;
}

void UserMain(void)
{
    printf("\n user task \n");

    // 初始化WiFi
    init_wifi_config();
    connect_wifi();

    // 初始化NVS
    nvs_init();

    // 测试NVS功能
    nvs_set("test_key", "test_value");
    char read_value[256];
    if (nvs_get("test_key", read_value, sizeof(read_value)) == WM_SUCCESS) {
        printf("Read from NVS: %s\n", read_value);
    } else {
        printf("Failed to read from NVS\n");
    }

    // 测试NVS删除功能
    nvs_delete("test_key");
    if (nvs_get("test_key", read_value, sizeof(read_value)) == WM_FAILED) {
        printf("Successfully deleted key: test_key\n");
    }

    // 测试多个键值对
    nvs_set("wifi_ssid", WIFI_SSID);
    nvs_set("wifi_password", WIFI_PASSWORD);

    char ssid[256];
    char password[256];
    if (nvs_get("wifi_ssid", ssid, sizeof(ssid)) == WM_SUCCESS &&
        nvs_get("wifi_password", password, sizeof(password)) == WM_SUCCESS) {
        printf("Stored WiFi credentials: SSID=%s, Password=%s\n", ssid, password);
    }

#if DEMO_CONSOLE
    CreateDemoTask();  // 调用wm_demo_console.h和.c内的函数
#endif
//用户自己的task
}

