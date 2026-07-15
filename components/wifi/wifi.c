#include "wifi.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_smartconfig.h"
#include "string.h"


#define WIFI_INFO "wifi_inifo"
#define WIFI_SSID "ssid"
#define WIFI_PASSWD "password"


//smartconfig 状态标志
static bool smtcfg_stauts=false;
//配网完成标志
static bool smtcfg_done=false;
//缓存一份ssid
static char ssid_value[33] = {0};
//缓存一份password
static char password_value[65] = {0};

void smartconfig_start(void);

static esp_err_t nvs_saveSSID(char * ssid){
    nvs_handle_t nvs_handle;
    esp_err_t res;
    ESP_ERROR_CHECK(nvs_open(WIFI_INFO,NVS_READWRITE,&nvs_handle));
    res = nvs_set_str(nvs_handle,WIFI_SSID,ssid);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return res;
}

static esp_err_t nvs_savePasswd(char * passwd){
    nvs_handle_t nvs_handle;
    esp_err_t res;
    ESP_ERROR_CHECK(nvs_open(WIFI_INFO,NVS_READWRITE,&nvs_handle));
    res = nvs_set_str(nvs_handle,WIFI_PASSWD,passwd);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return res;
}


static size_t nvs_readSSID (char * ssid,uint16_t maxlen){
    nvs_handle_t nvs_handle;
    esp_err_t res;
    size_t len=0;
    ESP_ERROR_CHECK(nvs_open(WIFI_INFO,NVS_READWRITE,&nvs_handle));
    res = nvs_get_str(nvs_handle,WIFI_SSID,NULL,&len);
    if (res == ESP_OK && len <= maxlen)
    {
        nvs_get_str(nvs_handle,WIFI_SSID,ssid,&len);
    }
    else len=0;
    nvs_close(nvs_handle);
    return len;
}

static size_t nvs_readPasswd (char * passwd,uint16_t maxlen){
    nvs_handle_t nvs_handle;
    esp_err_t res;
    size_t len=0;
    ESP_ERROR_CHECK(nvs_open(WIFI_INFO,NVS_READWRITE,&nvs_handle));
    res = nvs_get_str(nvs_handle,WIFI_PASSWD,NULL,&len);
    if (res == ESP_OK && len <= maxlen)
    {
        nvs_get_str(nvs_handle,WIFI_PASSWD,passwd,&len);
    }
    else len=0;
    nvs_close(nvs_handle);
    return len;
}

void wifi_event_handler(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data){
    if(event_base == WIFI_EVENT){
        switch(event_id){
            case WIFI_EVENT_STA_START:
                           
            if (ssid_value[0] != 0) {
                ESP_LOGI("SmartConfig", "Using saved credentials, connecting...");
                esp_wifi_connect();
            } else {
                ESP_LOGI("SmartConfig", "No saved credentials, starting SmartConfig...");
                smartconfig_start();
            }  
            break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI("WIFI_EVENT", "Wi-Fi connected");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI("WIFI_EVENT", "Wi-Fi disconnected, trying to reconnect...");
                if (!smtcfg_stauts)
                {
                    esp_wifi_connect();
                }
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
    else if (event_base == SC_EVENT){
        switch (event_id)
        {
            case SC_EVENT_SCAN_DONE:
                ESP_LOGI("SmartConfig","SCAN DONE");
                break;
            case SC_EVENT_FOUND_CHANNEL:
                ESP_LOGI("SmartConfig","FOUND CHANNEL");
                break;
            case SC_EVENT_GOT_SSID_PSWD:
                ESP_LOGI("SmartConfig","GOT SSID AND PASSWORD");
                wifi_config_t wifi_cft;
                uint8_t ssid[33]={0};
                uint8_t passwd[65]={0};
                smartconfig_event_got_ssid_pswd_t * evt=(smartconfig_event_got_ssid_pswd_t *)event_data;
                memcpy(ssid,evt->ssid,sizeof(evt->ssid));
                memcpy(passwd,evt->password,sizeof(evt->password));
                ESP_LOGI("SmartConfig","SSID:%s",ssid);
                ESP_LOGI("SmartConfig","PASSWORD:%s",passwd);
                bzero(&wifi_cft,sizeof(wifi_cft));
                memcpy(wifi_cft.sta.ssid,evt->ssid,sizeof(evt->ssid));
                memcpy(wifi_cft.sta.password,evt->password,sizeof(evt->password));
                wifi_cft.sta.bssid_set = evt->bssid_set;
                if (wifi_cft.sta.bssid_set == true)
                {
                    memcpy(wifi_cft.sta.bssid,evt->bssid,sizeof(evt->bssid));
                }
                snprintf(ssid_value,33,"%s",(char*)ssid);
                snprintf(password_value,65,"%s",(char*)passwd);
                nvs_saveSSID(ssid_value);
                nvs_savePasswd(password_value);
                ESP_LOGI("NVS","SAVE SSID AND PASSWORD");
                ESP_ERROR_CHECK(esp_wifi_disconnect());
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifi_cft));
                ESP_ERROR_CHECK(esp_wifi_connect());
                break;
            case SC_EVENT_SEND_ACK_DONE:
                ESP_LOGI("SmartConfig","SEND ACK DONE");
                smtcfg_done=true;
                smtcfg_stauts=false;
                break;
            default:
                break;
        }
    }    
}


static void smartconfig_task(void * parm){
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (1)
    {

        if (smtcfg_done)
        {
            ESP_LOGI("SmartConfig","SmartConfig over");
            esp_smartconfig_stop();
            smtcfg_done=false;
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void smartconfig_start(void){
    if (smtcfg_stauts==false)
    {
        smtcfg_stauts=true;
        xTaskCreatePinnedToCore(smartconfig_task, "smartconfig_task", 4096, NULL, 5, NULL, 0);
    }
    
}

void mywifi_init(void){
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
    esp_event_handler_register(SC_EVENT,ESP_EVENT_ANY_ID,wifi_event_handler,NULL);

    nvs_readSSID(ssid_value,32);
    nvs_readPasswd(password_value,64);
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
    if (ssid_value[0]!=0)
    {   

        memcpy(wifi_config.sta.ssid, ssid_value, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, password_value, sizeof(wifi_config.sta.password));
        ESP_LOGI("NVS", "Wi-Fi SSID: %s, Password: %s", wifi_config.sta.ssid, wifi_config.sta.password); //打印Wi-Fi SSID和密码配置
    }
    else {
        ESP_LOGI("WIFI_INIT", "No Wi-Fi SSID and Password found in NVS, starting SmartConfig..."); //打印未找到Wi-Fi SSID和密码配置
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); //设置Wi-Fi模式
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start()); //启动Wi-Fi模块
}
