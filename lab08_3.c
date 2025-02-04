#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/mcpwm.h"
#include "esp_log.h"

#define POTENTIOMETER_PIN ADC1_CHANNEL_4  // ขา ADC (GPIO32)
#define PWM_OUT_PIN GPIO_NUM_18           // ขา PWM Output (ไม่มีมอเตอร์)
#define PWM_MIN_PULSEWIDTH 500            // ความกว้างพัลส์ต่ำสุด (0.5ms)
#define PWM_MAX_PULSEWIDTH 2500           // ความกว้างพัลส์สูงสุด (2.5ms)
#define ADC_MAX_VALUE 4095                 // ค่าสูงสุดของ ADC (12-bit)

static const char *TAG = "PWM_TEST";

// ฟังก์ชันแปลงค่าจาก ADC เป็นค่าพัลส์ของ PWM
static uint32_t adc_to_pwm_pulsewidth(int adc_value)
{
    return (adc_value * (PWM_MAX_PULSEWIDTH - PWM_MIN_PULSEWIDTH) / ADC_MAX_VALUE) + PWM_MIN_PULSEWIDTH;
}

// Task อ่านค่า ADC และแสดงค่าความกว้างของ PWM
void pwm_test_task(void *arg)
{
    int adc_value = 0;
    uint32_t pulse_width;

    while (1)
    {
        adc_value = adc1_get_raw(POTENTIOMETER_PIN); // อ่านค่าจาก ADC
        pulse_width = adc_to_pwm_pulsewidth(adc_value); // แปลงเป็นค่าพัลส์ PWM

        ESP_LOGI(TAG, "ADC Value: %d, Pulse Width: %d us", adc_value, pulse_width);

        // ตั้งค่าความกว้างพัลส์ของ PWM
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

        vTaskDelay(pdMS_TO_TICKS(100)); // หน่วงเวลา 100 ms
    }
}

// ฟังก์ชันตั้งค่า PWM
void pwm_init()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWM_OUT_PIN);

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50; // ใช้ความถี่ 50 Hz
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

// ฟังก์ชันตั้งค่า ADC
void adc_init()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POTENTIOMETER_PIN, ADC_ATTEN_DB_11);
}

// ฟังก์ชันหลัก
void app_main()
{
    ESP_LOGI(TAG, "Initializing ADC and PWM");

    adc_init(); // ตั้งค่า ADC
    pwm_init(); // ตั้งค่า PWM

    xTaskCreate(pwm_test_task, "pwm_test_task", 2048, NULL, 5, NULL);
}

// วิธีทดสอบโดยใช้ Oscilloscope

// การต่อวงจร
// ขา ESP32	ต่อไปที่
// GPIO18 (PWM Output)	Probe ช่องสัญญาณ CH1 ของ Oscilloscope
// GND	GND ของ Oscilloscope

// ตั้งค่า Oscilloscope

// โหมดวัดค่า: "DC Coupling"
// ตั้งค่าความถี่ของ Oscilloscope: 50 Hz
// ตั้งค่า Time Division (Horizontal Scale): 5 - 10 ms/div
// ตั้งค่า Voltage Division (Vertical Scale): 1 - 2V/div
// Trigger Mode: ตั้งค่า Trigger Edge เป็น Rising Edge

// ----------------------------------------------------------------------------

//  ตัวอย่างการแสดงผลใน Serial Monitor

// I (1000) PWM_TEST: ADC Value: 0, Pulse Width: 500 us
// I (1100) PWM_TEST: ADC Value: 1024, Pulse Width: 1166 us
// I (1200) PWM_TEST: ADC Value: 2048, Pulse Width: 1833 us
// I (1300) PWM_TEST: ADC Value: 4095, Pulse Width: 2500 us
