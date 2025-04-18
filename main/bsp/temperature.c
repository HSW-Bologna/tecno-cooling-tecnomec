#include <driver/spi_master.h>
#include "hardwareprofile.h"
#include "temperature.h"
#include "spi_common/spi_common.h"
#include "spi_devices/rtdd/MAX31865/max31865.h"
#include "spi_ports/esp-idf/esp_idf_spi_port.h"
#include "services/timestamp.h"
#include <esp_log.h>


#define AVERAGE_BUFFER_SIZE 10


static int cs_control(int level, void *user_data);


static spi_driver_t max31865_driver = {
    .spi_exchange   = esp_idf_spi_port_exchange,
    .spi_cs_control = cs_control,
};

static const char  *TAG                                 = __FILE_NAME__;
static int          round_trip                          = 0;
static size_t       average_index                       = 0;
static unsigned int average_buffer[AVERAGE_BUFFER_SIZE] = {0};


void bsp_temperature_init(void) {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(BSP_HAP_CS_PT100),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 500UL * 1000UL,     // Up to 5MHz
        .mode           = 1,
        .spics_io_num   = -1,
        .queue_size     = 10,
        .pre_cb         = NULL,
        .post_cb        = NULL,
    };

    spi_device_handle_t spi_handle = NULL;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
    max31865_driver.user_data = spi_handle;

    max31865_set_wires(max31865_driver, MAX31865_THREE_WIRES);
    vTaskDelay(pdMS_TO_TICKS(5));
    max31865_bias(max31865_driver, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    max31865_autoconvert(max31865_driver, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    max31865_set_high_threshold(max31865_driver, 0x7FFF);
    vTaskDelay(pdMS_TO_TICKS(5));
    max31865_set_low_threshold(max31865_driver, 0x0000);
    vTaskDelay(pdMS_TO_TICKS(5));
    max31865_clear_fault(max31865_driver);
    vTaskDelay(pdMS_TO_TICKS(5));
}


double bsp_temperature_get(void) {
    unsigned long sum = 0;

    size_t length = round_trip ? AVERAGE_BUFFER_SIZE : average_index;

    if (length == 0) {
        return 0;
    }

    for (size_t i = 0; i < length; i++) {
        sum += average_buffer[i];
    }

    return max31865_calculate_temperature(4000., (uint16_t)(sum / length));
}


double bsp_temperature_manage(void) {
    static timestamp_t ts     = 0;
    static double      cached = 0;

    if (timestamp_is_expired(ts, 1000)) {
        if (max31865_status(max31865_driver)) {
            max31865_clear_fault(max31865_driver);
        }

        average_buffer[average_index] = max31865_read_resistance(max31865_driver);

        if (average_index + 1 >= AVERAGE_BUFFER_SIZE) {
            // Completed on full round
            round_trip = 1;
        }

        cached = bsp_temperature_get();
        ESP_LOGI(TAG, "%i %f %i", average_buffer[average_index], cached, max31865_status(max31865_driver));

        // Full sensor row, on to the next reading
        average_index = (average_index + 1) % AVERAGE_BUFFER_SIZE;

        ts = timestamp_get();
    }

    return cached;
}


static int cs_control(int level, void *user_data) {
    (void)user_data;
    gpio_set_level(BSP_HAP_CS_PT100, level);
    return 0;
}
