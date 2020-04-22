#include <stdint.h>
#include "driver/gpio.h"
#include "power_control.h"
#include "esp_intr_alloc.h"
#include "esp_timer.h"

#define DET_PIN CONFIG_ZERO_DETECT_GPIO
#define REP_PIN CONFIG_DEBUG_OUT_GPIO

uint32_t g_powerCycles;


static void  gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num == DET_PIN) {
        handleZeroCross();
    };
}

uint64_t tStart;

void initPowerControlModule() 
{
    gpio_config_t io_conf;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << DET_PIN;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE; //rising edge trg
    //configure GPIO with the given settings
    esp_err_t res = gpio_config(&io_conf);
    if (res != ESP_OK) {
        printf("error gpio init");
        return;
    }
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = 0;
    io_conf.pin_bit_mask = 1ULL << REP_PIN;
    res = gpio_config(&io_conf);
    if (res != ESP_OK) {
        printf("error gpio init");
        return;
    }
    //install gpio isr service
    /*res = gpio_install_isr_service(0);
    if (res != ESP_OK) {
        printf("error gpio isr_service");
        return;
    }*/
    //hook isr handler for specific gpio pin
    res = gpio_isr_handler_add(DET_PIN, gpio_isr_handler, (void*) DET_PIN);
    if (res != ESP_OK) {
        printf("error gpio isr add");
        return;
    }
    tStart = esp_timer_get_time();
}

uint64_t lastZero = 0ULL;
//debounce - if we get an interrupt we ignore other interrupts for 2 ms (2 because detection impulse lasts about 1 ms)
void handleZeroCross() 
{
    uint64_t us = esp_timer_get_time();
    if (us - lastZero < 2000) return;
    lastZero = us;
    g_powerCycles++;
    gpio_set_level(REP_PIN, g_powerCycles % 2);

}


void setPowerOutState(uint8_t chan, uint8_t on)
{

}

uint8_t getPowerOutState(uint8_t chan)
{
    return 0;
}

