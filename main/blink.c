/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "timerwire.h"
#include "power_control.h"
#include "esp_timer.h"
#include <sys/time.h>
#include "ui_screen.h"
#include "temp_sensors.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 4

int64_t xx_time_get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

uint64_t tstart = 0ULL;

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
   tstart = esp_timer_get_time();
   uint64_t mstart = tstart;
    gpio_pad_select_gpio(BLINK_GPIO);
    setupTimer(BLINK_GPIO);
    initPowerControlModule();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //i2cscan(); need to disable or display will not show...
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //initializeDisplay2();
    printf("display inited\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    scanOneWire();
    initThermocoupleSensors();
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while(1) {
        tstart = esp_timer_get_time();
        g_powerCycles = 0;
        /* Blink off (output low) */
        printf("Turning off the LED at %d\n", g_timer_count);
        
        //gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
	    printf("Turning on the LED\n");
        //gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        uint64_t dt = (esp_timer_get_time() - tstart) / 1000;
        double freq = (double) g_powerCycles / dt;
        freq *= 1000.0;
        printf("Freq is so far %f, elapsed secs: %lld, ac %d\n", freq, (esp_timer_get_time() - mstart) / 1000000, g_powerCycles);
        
    }
}
