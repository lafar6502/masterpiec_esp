#ifndef _MASTERPIEC_UI_CONTROL_H_INCLUDED_
#define _MASTERPIEC_UI_CONTROL_H_INCLUDED_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//ui events, ui event queue, handling of rotary input

//ESP_EVENT_DECLARE_BASE(MPUI_EVENT);


typedef enum UI_EVENT_TYPE {
    MPUI_NONE,
    MPUI_UP,
    MPUI_DOWN,
    MPUI_BTNPRESS,
    MPUI_BTNLONGPRESS,
    MPUI_IDLE
    
}  UI_EVENT_TYPE;

typedef struct MPUIEvent {
    UI_EVENT_TYPE Type;
    int Position;
    uint64_t Us;
}  MPUIEvent;

extern QueueHandle_t g_mpuiQueue;


esp_err_t setupRotaryEncoderInput();

//how does the event queue work
//we have rotary encoder/button handling (on interrupts) where the handlers
//post events to a queue.
//the queue is handled by ui handling task
void mpuiHandlerTask(void*);


esp_err_t initializeMPUI();

void defaultMPUIEventHandler(MPUIEvent ev);

#endif