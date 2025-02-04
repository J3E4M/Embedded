#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_0  // ปุ่มกดที่ GPIO0
#define DEBOUNCE_DELAY 50       // ป้องกันสัญญาณดีบาวน์ 50ms

static const char *TAG = "RTOS_QUEUE";

QueueHandle_t xQueue; // ตัวแปรสำหรับเก็บ Queue

// ** Task 1: Producer (อ่านค่าปุ่มและส่งค่า) **
void producer_task(void *pvParameters)
{
    uint32_t count = 0; // ตัวแปรเก็บจำนวนครั้งที่กดปุ่ม
    int prev_button_state = 1; // ค่าเริ่มต้นของปุ่ม (Pull-up เป็น 1)
    
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    while (1)
    {
        int button_state = gpio_get_level(BUTTON_GPIO); // อ่านค่าปุ่ม
        if (button_state == 0) // ถ้าปุ่มถูกกด
        {
            count++; // เพิ่มค่า count
            ESP_LOGI(TAG, "Button pressed, count: %d", count);
        }
        else if (prev_button_state == 0) // ถ้าปล่อยปุ่ม
        {
            if (count > 0)
            {
                xQueueSend(xQueue, &count, portMAX_DELAY); // ส่งค่าไปยัง Queue
                ESP_LOGI(TAG, "Button released, sent count: %d", count);
                count = 0; // รีเซ็ตค่า
            }
        }
        prev_button_state = button_state; // อัปเดตสถานะปุ่มก่อนหน้า
        vTaskDelay(pdMS_TO_TICKS(500)); // หน่วงเวลา 0.5 วินาที
    }
}

// ** Task 2: Consumer (รับค่าและแสดงผล) **
void consumer_task(void *pvParameters)
{
    uint32_t received_count;
    while (1)
    {
        if (xQueueReceive(xQueue, &received_count, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Received count from producer: %d", received_count);
        }
    }
}

// ** Main function **
void app_main(void)
{
    xQueue = xQueueCreate(5, sizeof(uint32_t)); // สร้าง Queue รองรับ 5 ค่า

    if (xQueue != NULL)
    {
        xTaskCreate(producer_task, "Producer_Task", 2048, NULL, 2, NULL);
        xTaskCreate(consumer_task, "Consumer_Task", 2048, NULL, 2, NULL);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create queue");
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // ป้องกัน Watchdog Timer Error
    }
}
