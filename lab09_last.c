#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_17

uint32_t ulVar = 0;               
QueueHandle_t xQueue = NULL;      

int isButtonPressed()
{
    return gpio_get_level(BUTTON_PIN) == 0; // สมมติว่ากดปุ่มจะได้ค่า 0
}

void vTaskProducer(void *pvParameters)
{
    uint32_t tempVar = 0; // ตัวแปรเก็บค่าชั่วคราว
    while (1)
    {
        if (isButtonPressed())
        {
            tempVar++;         // เพิ่มค่าทีละ 1
            ulVar = tempVar; 
        }
        else if (tempVar > 0) // หากปล่อยปุ่มและมีค่าใน tempVar
        {
            xQueueSend(xQueue, &ulVar, portMAX_DELAY);
            tempVar = 0;       
            ulVar = 0;          
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // หน่วงเวลา 0.5 วินาที
    }
}

void vTaskConsumer(void *pvParameters)
{
    uint32_t receivedValue = 0;
    while (1)
    {
        if (xQueueReceive(xQueue, &receivedValue, portMAX_DELAY)) // รอรับค่าจาก Queue
        {
            printf("Consumer Received Value: %ld\n", receivedValue);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // หน่วงเวลา 0.5 วินาที
    }
}

void vApplicationIdleHook(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000)); // หน่วงเวลา 1 วินาที
}

// ฟังก์ชันหลัก
void app_main(void)
{
    esp_rom_gpio_pad_select_gpio(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_PIN); 
    xQueue = xQueueCreate(10, sizeof(uint32_t));
    if (xQueue == NULL)
    {
        printf("Failed to create queue\n");
        return;
    }

    xTaskCreate(vTaskProducer, "Task Producer", 2048, NULL, 2, NULL);
    xTaskCreate(vTaskConsumer, "Task Consumer", 2048, NULL, 1, NULL);

}

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // ป้องกัน Watchdog Timer Error
    }
}
