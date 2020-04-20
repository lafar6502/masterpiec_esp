#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#include "ui_control.h"

//ESP_EVENT_DEFINE_BASE(MP_UI_EVENT);



void setupRotaryEncoderInput()
{

}


static void all_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "%s:%s: all_event_handler", base, get_id_string(base, id));
}


QueueHandle_t g_mpuiQueue;

void setupUIEventQueue() 
{
    g_mpuiQueue = xQueueCreate(10, sizeof(MPUIEvent));
    
    if (g_mpuiQueue == NULL) {
        //fatal error
    }


}



