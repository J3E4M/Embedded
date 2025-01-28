#include <stdio.h>
#include <sys/time.h>
#include "esp_sleep.h"

static RTC_DATA_ATTR int deep_sleep_count = 0; // ตัวนับรอบของการหลับลึก
static RTC_DATA_ATTR struct timeval sleep_enter_time;

void app_main(void) {
    const int wakeup_time_sec = 5; // ระยะเวลาหลับลึก 5 วินาที
    printf("Enabling timer wakeup, %d seconds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000 * 1000)); // ตั้งค่าเวลาปลุกในหน่วยไมโครวินาที

    struct timeval now;
    gettimeofday(&now, NULL);

    // คำนวณเวลาที่ใช้ในโหมดหลับลึก
    if (deep_sleep_count > 0) { // ตรวจสอบว่าเคยเข้าสู่โหมดหลับลึกมาก่อนหรือไม่
        int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 
                          + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;
        printf("Wake up from deep sleep. Time spent in deep sleep: %d ms\n", sleep_time_ms);
    }

    // เพิ่มตัวนับรอบการหลับลึก
    deep_sleep_count++;
    printf("Deep sleep count: %d\n", deep_sleep_count);

    // จับเวลาเริ่มต้นก่อนเข้าสู่โหมดหลับลึก
    gettimeofday(&sleep_enter_time, NULL);
    printf("Entering deep sleep...\n");

    esp_deep_sleep_start(); // เข้าสู่โหมดหลับลึก
}