#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define STACK_SIZE 2048
#define TASK_PRIORITY 10

const char *pcTextForTask1 = "Task 1 is running\r\n";
const char *pcTextForTask2 = "Task 2 is running\r\n";

void vTaskFunction(void *pvParameters) {
    while (1) {
        printf("%s", (char *)pvParameters);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    xTaskCreate(
        vTaskFunction,         // Task function to process
        "Task 1",              // Task name
        STACK_SIZE,            // Stack size
        (void *)pcTextForTask1, // Parameter to pass through each task
        TASK_PRIORITY,         // Task Priority
        NULL                   // Pointer to task handler
    );

    xTaskCreate(
        vTaskFunction,         // Task function to process
        "Task 2",              // Task name
        STACK_SIZE,            // Stack size
        (void *)pcTextForTask2, // Parameter to pass through each task
        TASK_PRIORITY,         // Task Priority
        NULL                   // Pointer to task handler
    );

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
