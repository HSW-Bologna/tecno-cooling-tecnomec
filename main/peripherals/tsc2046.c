/*
 * Funzioni di comunicazione con l'integrato tsc2046 per il touch screen.
 * Gestiscono uno stato interno con una media dei valori letti e un mutex
 * per garantire la mutua esclusione sull'accesso a suddetti valori.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h>
#include "tsc2046.h"

/*********************
 *      DEFINES
 *********************/
#define CMD_X_READ  0b10010000
#define CMD_Y_READ  0b11010000
#define CMD_Z1_READ 0b10110000
#define CMD_Z2_READ 0b11000000
#define TP_VER_RES  320
#define TP_HOR_RES  240


/**********************
 *  STATIC VARIABLES
 **********************/
// static int     touch_x = 0, touch_y = 0, f_touch_detected = 0;
static int16_t avg_buf_x[XPT2046_AVG];
static int16_t avg_buf_y[XPT2046_AVG];
static uint8_t avg_last;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void tsc2046_corr(int16_t *x, int16_t *y) {
#if XPT2046_XY_SWAP != 0
    int16_t swap_tmp;
    swap_tmp = *x;
    *x       = *y;
    *y       = swap_tmp;
#endif

    if ((*x) > XPT2046_X_MIN)
        (*x) -= XPT2046_X_MIN;
    else
        (*x) = 0;

    if ((*y) > XPT2046_Y_MIN)
        (*y) -= XPT2046_Y_MIN;
    else
        (*y) = 0;

    (*x) = (uint32_t)((uint32_t)(*x) * TP_HOR_RES) / (XPT2046_X_MAX - XPT2046_X_MIN);

    (*y) = (uint32_t)((uint32_t)(*y) * TP_VER_RES) / (XPT2046_Y_MAX - XPT2046_Y_MIN);

#if XPT2046_X_INV != 0
    (*x) = TP_HOR_RES - (*x);
#endif

#if XPT2046_Y_INV != 0
    (*y) = TP_VER_RES - (*y);
#endif
}


static void tsc2046_avg(int16_t *x, int16_t *y) {
    /*Shift out the oldest data*/
    uint8_t i;
    for (i = XPT2046_AVG - 1; i > 0; i--) {
        avg_buf_x[i] = avg_buf_x[i - 1];
        avg_buf_y[i] = avg_buf_y[i - 1];
    }

    /*Insert the new point*/
    avg_buf_x[0] = *x;
    avg_buf_y[0] = *y;
    if (avg_last < XPT2046_AVG)
        avg_last++;

    /*Sum the x and y coordinates*/
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    for (i = 0; i < avg_last; i++) {
        x_sum += avg_buf_x[i];
        y_sum += avg_buf_y[i];
    }

    /*Normalize the sums*/
    (*x) = (int32_t)x_sum / avg_last;
    (*y) = (int32_t)y_sum / avg_last;
}

static uint16_t tsc_data(spi_device_handle_t spi, const uint8_t data) {
    esp_err_t ret;
    uint8_t   datas[3] = {0};
    datas[0]           = data;

    spi_transaction_t t = {
        .length    = 8 * 3,      // Len is in bytes, transaction length is in bits.
        .tx_buffer = &datas,     // Data
        .flags     = SPI_TRANS_USE_RXDATA,
    };
    ret = spi_device_polling_transmit(spi, &t);     // Transmit!
    assert(ret == ESP_OK);                          // Should have had no issues.

    return (t.rx_data[1] << 8 | t.rx_data[2]) >> 3;
}

spi_device_handle_t tpspi;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void tsc2046_init() {
    esp_err_t ret;

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2 * 1000 * 1000,     // Clock out at 80 MHz
        .mode           = 0,                   // SPI mode 0
        .spics_io_num   = TP_SPI_CS,           // CS pin
        .queue_size     = 10,
        .pre_cb         = NULL,
        .post_cb        = NULL,
    };

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &tpspi);
    assert(ret == ESP_OK);

    gpio_set_direction(XPT2046_IRQ, GPIO_MODE_INPUT);
}


/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no more data to be read
 */
void tsc2046_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
#define DEBOUNCE_READINGS 2
    static int16_t last_x  = 0;
    static int16_t last_y  = 0;
    bool           valid   = true;
    static size_t  counter = 0;

    int16_t x = 0;
    int16_t y = 0;

    uint8_t irq = gpio_get_level(XPT2046_IRQ);

    if (irq == 0) {
        x = tsc_data(tpspi, CMD_X_READ);
        y = tsc_data(tpspi, CMD_Y_READ);

        printf("%4i, %4i\n", x, y);

        // Se le coordinate sono quelle da fermo ignora la lettura
        if ((x == 4095 && y == 0) || x > 4095 || y > 4095) {
            x        = last_x;
            y        = last_y;
            avg_last = 0;
            valid    = false;
            counter  = 0;
        } else {
            if (counter >= DEBOUNCE_READINGS) {
                tsc2046_corr(&x, &y);
                tsc2046_avg(&x, &y);
                last_x = x;
                last_y = y;
            } else {
                counter++;
                x        = last_x;
                y        = last_y;
                avg_last = 0;
                valid    = false;
            }
        }
    } else {
        x        = last_x;
        y        = last_y;
        avg_last = 0;
        valid    = false;
        counter  = 0;
    }

    data->point.x = x;
    data->point.y = y;
    data->state   = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
}
