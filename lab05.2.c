#include <stdio.h>
#include <inttypes.h>
#include "driver/gpio.h"

// Include FreeRTOS header for vTaskDelay and portTICK_PERIOD_MS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Define GPIO pins
#define GPIO_INPUT_IO_0     CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_IO_2     CONFIG_GPIO_INPUT_2
#define GPIO_INPUT_PIN_SEL  ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define GPIO_INPUT_PIN_INTR_SEL (1ULL << GPIO_INPUT_IO_2)

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

// ISR for Interrupt
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Task to show output to computer when interrupt was triggered
static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%" PRIu32 "] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void app_main(void)
{
    // ***************** Polling
    gpio_config_t io_conf;
    // Disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // Set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // Bit mask of the pins to set, e.g., GPIO18/19
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // Disable pull-down mode
    io_conf.pull_down_en = 0;
    // Enable pull-up mode
    io_conf.pull_up_en = 1;
    // Configure GPIO
    gpio_config(&io_conf);

    // ***************** Interrupt
    // Enable interrupt
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // NEGEDGE, POSEDGE, ANYEDGE
    // Set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // Bit mask of the pins to set, e.g., GPIO
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_INTR_SEL;
    // Disable pull-down mode
    io_conf.pull_down_en = 0;
    // Enable pull-up mode
    io_conf.pull_up_en = 1;
    // Configure GPIO
    gpio_config(&io_conf);

    // Set ISR for interrupt service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // Hook ISR handler for a specific GPIO pin
    gpio_isr_handler_add(GPIO_INPUT_IO_2, gpio_isr_handler, (void*) GPIO_INPUT_IO_2);

    // ***************** Create a Task
    // Create a queue to handle GPIO events from ISR
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // Create a display task for ISR
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    int cnt = 0;
    for (;;)
    {
        printf("cnt: %d\n", cnt++);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO_0, cnt & 0x01);
        gpio_set_level(GPIO_OUTPUT_IO_1, cnt & 0x02);
        printf("GPIO[%d] poll, val: %d\n", GPIO_INPUT_IO_0, gpio_get_level(GPIO_INPUT_IO_0));
        printf("GPIO[%d] poll, val: %d\n", GPIO_INPUT_IO_1, gpio_get_level(GPIO_INPUT_IO_1));
    }
}
