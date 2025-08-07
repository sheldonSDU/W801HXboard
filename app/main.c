/***************************************************************************** 
* 
* File Name : main.c
* 
* Description: main 
*****************************************************************************/ 
#include "wm_include.h"
#include "wm_demo.h"
#include "wm_sockets.h"
#include "nvs.h"
#include "wm_cmdp.h"  // 包含定义tls_cmd_ip_params_t和tls_cmd_get_ip_info的头文件
#include "wm_wifi.h"   // 包含定义tls_wifi_get_state和WiFi状态的头文件
#include "wm_wifi_oneshot.h"  // 参考demo添加oneshot相关头文件
#include "wm_param.h"  // 参考demo添加参数相关头文件
#include "wm_wifi_manager.h"


void UserMain(void)
{
    printf("\n Starting user task \n");

    // 初始化NVS
    nvs_init();

    // 初始化WiFi管理器
    wifi_manager_init();

    // 从NVS加载WiFi配置
    wifi_manager_load_config();

    // 连接WiFi (使用从NVS加载的配置)
    printf("尝试连接WiFi: %s\n", g_wifi_config.ssid);
    wifi_manager_connect(g_wifi_config.ssid, g_wifi_config.password);
    
    // 测试NVS功能
    nvs_set("test_key", "test_value");
    char read_value[256];
    if (nvs_get("test_key", read_value, sizeof(read_value), NULL) == WM_SUCCESS) {
        printf("Read from NVS: %s\n", read_value);
    } else {
        printf("Failed to read from NVS\n");
    }

    // 测试NVS删除功能
    nvs_delete("test_key");
    printf("Deleted key: test_key\n");

#if DEMO_CONSOLE
    CreateDemoTask();  // 调用wm_demo_console.h和.c内的函数
#endif

    // 用户自己的任务循环
    while(1)
    {
        // 定期检查WiFi状态
        static u32 last_check_time = 0;
        u32 current_time = tls_os_get_time();
        if (current_time - last_check_time >= 1000)  // 每秒检查一次
        {
            last_check_time = current_time;

            // 获取并打印WiFi连接状态
            wifi_state_t state = wifi_manager_get_state();
            switch(state)
            {
            case WIFI_STATE_DISCONNECTED:
                printf("WiFi状态: 未连接\n");
                break;
            case WIFI_STATE_CONNECTING:
                printf("WiFi状态: 连接中...\n");
                break;
            case WIFI_STATE_CONNECTED:
                printf("WiFi状态: 已连接\n");
                
                // 打印IP信息
                struct tls_cmd_ip_params_t ip_info;
                int ret = wifi_manager_get_ip_info(&ip_info);
                if (ret == WM_SUCCESS)
                {
                    printf("IP地址: %d.%d.%d.%d\n", ip_info.ip_addr[0], ip_info.ip_addr[1], ip_info.ip_addr[2], ip_info.ip_addr[3]);
                    printf("子网掩码: %d.%d.%d.%d\n", ip_info.netmask[0], ip_info.netmask[1], ip_info.netmask[2], ip_info.netmask[3]);
                    printf("网关: %d.%d.%d.%d\n", ip_info.gateway[0], ip_info.gateway[1], ip_info.gateway[2], ip_info.gateway[3]);
                    printf("DNS服务器: %d.%d.%d.%d\n", ip_info.dns[0], ip_info.dns[1], ip_info.dns[2], ip_info.dns[3]);
                    printf("IP获取方式: %s\n", ip_info.type == 0 ? "DHCP" : "静态IP");
                }
                else
                {
                    printf("获取IP信息失败，错误码: %d\n", ret);
                    // 尝试直接调用底层API获取IP信息
                    if (tls_cmd_get_ip_info(&ip_info) == WM_SUCCESS)
                    {
                        printf("直接调用tls_cmd_get_ip_info获取IP信息成功:\n");
                        printf("IP地址: %d.%d.%d.%d\n", ip_info.ip_addr[0], ip_info.ip_addr[1], ip_info.ip_addr[2], ip_info.ip_addr[3]);
                        printf("子网掩码: %d.%d.%d.%d\n", ip_info.netmask[0], ip_info.netmask[1], ip_info.netmask[2], ip_info.netmask[3]);
                        printf("网关: %d.%d.%d.%d\n", ip_info.gateway[0], ip_info.gateway[1], ip_info.gateway[2], ip_info.gateway[3]);
                    }
                    else
                    {
                        printf("直接调用tls_cmd_get_ip_info也失败\n");
                    }
                }
                break;
            case WIFI_STATE_ERROR:
                printf("WiFi状态: 错误\n");
                break;
            default:
                printf("WiFi状态: 未知\n");
                break;
            }
        }

        // 其他任务代码...
        tls_os_time_delay(10);
    }
}

