#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "ui_control.h"
#include "rotary.h"


static const char* TAG = "ui";

const TickType_t IdleDelay = 5000 / portTICK_PERIOD_MS;





#define ROT_ENC_A_GPIO (CONFIG_ROT_ENC_A_GPIO)
#define ROT_ENC_B_GPIO (CONFIG_ROT_ENC_B_GPIO)
#define ROT_BTN_GPIO (CONFIG_ROT_ENC_BTN_GPIO)


QueueHandle_t g_rotaryQueue;
esp_timer_handle_t g_rotBtnTimer;
esp_timer_handle_t g_rotEncTimer;

ESP_EVENT_DEFINE_BASE(MPUI_EVENT);

void button_delayed_handler(void* p) 
{
    static uint64_t lastPress = 0;
    uint64_t us = esp_timer_get_time();
    
    uint8_t bstate = gpio_get_level(ROT_BTN_GPIO);
    MPUIEvent ev = {MPUI_NONE, 0, us};
    uint64_t prtime = us < lastPress ? lastPress - us : us - lastPress;   
    ESP_LOGD(TAG, "Button delayed handler, elaps %ld, b: %d", (long int) prtime, (int) bstate);
    
    if (bstate == 0) 
    {   //pressed
        lastPress = us;
        ev.Type = MPUI_BTNDOWN;
        xQueueSend(g_rotaryQueue, &ev, 0);
        
    } 
    else 
    { //released
        
        ev.Type = MPUI_BTNUP;
        xQueueSend(g_rotaryQueue, &ev, 0);
        if (lastPress != 0)
        {
            ev.Type = prtime < 3000 * 1000 ? MPUI_BTNPRESS : MPUI_BTNLONGPRESS;
            xQueueSend(g_rotaryQueue, &ev, 0);
        }
        lastPress = 0;
    }
}



int g_position = 0;


static void rotary_btn_isr(void* args) {
    static uint64_t lastIntr = 0;
    uint64_t us = esp_timer_get_time();
    
    if (us - lastIntr < 3000) {
        return;
    }
    
    lastIntr = us;
    esp_err_t res = esp_timer_start_once(g_rotBtnTimer, 3 * 1000); //3ms
    if (res != ESP_OK) 
    {

    }

}

void rotary_encoder_delayed_handler(void *p) {
    static uint32_t cnt=0;
    uint64_t us = esp_timer_get_time();
    cnt++;

    MPUIEvent ev = {MPUI_NONE, 0, us};
    
    unsigned char pina = gpio_get_level(ROT_ENC_A_GPIO) ? 1 : 0;
    unsigned char pinb = gpio_get_level(ROT_ENC_B_GPIO) ? 1 : 0;
    
    int8_t dir = read_rotary_2(pina, pinb);
    int n = gpio_get_level(CONFIG_DEBUG_OUT_GPIO);
    if (dir != 0) gpio_set_level(CONFIG_DEBUG_OUT_GPIO, n ? 0 : 1);
    //dir = 1;
    if (dir == 0) {
        //ESP_LOGD(TAG, "ROT none");
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
    //ev.Position = cnt;
    //ev.Type = pina << 1 | pinb;
    if (ev.Type != MPUI_NONE) 
    {
        xQueueSend(g_rotaryQueue, &ev, 0);
    }
}

static void rotary_encoder_isr(void * args)
{
    static uint32_t cnt=0;
    static uint64_t lastIntr = 0;
    static uint8_t  lastPin = 0;
    uint64_t us = esp_timer_get_time();
    uint8_t pin = (uint8_t) args;
    
    if (us - lastIntr < 1500 && lastPin == pin) {
        return;
    }

    lastIntr = us;
    lastPin = pin;
    cnt++;
    
    esp_err_t res = esp_timer_start_once(g_rotEncTimer, 3 * 1000); //3ms
    if (res != ESP_OK) 
    {

    }

}

esp_err_t initializeRotary() 
{
    ESP_LOGD(TAG, "Initializing rotary pin A=%d, pin B=%d, btn=%d", (int) ROT_ENC_A_GPIO, (int) ROT_ENC_B_GPIO, (int) ROT_BTN_GPIO);
    

    uint8_t pinz[] = {ROT_ENC_A_GPIO, ROT_ENC_B_GPIO};
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
    
    gpio_pad_select_gpio(ROT_BTN_GPIO);
    gpio_set_pull_mode(ROT_BTN_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_direction(ROT_BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(ROT_BTN_GPIO, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(ROT_BTN_GPIO, rotary_btn_isr, NULL);

    ESP_LOGD(TAG, "Rotary gpio inited");

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
    
    esp_timer_create_args_t tArgs;
    tArgs.callback = button_delayed_handler;
    tArgs.dispatch_method = ESP_TIMER_TASK;
    esp_err_t res = esp_timer_create(&tArgs, &g_rotBtnTimer);
    if (res != ESP_OK) {
        printf("timer 1 init fail");
        return;
    }

    tArgs.callback = rotary_encoder_delayed_handler;
    tArgs.dispatch_method = ESP_TIMER_TASK;
    res = esp_timer_create(&tArgs, &g_rotEncTimer);
    if (res != ESP_OK) {
        printf("timer 2 init fail");
        return;
    }
    

    ESP_LOGD(TAG, "configuring rotary");
    res = initializeRotary();
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init rotary %x", res);
        return;
    }
    
    ESP_LOGD(TAG, "Starting event queue");
    uint8_t idleSent = 0;

    MPUIEvent event;
    int pos = 0;
    unsigned char pina, pinb;
    while (1)
    {
        // Wait for incoming events on the event queue.
        if (xQueueReceive(g_rotaryQueue, &event, IdleDelay) == pdTRUE)
        {
            idleSent = 0;
            switch(event.Type) 
            {
                case MPUI_BTNDOWN:
                case MPUI_BTNUP: 
                    break;
                case MPUI_BTNPRESS:
                    ESP_LOGD(TAG, "BTN press");
                    break;
                case MPUI_BTNLONGPRESS:
                    ESP_LOGD(TAG, "BTN long press");
                    break;
                case MPUI_UP:
                case MPUI_DOWN:
                    
                    ESP_LOGD(TAG, "Got rotary event %d, pos %d, delay %d", (int) event.Type, event.Position, (int) (esp_timer_get_time() - event.Us));

                    break;
                default:
                    break;
            }
            defaultMPUIEventHandler(event);
        }
        else
        {
            ESP_LOGD(TAG, "Queue idle");

            if (!idleSent) {
                //send simulated idle event
                idleSent = 1;
                event.Type = MPUI_IDLE;
                defaultMPUIEventHandler(event);                
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



void defaultMPUIEventHandler(MPUIEvent ev) {
    esp_err_t res = esp_event_post(MPUI_EVENT, ev.Type, &ev, sizeof(MPUIEvent), 1);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "ui post fail %d", res);
    }
}