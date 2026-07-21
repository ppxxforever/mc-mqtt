#include "led.h"
#include "driver/gpio.h"

#define LED_GPIO_NUM GPIO_NUM_10

void led_init(void)
{
    gpio_config_t led_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pin_bit_mask = (1ULL << LED_GPIO_NUM),
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&led_cfg);
    gpio_set_level(LED_GPIO_NUM,1);
}

void led_on(void){
    gpio_set_level(LED_GPIO_NUM,0);
}

void led_off(void){
    gpio_set_level(LED_GPIO_NUM,1);
}