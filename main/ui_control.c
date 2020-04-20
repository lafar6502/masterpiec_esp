#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <esp_log.h>

#include "ui_control.h"

//ESP_EVENT_DEFINE_BASE(MP_UI_EVENT);
const char* TAG = "ui";

const TickType_t IdleDelay = 5000 / portTICK_PERIOD_MS;






QueueHandle_t g_mpuiQueue;

void setupUIEventQueue() 
{
    g_mpuiQueue = xQueueCreate(10, sizeof(MPUIEvent));
    
    if (g_mpuiQueue == NULL) {
        //fatal error
        ESP_LOGE(TAG, "Failed to init ui queue");
    }
    ESP_ERROR_CHECK(setupRotaryEncoderInput());

    MPUIEvent ev;
    uint8_t idleSent = 0;

    while(true) {
        if (xQueueReceive(g_mpuiQueue, &ev, IdleDelay)) {
            //handle the EV
            idleSent = 0;
        }
        else {
            if (!idleSent) {
                //send simulated idle event
            };
        }
    }
}



esp_err_t setupRotaryEncoderInput()
{

    return ESP_OK;
}


void defaultMPUIEventHandler(MPUIEvent ev) {

}