#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "driver/gpio.h"

#include "Lab11.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define LED_GPIO 2

static const char *TAG = "MQTT";

static void log_error_if_nonzero(const char *message, int error_code)
{
  if(error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

void process_mqtt_data(esp_mqtt_event_handle_t event) {
  ESP_LOGI(TAG, "MQTT Received Data");

  char topic[50];   
  char payload[50]; 

  strncpy(topic, event->topic, event->topic_len);
  topic[event->topic_len] = '\0';

  strncpy(payload, event->data, event->data_len);
  payload[event->data_len] = '\0';

  ESP_LOGI(TAG, "Received topic: %s, data: %s", topic, payload);

  // ตรวจสอบ topic และ payload
  if (strcmp(topic, "/topic/66025/led") == 0) {
      if (strcmp(payload, "ON") == 0) {
          control_led(1); // ✅ เรียกฟังก์ชันที่แยกออกไป
      } else if (strcmp(payload, "OFF") == 0) {
          control_led(0); // ✅ เรียกฟังก์ชันที่แยกออกไป
      } else {
          ESP_LOGW(TAG, "Unknown payload received: %s", payload);
      }
  }
}



static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "✅ MQTT Connected to Broker");

            // ✅ Subscribe ไปที่ Topic ควบคุม LED
            int msg_id = esp_mqtt_client_subscribe(client, "/topic/66025/led", 1);
            if (msg_id >= 0) {
                ESP_LOGI(TAG, "✅ Successfully subscribed to /topic/66025/led, msg_id=%d", msg_id);
            } else {
                ESP_LOGE(TAG, "❌ Failed to subscribe to /topic/66025/led");
            }
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "✅ Received MQTT Data");
            process_mqtt_data(event);  // ✅ ต้องเรียกเพื่อให้ทำงาน!
            break;

        default:
            ESP_LOGI(TAG, "Other MQTT event received: %d", event->event_id);
            break;
    }
}
void process_mqtt_data(esp_mqtt_event_handle_t event) {
    ESP_LOGI(TAG, "✅ MQTT Received Data");

    char topic[50];
    char payload[50];

    snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
    snprintf(payload, sizeof(payload), "%.*s", event->data_len, event->data);

    ESP_LOGI(TAG, "✅ Received topic: %s, data: %s", topic, payload);

    if (strcmp(topic, "/topic/66025/led") == 0) {
        if (strcmp(payload, "ON") == 0) {
            gpio_set_level(LED_GPIO, 1);  // ✅ เปิดไฟ LED
            ESP_LOGI(TAG, "✅ LED turned ON");
        } 
        else if (strcmp(payload, "OFF") == 0) {
            gpio_set_level(LED_GPIO, 0);  // ✅ ปิดไฟ LED
            ESP_LOGI(TAG, "✅ LED turned OFF");
        } 
        else {
            ESP_LOGW(TAG, "⚠️ Unknown command received: %s", payload);
        }
    }
}
