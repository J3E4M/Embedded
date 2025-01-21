#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

uint32_t ulVar = 0;
QueueHandle_t queue;

void taskProducer(void *pvParameter) {
    while (1) {
        ulVar++;
        xQueueSend(queue, (void *)&ulVar, (TickType_t)10);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void taskConsumer(void *pvParameter) {
    uint32_t data;
    while (1) {
        if (xQueueReceive(queue, &data, portMAX_DELAY)) {
            printf("[Queue] Recv, val: %d\r\n", data);
        }
    }
}

void app_main(void) {
    queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(taskProducer, "Producer", 2048, NULL, 10, NULL);
    xTaskCreate(taskConsumer, "Consumer", 2048, NULL, 10, NULL);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
