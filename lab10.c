#include <stdio.h>
#include <sys/time.h>
#include "esp_sleep.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;

void app_main(void) {
    const int wakeup_time_sec = 20;
    printf("Enabling timer wakeup, %ds\\r\\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000 * 1000)); // time in µs

    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 
                        + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER:
            printf("Wake up from timer. Time spent in deep sleep: %dms\\r\\n", sleep_time_ms);
            break;
        default:
            printf("Not a deep sleep reset\\r\\n");
            break;
    }

    printf("Entering deep sleep\\r\\n");
    gettimeofday(&sleep_enter_time, NULL);

    esp_deep_sleep_start();
}
