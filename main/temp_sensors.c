#include "onewire.h"
#include <stdint.h>
#include "temp_sensors.h"
#include "ds18x20.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "hwsetup.h"

static const char* TAG = "temp_sensors";


void scanOneWire() {
    gpio_num_t gpio = ONEWIRE_PIN;
    //not needed gpio_pad_select_gpio(gpio);
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = 0;
    io_conf.pin_bit_mask = 1ULL << gpio;
    esp_err_t res = gpio_config(&io_conf);
    if (res != ESP_OK) {
        printf("error gpio init");
        return;
    }
    

    gpio_set_level(gpio, 1); //we need this initially for first scan to work
    //vTaskDelay(100/portTICK_PERIOD_MS);

    onewire_search_t srch;
    onewire_search_start(&srch);
    int devs =0;
    while(true) {
        onewire_addr_t addr = onewire_search_next(&srch, gpio);
        if (addr == ONEWIRE_NONE) {
            printf("end of search\n");
            break;
        }
        devs++;
    }
    
    printf("found %d devices\r\n", devs);

     
    printf("found %d devices\r\n", devs);


    ds18x20_addr_t addrs[8];

    int n = ds18x20_scan_devices(gpio, addrs, 8);
    for(int i=0; i<n;i++) {
        printf("DALLAS 0x%llx", addrs[i]);
        float t;
        esp_err_t res = ds18x20_measure_and_read(gpio, addrs[i], &t);
        if (res != ESP_OK) {
            printf("Failed to read temp 0x%x\n", res);
        }
        else {
            printf("Temp is %f\n", t);
        }

    }
}

spi_device_handle_t spi;

void spi_therm_init(void) {

  spi_device_interface_config_t devCfg={
    .mode = 0,
    .clock_speed_hz = 1000*1000,
    .spics_io_num=MAX6675_CS_PIN,
    .queue_size=3
  };

  esp_err_t res = spi_bus_add_device(VSPI_HOST, &devCfg, &spi);
  if (res != ESP_OK) {
      printf("failed to add spi device 0x%x\n", res);
  }
}

float spi_therm_read(void) {

    printf("readingn SPI temp\n");
  uint16_t bits = 0;
  spi_transaction_t tM = {
    .tx_buffer = NULL,
    .rx_buffer = &bits,
    .length = 16,
    .rxlength = 16,
  };

  spi_device_acquire_bus(spi, portMAX_DELAY); // Probably unnecessary
  vTaskDelay(500 / portTICK_RATE_MS);
  spi_device_transmit(spi, &tM);
  spi_device_release_bus(spi);

  uint16_t res = SPI_SWAP_DATA_RX(bits,16);
    printf("RAW read temp 0x%x, 0x%x\n", res >> 3, res & 0x07);
    if (res & 0x04) {
        ESP_LOGD(TAG, "Thermocouple input is open");
    }
  res >>= 3;
  ESP_LOGD(TAG, "Value %d, temp %f", res, res*0.25);
    printf("MAX6675 temp: %f\n", res * 0.25);

    
    uint16_t temp = ((((bits & 0x00FF) << 8) | ((bits & 0xFF00) >> 8))>>3)*25;
   // temp = ((rawtemp)>>3)*25;
	ESP_LOGI(TAG, "readMax6675 spiReadWord=%x temp=%d.%d",bits, temp/100, temp%100);

  return res*0.25;

}

esp_err_t spi_bus_init() {
    spi_bus_config_t buscfg={
        .miso_io_num=GPIO_NUM_19,
        .mosi_io_num=-1, //GPIO_NUM_23,
        .sclk_io_num=GPIO_NUM_18,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=0
    };

    esp_err_t res = spi_bus_initialize(VSPI_HOST, &buscfg, 0);
    if (res != ESP_OK) {
        printf("VSPI init error 0x%x\n", res);
    }
    return res;
}

void initThermocoupleSensors() {
    esp_err_t res = spi_bus_init();
    if (res != ESP_OK) {
        return res;
    };
    spi_therm_init();
    for (int i=0;i<3; i++) {
        spi_therm_read();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
    
}