
#include "timerwire.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"
#include "driver/timer.h"

static esp_timer_handle_t _theTimer;
uint8_t lvl = 0;
uint32_t g_timer_count = 0;

void softickHandler(void*p) {
    int pin = (int) p;
    gpio_set_level(pin, lvl);
    lvl = lvl ? 0 : 1;
}

void setupSoftTimer(int pin) {

    esp_timer_create_args_t tArgs;
    tArgs.callback = softickHandler;
    tArgs.arg = pin;
    tArgs.dispatch_method = ESP_TIMER_TASK;
    esp_err_t res = esp_timer_create(&tArgs, &_theTimer);
    if (res != ESP_OK) {
        printf("timer init fail");
        return;
    }
    esp_timer_start_periodic(_theTimer, 1000 * 50);
}

void hardTimerIsr(void* p) 
{
    int pin = (int) p;
    g_timer_count++;
    uint32_t tc = g_timer_count / 500;
    TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[1].config.alarm_en = 1;
    gpio_set_level(pin, tc % 2 == 0 ? 1 : 0);
}


esp_err_t setupHardTimer(int pin) {

    timer_config_t cfg;
    timer_idx_t tmi = TIMER_1;
    esp_err_t res = 0;
    /*
    res = timer_group_intr_enable(TIMER_GROUP_0, TIMER_INTR_T1);
    if (res != ESP_OK) {
        printf("timer group intr error");
        return res;
    }*/

    cfg.divider =  80; //80 mhz bus -> 1us resolution, but divider is up to 65536
    cfg.auto_reload = TIMER_AUTORELOAD_EN;
    cfg.counter_dir = TIMER_COUNT_UP;
    cfg.counter_en = TIMER_PAUSE;
    cfg.intr_type = TIMER_INTR_LEVEL; //only supported
    cfg.alarm_en = TIMER_ALARM_EN;
    cfg.counter_en = false;

    uint32_t timerFreq = TIMER_BASE_CLK / cfg.divider;

    res = timer_init(TIMER_GROUP_0, tmi, &cfg);
    if (res != ESP_OK) {
        printf("timer init error");
        return res;
    }

    res = timer_set_counter_value(TIMER_GROUP_0, tmi, 0x00000000ULL);

    res = timer_set_alarm_value(TIMER_GROUP_0, tmi, 0.5 * timerFreq);
    res = timer_enable_intr(TIMER_GROUP_0, tmi);
    if (res != ESP_OK) {
        printf("timer enable intr fail");
        return res;
    }
    timer_isr_handle_t s_timer_handle;
    res = timer_isr_register(TIMER_GROUP_0, tmi, hardTimerIsr, (void *) pin, 0, &s_timer_handle);
    if (res != ESP_OK) {
        printf("timer intr fail");
        return res;
    }
    res = timer_start(TIMER_GROUP_0, tmi);
    if (res != ESP_OK) {
        printf("timer start fail");
        return res;
    }
    return ESP_OK;
}

void setupTimer(int pin) {

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    timer_config_t ts;
    //setupSoftTimer(pin);
    setupHardTimer(pin);
}