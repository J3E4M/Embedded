#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define GPS_UART_NUM         UART_NUM_1     // ใช้ UART1
#define GPS_TXD_PIN          GPIO_NUM_17    // TXD Pin ของ ESP32
#define GPS_RXD_PIN          GPIO_NUM_16    // RXD Pin ของ ESP32
#define GPS_BAUD_RATE        9600           // Baud Rate ของ GPS Module
#define GPS_BUFFER_SIZE      1024           // ขนาดของ Buffer

static const char *TAG = "GPS_Module";

void gps_task(void *arg)
{
    // สร้าง Buffer สำหรับเก็บข้อมูลที่อ่านจาก GPS
    uint8_t data[GPS_BUFFER_SIZE];

    while (1)
    {
        // อ่านข้อมูลจาก GPS Module
        int len = uart_read_bytes(GPS_UART_NUM, data, GPS_BUFFER_SIZE - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            data[len] = '\0'; // เพิ่ม null-terminator เพื่อให้เป็น string
            ESP_LOGI(TAG, "GPS Data: %s", data);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // หน่วงเวลา 100 ms
    }
}

void app_main(void)
{
    // ตั้งค่า UART สำหรับ GPS
    uart_config_t uart_config = {
        .baud_rate = GPS_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // ติดตั้ง UART Driver
    ESP_ERROR_CHECK(uart_driver_install(GPS_UART_NUM, GPS_BUFFER_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(GPS_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(GPS_UART_NUM, GPS_TXD_PIN, GPS_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // สร้าง Task สำหรับอ่านข้อมูล GPS
    xTaskCreate(gps_task, "gps_task", 2048, NULL, 10, NULL);
}
