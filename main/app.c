#include "app.h"

static const char *TAG = "APP";

static WS2812_stripe_t stripe;
static const uint8_t stripe_length = 5;

static i2c_port_t i2c_port0 = I2C_NUM_0;

void WS2812_task(void *pvParameters) {

    ESP_LOGI(TAG, "Entering WS2812 task\n");

    uint8_t mode = 0;

    WS2812_color_t warmwhite = { 255, 150, 70 };
    WS2812_color_t pink = { 255, 0, 255 };

    if (mode == 0) {
        while (1) {
            WS2812_color_t color = warmwhite;
            for (uint8_t x = 0; x <  stripe.length; x++) {
                WS2812_set_color(&stripe, x, &color);
            }
            WS2812_write(&stripe);
            delay_us(1000000);
        }
    } else if (mode == 1) {
        uint8_t val = 0;
        int8_t direction = 1;

        while (1) {
            val += direction;

            if (val == 0 || val == 255) {
                direction *= -1;
            }

            WS2812_color_t color = { val, 0, val };
            for (uint8_t x = 0; x <  stripe.length; x++) {
                WS2812_set_color(&stripe, x, &color);
            }
            WS2812_write(&stripe);
        }
    } else if (mode == 2) {
        uint8_t add = 0;
        uint8_t sub = 2;
        uint8_t step = 4;

        while (1) {
            WS2812_color_t last;

            for (uint16_t i = stripe.length; i > 0; i--) {
                WS2812_get_color(&stripe, i-1, &last);
                WS2812_set_color(&stripe, i, &last);
            }

            uint8_t new[3] = { last.r, last.g, last.b };

            // if uint8_t running over
            if ((new[add] + step) % 255 < new[add]) {
                // ESP_LOGI(TAG, "INC SUB UND ADD");
                sub = add;
                add = (add + 1) % 3;
            }

            new[add] = new[add] + step;

            // if uint8_t not running over
            if (new[sub] >= step) {
                new[sub] = new[sub] - step;
            }

            // ESP_LOGI(TAG, "New color: %d %d %d", new[0], new[1], new[2]);

            WS2812_color_t color = { new[0], new[1], new[2] };
            WS2812_set_color(&stripe, 0, &color);

            WS2812_write(&stripe);
            delay_us(10000);
            esp_task_wdt_reset();
        }
    }
}

