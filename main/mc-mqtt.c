#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "wifi.h"
#include "mymqtt.h"
#include "mqtt_client.h"
#include "led.h"

//配网完成标志
extern bool smtcfg_done;
//mqtt连接标志
extern bool mqtt_connect_status;
extern esp_mqtt_client_handle_t mqtt_handle;

void app_main(void)
{
    mywifi_init();
    led_init();
        // 等待配网完成
    ESP_LOGI("main", "Waiting for WiFi configuration...");
    int wait_count = 0;
    while (!smtcfg_done && wait_count < 300) {  // 等待最多30秒
        vTaskDelay(pdMS_TO_TICKS(100));
        wait_count++;
    }
    if (smtcfg_done==true)
    {    
        ESP_LOGI("main","START MQTT");
        mqtt_start();
    }else ESP_LOGI("main","False MQTT");
    
}