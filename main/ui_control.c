#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <esp_log.h>

#include "ui_control.h"
#include "rotary_encoder.h"

//ESP_EVENT_DEFINE_BASE(MP_UI_EVENT);
static const char* TAG = "ui";

const TickType_t IdleDelay = 5000 / portTICK_PERIOD_MS;






QueueHandle_t g_mpuiQueue;

void mpuiHandlerTask(void * pvParameters) 
{
    g_mpuiQueue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, g_mpuiQueue));
    
    ESP_ERROR_CHECK(setupRotaryEncoderInput());

    ESP_LOGD(TAG, "Starting event queue");
    uint8_t idleSent = 0;

    rotary_encoder_event_t event = { 0 };
        
    while (1)
    {
        // Wait for incoming events on the event queue.
        if (xQueueReceive(event_queue, &event, IdleDelay) == pdTRUE)
        {
            ESP_LOGI(TAG, "ENC Event: position %d, direction %s", event.state.position,
                     event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
            idleSent = 0;
        }
        else
        {
            // Poll current position and direction
            rotary_encoder_state_t state = { 0 };
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            ESP_LOGI(TAG, "ENC Poll: position %d, direction %s", state.position,
                     state.direction ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");

            // Reset the device
            if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
            {
                ESP_LOGI(TAG, "ENC Reset");
                ESP_ERROR_CHECK(rotary_encoder_reset(&info));
            }
            if (!idleSent) {
                //send simulated idle event
                idleSent = 1;
                ev.Type = MPUI_IDLE;
                //defaultMPUIEventHandler(ev);                
            };
        }
    }
    
}


esp_err_t initializeMPUI() {
    TaskHandle_t xHandle = NULL;
    BaseType_t res = xTaskCreate(mpuiHandlerTask, "UI", 1000, NULL, tskIDLE_PRIORITY, &xHandle );
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ui task %x", res);
        return ESP_OK;
    }
    ESP_LOGD(TAG, "Created ui task");
    return ESP_OK;

}

#define ROT_ENC_A_GPIO (CONFIG_ROT_ENC_A_GPIO)
#define ROT_ENC_B_GPIO (CONFIG_ROT_ENC_B_GPIO)
#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded
//#define FLIP_DIRECTION    0  // Set to true to reverse the clockwise/counterclockwise sense

esp_err_t setupRotaryEncoderInput()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    rotary_encoder_info_t info = { 0 };
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif
    
    return ESP_OK;
}


void defaultMPUIEventHandler(MPUIEvent ev) {

}