#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "driver/gpio.h"



#include "esp_log.h"
#include "mqtt_client.h"

#define LED_GPIO 2

static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void process_mqtt_data(esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "MQTT Received Data");

    char topic[50] = {0};   
    char payload[50] = {0}; 

    snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
    snprintf(payload, sizeof(payload), "%.*s", event->data_len, event->data);

    ESP_LOGI(TAG, "Received topic: %s, data: %s", topic, payload);

    if (strncmp(topic, "/topic/66025/led", sizeof(topic)) == 0) {
        if (strncmp(payload, "ON", sizeof(payload)) == 0) {
            gpio_set_level(LED_GPIO, 0);  // ✅ เปิดไฟ
            ESP_LOGI(TAG, "✅ LED turned ON");
        } 
        else if (strncmp(payload, "OFF", sizeof(payload)) == 0) {
            gpio_set_level(LED_GPIO, 1);  // ✅ ปิดไฟ
            ESP_LOGI(TAG, "✅ LED turned OFF");
        } 
        else {
            ESP_LOGW(TAG, "⚠️ Unknown command received: %s", payload);
        }
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT_EVENT_CONNECTED\n");
            break;
        case MQTT_EVENT_DATA:
            printf("MQTT_EVENT_DATA\n");
            break;
        default:
            printf("Unhandled MQTT event: %ld\n", event_id);
            break;
    }
    
}

void mqtt_app_start(void) { 
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 1);  // ✅ ตั้งค่าให้ LED ดับตอนเริ่มต้น

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.1.100:1883",  // ✅ ใส่ IP ของ Broker จริง
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}
