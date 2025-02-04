#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define STUDENT_ID "12345678"  // แทนที่ด้วยรหัสนิสิตของคุณ
#define MQTT_BROKER "mqtt://test.mosquitto.org"

#define LED_GPIO GPIO_NUM_2  // กำหนดขา GPIO สำหรับ LED
#define BUTTON_GPIO GPIO_NUM_0  // กำหนดขา GPIO สำหรับปุ่มกด

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client;

// ฟังก์ชันอ่านค่าปุ่มและส่งไป MQTT
void button_task(void *pvParameters)
{
    char topic[50];
    snprintf(topic, sizeof(topic), "/topic/%s/button", STUDENT_ID);
    
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY); // ใช้ Pull-up Resistor

    while (1)
    {
        int button_state = gpio_get_level(BUTTON_GPIO);
        const char *message = button_state == 0 ? "ON" : "OFF"; // กดเป็น 0, ปล่อยเป็น 1

        esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG, "Sent: %s to %s", message, topic);

        vTaskDelay(pdMS_TO_TICKS(1000)); // ส่งทุกๆ 1 วินาที
    }
}

// ฟังก์ชันควบคุม LED
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected");
        char topic[50];
        snprintf(topic, sizeof(topic), "/topic/%s/led", STUDENT_ID);
        esp_mqtt_client_subscribe(client, topic, 0); // สมัครรับค่าควบคุม LED
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received MQTT Message: %.*s = %.*s", event->topic_len, event->topic, event->data_len, event->data);

        // เช็คว่าข้อความที่ได้รับเป็น "ON" หรือ "OFF"
        if (strncmp(event->data, "ON", event->data_len) == 0)
        {
            gpio_set_level(LED_GPIO, 1); // เปิดไฟ LED
        }
        else if (strncmp(event->data, "OFF", event->data_len) == 0)
        {
            gpio_set_level(LED_GPIO, 0); // ปิดไฟ LED
        }
        break;

    default:
        break;
    }
}

// ฟังก์ชันเริ่มต้น MQTT
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// ฟังก์ชันหลัก
void app_main(void)
{
    // กำหนด GPIO
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0); // เริ่มต้นปิดไฟ LED

    // เชื่อมต่อ Wi-Fi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    wifi_connect();
    
    // เริ่ม MQTT
    mqtt_app_start();

    // สร้าง Task สำหรับส่งค่าปุ่มกด
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
}
