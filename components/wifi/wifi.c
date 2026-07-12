#include "wifi.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "string.h"

#define WIFI_SSID "805" //请替换为实际的Wi-Fi名称
#define WIFI_PASSWORD "pp060409" //请替换为实际的Wi-Fi密码


void wifi_event_handler(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data){
    if(event_base == WIFI_EVENT){
        switch(event_id){
            case WIFI_EVENT_STA_START:
                esp_wifi_connect(); //连接Wi-Fi
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI("WIFI_EVENT", "Wi-Fi connected");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI("WIFI_EVENT", "Wi-Fi disconnected, trying to reconnect...");
                esp_wifi_connect(); //重新连接Wi-Fi
                break;
            default:
                break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI("IP_EVENT", "Got IP");
                break;
            default:
                break;
        }
    }    
}



void wifi_init(void){
    /*基础组件初始化*/
    ESP_ERROR_CHECK(nvs_flash_init()); //初始化非易失性存储
    ESP_ERROR_CHECK(esp_netif_init()); //初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default());//创建默认事件循环

    /*wifi模块初始化*/
    esp_netif_create_default_wifi_sta(); //创建默认Wi-Fi站点接口
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //获取默认Wi-Fi初始化配置
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); //初始化Wi-Fi模块 

    /*注册事件处理回调*/
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); //注册Wi-Fi事件处理回调
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); //注册IP事件处理回调
    /*配置Wi-Fi*/
    wifi_config_t wifi_config = {
        .sta = {
            .channel = 6, //设置Wi-Fi信道为自动选择
            .scan_method = WIFI_FAST_SCAN, //设置扫描方式为快速扫描
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL, //设置连接方式为按信号强度连接
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, //设置认证模式为WPA2-PSK
            .pmf_cfg = {
                .capable = true, //启用PMF功能
                .required = false //不要求PMF功能
            },
        },
    };

    memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid)); //清空SSID
    memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password)); //清空密码
    memcpy(wifi_config.sta.ssid, WIFI_SSID, strlen(WIFI_SSID)+1); //设置Wi-Fi SSID
    memcpy(wifi_config.sta.password, WIFI_PASSWORD, strlen(WIFI_PASSWORD)+1); //

    ESP_LOGI("WIFI_INIT", "Wi-Fi SSID: %s, Password: %s", wifi_config.sta.ssid, wifi_config.sta.password); //打印Wi-Fi SSID和密码
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //设置Wi-Fi模式为STA模式
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); //设置Wi-Fi配置
    ESP_ERROR_CHECK(esp_wifi_start()); //启动Wi-Fi模块
}
