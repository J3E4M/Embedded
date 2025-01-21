#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"

#define LED_GPIO_PIN 19 // GPIO ที่เชื่อมต่อ LED ภายนอก
static const char *TAG = "External_LED_Timer";

static bool led_on = false; // สถานะของ LED

// Callback Function ที่ทำงานเมื่อ Timer ถึงค่า Alarm
bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *event_data, void *user_ctx) {
    // สลับสถานะ LED
    led_on = !led_on;
    gpio_set_level(LED_GPIO_PIN, led_on); // ตั้งค่า GPIO ให้ LED เปิด/ปิด
    ESP_EARLY_LOGI(TAG, "LED %s", led_on ? "ON" : "OFF");

    return true; // ส่งค่า true เพื่อให้ Timer Auto-reload ทำงานต่อ
}

void app_main(void) {
    // ตั้งค่า GPIO สำหรับ LED
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "Configuring Timer");

    // สร้าง Timer และตั้งค่า
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,  // ใช้แหล่งสัญญาณนาฬิกาเริ่มต้น
        .direction = GPTIMER_COUNT_UP,      // Timer นับขึ้น
        .resolution_hz = 1000000,           // ความละเอียด 1 MHz (1 ไมโครวินาที)
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    // ตั้งค่า Alarm ของ Timer
    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,                  // ค่าเริ่มต้นของตัวนับ
        .alarm_count = 250000,             // 250,000 ไมโครวินาที = 250ms
        .flags.auto_reload_on_alarm = true // เปิดใช้ Auto-reload เมื่อถึง Alarm
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    // ลงทะเบียน Callback Function
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback, // เรียกใช้ timer_callback เมื่อ Timer ถึง Alarm
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // เปิดใช้งาน Timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    ESP_LOGI(TAG, "LED Blink started using Timer");

    // Main Task (ไม่ต้องหน่วงเวลา)
    while (true) {
        vTaskDelay(portMAX_DELAY); // Task หลักทำงานแบบไม่มีผลกระทบ
    }
}