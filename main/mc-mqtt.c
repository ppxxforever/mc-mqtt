#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "wifi.h"



void wifi_init_task(void *pvParameters)
{   
    ESP_LOGI("WIFI_INIT", "Starting Wi-Fi initialization task");
    wifi_init(); // 初始化Wi-Fi
    vTaskDelete(NULL); // 删除任务
}



void app_main(void)
{
    // 创建一个任务来初始化Wi-Fi
    xTaskCreatePinnedToCore(wifi_init_task, "wifi_init_task", 4096, NULL, 5, NULL, 0);
}