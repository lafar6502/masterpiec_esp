#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "ui_control.h"
#include "rotary.h"

//ESP_EVENT_DEFINE_BASE(MP_UI_EVENT);
static const char* TAG = "ui";

const TickType_t IdleDelay = 5000 / portTICK_PERIOD_MS;





#define ROT_ENC_A_GPIO (CONFIG_ROT_ENC_A_GPIO)
#define ROT_ENC_B_GPIO (CONFIG_ROT_ENC_B_GPIO)
#define ROT_BTN_GPIO (CONFIG_ROT_ENC_BTN_GPIO)
#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded

QueueHandle_t g_rotaryQueue;
int g_position = 0;

static void rotary_encoder_isr(void * args)
{
    static uint32_t cnt=0;
    static uint64_t lastIntr = 0;
    static uint8_t  lastPin = 0;
    uint64_t us = esp_timer_get_time();
    uint8_t pin = (uint8_t) args;
    
    if (us - lastIntr < 1000 && lastPin == pin) {
        return;
    }

    lastIntr = us;
    lastPin = pin;
    cnt++;
    if (pin == ROT_BTN_GPIO) {
        //handle btn
        return;
    }

    MPUIEvent ev = {MPUI_DOWN, 0};
    unsigned char pina = gpio_get_level(ROT_ENC_A_GPIO) ? 1 : 0;
    unsigned char pinb = gpio_get_level(ROT_ENC_B_GPIO) ? 1 : 0;
    
    int8_t dir = read_rotary(pina, pinb);
    int n = gpio_get_level(CONFIG_DEBUG_OUT_GPIO);
    if (dir != 0) gpio_set_level(CONFIG_DEBUG_OUT_GPIO, n ? 0 : 1);
    
    if (dir == 0) {
        //ESP_LOGD(TAG, "rotary event, no movement");
        return;
    }
    if (dir == 1) {
        //ESP_LOGD(TAG, "ROT cw");
        g_position++;
        ev.Type = MPUI_DOWN;
    }
    else if (dir == -1) {
        //ESP_LOGD(TAG, "ROT ccw");
        g_position--;
        ev.Type = MPUI_UP;
    }
    else {
        //error
        return;
    }
    
    ev.Position = g_position;
    if (ev.Type != MPUI_NONE) 
    {
        BaseType_t task_woken = pdFALSE;
        xQueueSendToBackFromISR(g_rotaryQueue, &ev, &task_woken);
        if (task_woken)
        {
            portYIELD_FROM_ISR();
        }
    }
}

esp_err_t initializeRotary() 
{
    ESP_LOGD(TAG, "Initializing rotary pin A=%d, pin B=%d, btn=%d", (int) ROT_ENC_A_GPIO, (int) ROT_ENC_B_GPIO, (int) ROT_BTN_GPIO);
    
    ESP_LOGD(TAG, "Gpio inited");

    uint8_t pinz[] = {ROT_ENC_A_GPIO, ROT_ENC_B_GPIO, ROT_BTN_GPIO};
    for(int i=0; i<sizeof(pinz); i++) 
    {
        gpio_pad_select_gpio(pinz[i]);
        gpio_set_pull_mode(pinz[i], GPIO_PULLUP_ONLY);
        gpio_set_direction(pinz[i], GPIO_MODE_INPUT);
        gpio_set_intr_type(pinz[i], GPIO_INTR_ANYEDGE);

        ESP_LOGD(TAG, "init isr %d", pinz[i]);
        esp_err_t res = gpio_isr_handler_add(pinz[i], rotary_encoder_isr, (void*) pinz[i]);
        if (res != ESP_OK) {
            ESP_LOGE(TAG, "error initializing isr  for rotary 0x%x", res);
            return res;
        };
    }

    ESP_LOGD(TAG, "ISR Gpio inited");

    return ESP_OK;   

}

void mpuiHandlerTask(void * pvParameters) 
{
    ESP_LOGD(TAG, "creating event queue");
    g_rotaryQueue = xQueueCreate(20, sizeof(MPUIEvent));
    if (g_rotaryQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create rot queue");
        return;
    }
    ESP_LOGD(TAG, "configuring rotary");
    esp_err_t res = initializeRotary();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init rotary %x", res);
        return;
    }
    
    ESP_LOGD(TAG, "Starting event queue");
    uint8_t idleSent = 0;

    MPUIEvent event;;
        
    while (1)
    {
        // Wait for incoming events on the event queue.
        if (xQueueReceive(g_rotaryQueue, &event, IdleDelay) == pdTRUE)
        {
            ESP_LOGD(TAG, "Got rotary event %d, cnt %d", (int) event.Type, event.Position);
            //unsigned char pina = gpio_get_level(ROT_ENC_A_GPIO) ? 1 : 0;
            //unsigned char pinb = gpio_get_level(ROT_ENC_B_GPIO) ? 1 : 0;
            //unsigned char dir = rotary_process(pina, pinb);
            
        }
        else
        {
            ESP_LOGD(TAG, "Queue idle");
            // Poll current position and direction
            /*rotary_encoder_state_t state = { 0 };
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            ESP_LOGI(TAG, "ENC Poll: position %d, direction %s", state.position,
                     state.direction ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");

            // Reset the device
            if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
            {
                ESP_LOGI(TAG, "ENC Reset");
                ESP_ERROR_CHECK(rotary_encoder_reset(&info));
            }*/
            if (!idleSent) {
                //send simulated idle event
                idleSent = 1;
                //ev.Type = MPUI_IDLE;
                //defaultMPUIEventHandler(ev);                
            };
        }
    }
    
}


esp_err_t initializeMPUI() {
    TaskHandle_t xHandle = NULL;
    BaseType_t res = xTaskCreate(mpuiHandlerTask, "UI", 2048, NULL, tskIDLE_PRIORITY, &xHandle );
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ui task %x", res);
        return ESP_OK;
    }
    ESP_LOGD(TAG, "Created ui task");
    return ESP_OK;

}

//#define FLIP_DIRECTION    0  // Set to true to reverse the clockwise/counterclockwise sense

esp_err_t setupRotaryEncoderInput()
{
    
    return ESP_OK;
}


void defaultMPUIEventHandler(MPUIEvent ev) {

}