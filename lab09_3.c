#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t xMutex = NULL;
const TickType_t xMaxBlockTimeTicks = 0x20;

static void vPrintString(const char *pcString) {
    // Take a semaphore for using resource (only 1 task to print string to display)
    xSemaphoreTake(xMutex, portMAX_DELAY);
    printf("%s", pcString);
    fflush(stdout);
    // Leave resource back for another task
    xSemaphoreGive(xMutex);
}

static void prvPrintTask(void *pvParameter) {
    char *pcString = (char *)pvParameter;
    const TickType_t xSlowDownDelay = pdMS_TO_TICKS(500);

    while (1) {
        vPrintString(pcString);
        vTaskDelay(rand() % xMaxBlockTimeTicks);
        vTaskDelay(xSlowDownDelay);
    }
}

const char *pvStringForTask1 = "Task 1 **************************************\r\n";
const char *pvStringForTask2 = "Task 2 --------------------------------------\r\n";

void app_main(void) {
    xMutex = xSemaphoreCreateMutex();

    xTaskCreate(prvPrintTask, "Task 1", 2048, (void *)pvStringForTask1, 1, NULL);
    xTaskCreate(prvPrintTask, "Task 2", 2048, (void *)pvStringForTask2, 2, NULL);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
