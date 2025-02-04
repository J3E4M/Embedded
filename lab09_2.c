#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// Define GPIO pin for button
#define BUTTON_PIN GPIO_NUM_14  // D5 on ESP32 is GPIO 14

// Define Queue Handle
QueueHandle_t xQueue;

// Prototype declaration
void vProducerTask(void *pvParameters);
void vConsumerTask(void *pvParameters);

// Initialize button
void initButton()
{
    gpio_reset_pin(BUTTON_PIN);  // Reset GPIO pin
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY); // Use internal pull-up resistor
}

// Producer Task (Task 1)
void vProducerTask(void *pvParameters)
{
    int value = 0; // Variable to store the count
    int buttonState = 1; // Track button state (1 = released, 0 = pressed)
    int lastButtonState = 1; // Track the last state of the button

    while (1)
    {
        buttonState = gpio_get_level(BUTTON_PIN); // Read the button state

        if (buttonState == 0) // Button is being held down
        {
            value++; // Increment value
            // printf("Task 1: Counting value %d\n", value);
            vTaskDelay(pdMS_TO_TICKS(500)); // Delay for counting speed
        }

        if (buttonState == 1 && lastButtonState == 0) // Button just released
        {
            // Send value to queue when button is released
            printf("Task 1: Button released, sending value %d\n", value);
            if (xQueueSend(xQueue, &value, portMAX_DELAY) != pdPASS)
            {
                printf("Task 1: Failed to send value to queue\n");
            }

            value = 0; // Reset value after sending
            printf("Task 1: Value reset to 0\n");
        }

        lastButtonState = buttonState; // Update the last button state
        vTaskDelay(pdMS_TO_TICKS(50)); // Delay to prevent busy loop
    }
}

// Consumer Task (Task 2)
void vConsumerTask(void *pvParameters)
{
    int receivedValue;
    while (1)
    {
        // Receive value from queue
        if (xQueueReceive(xQueue, &receivedValue, portMAX_DELAY) == pdPASS)
        {
            printf("Task 2: Received value %d\n", receivedValue);
        }
    }
}

// Main application
void app_main(void)
{
    // Initialize button GPIO
    initButton();

    // Create a queue to hold up to 10 integer values
    xQueue = xQueueCreate(10, sizeof(int));
    if (xQueue == NULL)
    {
        printf("Failed to create queue\n");
        return;
    }

    // Create Producer Task
    xTaskCreate(vProducerTask, "ProducerTask", 2048, NULL, 1, NULL);

    // Create Consumer Task
    xTaskCreate(vConsumerTask, "ConsumerTask", 2048, NULL, 2, NULL);

    // Prevent app_main from exiting
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}
