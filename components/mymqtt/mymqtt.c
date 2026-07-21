#include <stdio.h>
#include "mymqtt.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "led.h"
#define TAG                     "mqtt"
#define MQTT_BROKER_ADDRESS     "mqtt://192.168.31.73"
#define MQTT_BROKER_PORT        1883
#define MQTT_USERNAME           "test1"
#define MQTT_PASSWORD           "123456"
#define MQTT_ClientID           "mymqtt"
#define MQTT_TOPIC_PUBLISH      "/test1/publish"
#define MQTT_TOPIC_RECIVE       "/mc/light"

bool mqtt_connect_status = false;
esp_mqtt_client_handle_t mqtt_handle = NULL;
void mqtt_event_handle(void* event_handler_arg,esp_event_base_t event_base,int32_t event_id,void* event_data){
    
    esp_mqtt_event_handle_t event = event_data;
    //esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:
            mqtt_connect_status = true;
            ESP_LOGI(TAG,"MQTT CONNECTED");
            ESP_LOGI("main","start publish"); 
            char a='a';
            esp_mqtt_client_publish(mqtt_handle,MQTT_TOPIC_PUBLISH,&a,1,0,0);
            esp_mqtt_client_subscribe_single(mqtt_handle, MQTT_TOPIC_RECIVE,1);
            break;
        case MQTT_EVENT_DISCONNECTED:
            mqtt_connect_status = false;
            ESP_LOGI(TAG,"MQTT DISCONNECTED");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG,"MQTT SUCCESS PUBLISH MSG ID:%d",event->msg_id);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG,"MQTT SUCCESS SUBSCRIBED MSG ID:%d",event->msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        case MQTT_EVENT_DATA:
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);       //收到Pub消息直接打印出来
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            int b = strcmp(event->data,"DATA=ON");
            ESP_LOGI("MC-MQTT","%d",b);
            if (b==43)
            {
                led_on();
            }else led_off();
            
            break;        
        default:
            break;
    }
    
}


void mqtt_start(void){
    esp_mqtt_client_config_t mqtt_cfg={
        .broker.address.uri=MQTT_BROKER_ADDRESS,
        .broker.address.port=MQTT_BROKER_PORT,
        .credentials.client_id=MQTT_ClientID,
        .credentials.username=MQTT_USERNAME,
        .credentials.authentication.password=MQTT_PASSWORD,
    };
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle,ESP_EVENT_ANY_ID,mqtt_event_handle,mqtt_handle);
    esp_mqtt_client_start(mqtt_handle);
}