void SSD1306_task(void *pvParameters)
{
    EventBits_t event_bits = 0;
    ESP_LOGI(TAG, "Starting SSD1306 task");

    SSD1306_set_bitmap(espressif, 124, 24, 2, 20);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    SSD1306_set_bitmap(wifi, 36, 24, 46, 20);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    SSD1306_set_bitmap(bluetooth, 90, 22, 19, 21);
    SSD1306_display();
    vTaskDelay(333 / portTICK_PERIOD_MS);

    event_bits = xEventGroupGetBits(WIFI_event_group);

    // if (!(event_bits & WIFI_STA_CONNECTED_BIT)) {
        SSD1306_set_text_6x8(FONT_lcd5x7, "Connecting to...", 4, 23);
        SSD1306_set_text_6x8(FONT_lcd5x7, WIFI_STA_SSID, 4, 33);
        SSD1306_display();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    // }

    xEventGroupWaitBits(WIFI_event_group, WIFI_STA_CONNECTED_BIT, false, true, portMAX_DELAY);

    // if (!(event_bits & SNTP_TIME_SET_BIT)) {
        SSD1306_set_text_6x8(FONT_lcd5x7, "Fetching time...", 4, 28);
        SSD1306_display();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    // }

    xEventGroupWaitBits(WIFI_event_group, SNTP_TIME_SET_BIT, false, true, portMAX_DELAY);

    time_t t;
    timeinfo_t timeinfo = { 0 };

    char buffer[50];
    char strftime_buf[6];

    uint8_t buffer_bitmap_8x8[64];

    while (1) {
        time(&t);
        localtime_r(&t, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%R", &timeinfo);

        
        SSD1306_set_text_6x8(FONT_lcd5x7, strftime_buf, 95, 4);
        SSD1306_set_bitmap(bluetooth_icon_8x8, 8, 8, 85, 3);

        WIFI_sta_rssi_bitmap_8x8(&buffer_bitmap_8x8);
        SSD1306_set_bitmap(buffer_bitmap_8x8, 8, 8, 75, 3);

        SSD1306_set_bitmap(wifi_icon_8x8, 8, 8, 65, 3);

        sprintf(buffer, "Temp.:   %.2f *C", BME280_get_temperature_double());
        SSD1306_set_text_6x8(FONT_lcd5x7, buffer, 4, 18);

        // printf("BME280 Hum:   %.2f %%rH", BME280_get_humidity_double());
        sprintf(buffer, "Press.: %.2f hPa",  BME280_get_pressure_double() / 100);
        SSD1306_set_text_6x8(FONT_lcd5x7, buffer, 4, 30);

        /*
        for (uint16_t i = 0; i < SSD1306_LCDWIDTH * 2; i++) {
            _buffer[i] = ~_buffer[i];
        }
        */

        SSD1306_display();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*
void KEYBOARD_task(void *pvParameters)
{
    KEYBOARD_display();

    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/

void delay_task(
        void *pvParameters) {

    uint8_t i = 0;

    while(1) {

        delay_hundred_ns(10000000);
        printf("%u\n", ++i);
        delay_us(1000000);
        printf("%u\n", ++i);

        /*
        printf("001 us: %lu\n", micros());
        printf("100 ns: %lu\n", hundret_ns());
        printf("001 us: %lu\n", micros());
        printf("100 ns: %lu\n", hundret_ns());
        printf("001 us: %lu\n", micros());
        printf("100 ns: %lu\n", hundret_ns());
        */
    }
}

void app_main()
{
    esp_err_t esp_err;
    gpio_install_isr_service(0);

    // Init I2C bus and sensors

    i2c_config_t i2c_port0_conf;
    i2c_port0_conf.mode = I2C_MODE_MASTER;
    i2c_port0_conf.sda_io_num = I2C_P0_GPIO_SDA;
    i2c_port0_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.scl_io_num = I2C_P0_GPIO_SCL;
    i2c_port0_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_port0_conf.master.clk_speed = 1600000;

    I2C_init(i2c_port0, &i2c_port0_conf);

    ESP_ERROR_CHECK( SSD1306_init(i2c_port0, SSD1306_ADDR_LOW) );
    ESP_ERROR_CHECK( BME280_init(i2c_port0, BME280_ADDR_LOW) );

    xTaskCreate(&SSD1306_task, "SSD1306_task", 2048, NULL, 10, NULL);

    WIFI_init(WIFI_MODE_STA, NULL);

    // xTaskCreate(&delay_task, "delay_task", 2048, NULL, 10, NULL);

    // Init WS2812 stripe

    stripe.gpio_num = WS2812_GPIO;
    stripe.length = stripe_length;
    stripe.rmt_channel = RMT_CHANNEL_0;
    stripe.rmt_interrupt_num = 0;

    esp_err = WS2812_init(&stripe);
    if (!esp_err) {
        ESP_LOGI(TAG, "WS2812 init done");
        xTaskCreate(&WS2812_task, "WS2812_task", 8192, NULL, 10, NULL);
    }

    // ESP_ERROR_CHECK( ROTENC_init(ROTENC_GPIO_CLK, ROTENC_GPIO_DT, ROTENC_GPIO_SW) );

    // xTaskCreate(&KEYBOARD_task, "KEYBOARD_task", 2048, NULL, 10, NULL);
    // xTaskCreate(&BME280_task, "BME280_task", 2048, NULL, 10, NULL);

    return;

    // TODO: How to decide if update should be started?
    // xTaskCreate(&OTA_task, "OTA_task", 2048, NULL, 10, NULL);

    /*
    esp_bt_controller_init();

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        return;
    }

    ESP_LOGI("TAG", "TEST");

    xTaskCreate(&bleAdvtTask, "bleAdvtTask", 2048, (void* ) 0, 10, NULL);
//    xTaskCreate(&wifiTask,    "wifiTask",    2048, (void* ) 1, 10, NULL);
     */

    // don't call vTaskStartScheduler() because
    // the scheduler is already running per default
}

