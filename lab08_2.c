
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm.h"
#include "driver/adc.h"
#include "esp_log.h"

#define TAG "PWM_SERVO"

// ADC Config
#define ADC_CHANNEL ADC_CHANNEL_6 // GPIO34 (ADC1_CH6)
#define ADC_ATTEN ADC_ATTEN_DB_11 // ADC attenuation
#define ADC_WIDTH ADC_WIDTH_BIT_12

// MCPWM Config
#define SERVO_GPIO 18            // GPIO to connect to Servo Signal
#define PWM_FREQ 50              // 50 Hz PWM for Servo Motor
#define SERVO_MIN_PULSEWIDTH 500 // Minimum pulse width in microseconds (-90 degrees)
#define SERVO_MAX_PULSEWIDTH 2500 // Maximum pulse width in microseconds (90 degrees)

static void adc_init() {
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
}

static void pwm_servo_init() {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_GPIO);

    mcpwm_config_t pwm_config = {
        .frequency = PWM_FREQ, // PWM frequency = 50Hz
        .cmpr_a = 0,           // duty cycle of PWMxA = 0%
        .cmpr_b = 0,           // duty cycle of PWMxB = 0%
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

static uint32_t calculate_pulsewidth(int adc_value) {
    // Map ADC value (0-4095) to pulse width (500-2500 microseconds)
    return SERVO_MIN_PULSEWIDTH + (SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * adc_value / 4095;
}

void app_main(void) {
    adc_init();
    pwm_servo_init();

    while (1) {
        // Read ADC value
        int adc_value = adc1_get_raw(ADC_CHANNEL);

        // Calculate pulse width for servo motor
        uint32_t pulse_width = calculate_pulsewidth(adc_value);

        // Apply PWM to servo motor
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

        ESP_LOGI(TAG, "ADC Value: %d, Pulse Width: %dus", adc_value, pulse_width);

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
    }
}
```

// ### การอธิบายโค้ด:
// 1. **ADC**:
//    - อ่านค่าจากตัวต้านทานปรับค่าได้ผ่าน ADC1
//    - ค่าที่อ่านได้จะอยู่ระหว่าง 0 ถึง 4095 (12-bit resolution)

// 2. **PWM**:
//    - กำหนด PWM ความถี่ 50 Hz (ใช้สำหรับ Servo Motor)
//    - ใช้ MCPWM Unit เพื่อสร้างสัญญาณ PWM ที่สามารถปรับ Pulse Width ได้
//    - Pulse Width ถูกคำนวณจากค่าที่ได้จาก ADC โดย Map ค่าจาก 0-4095 ไปยัง 500-2500 μs

// 3. **Servo Control**:
//    - 500 μs → -90 องศา
//    - 2500 μs → 90 องศา

// ### การทดสอบ:
// - ใช้ออสซิลโลสโครปเพื่อตรวจสอบสัญญาณ PWM ที่ GPIO 18
// - เก็บภาพหน้าจอเมื่อปรับค่าตัวต้านทานไปที่ตำแหน่งต่ำสุดและสูงสุด
// - สังเกตการหมุนของเซอร์โวมอเตอร์ตามค่าที่อ่านได้จาก ADC

// หากต้องการปรับปรุงเพิ่มเติมหรือสอบถามเกี่ยวกับการทำงานของโค้ด สามารถแจ้งได้เลย!