#ifndef _MASTERPIEC_UI_CONTROL_H_INCLUDED_
#define _MASTERPIEC_UI_CONTROL_H_INCLUDED_


#include "esp_event.h"
#include "esp_timer.h"

//ui events, ui event queue, handling of rotary input

ESP_EVENT_DECLARE_BASE(MPUI_EVENT); //for events of type MPUIEvent declared below


typedef enum UI_EVENT_TYPE {
    MPUI_NONE,
    MPUI_UP,
    MPUI_DOWN,
    MPUI_BTNDOWN, //button down
    MPUI_BTNUP,   //button released
    MPUI_BTNPRESS, //button down+up, short press
    MPUI_BTNLONGPRESS, //button down+up, long press

    MPUI_IDLE
    
}  UI_EVENT_TYPE;

typedef struct MPUIEvent {
    UI_EVENT_TYPE Type;
    int Position;
    uint64_t Us;
}  MPUIEvent;



//how does the event queue work
//we have rotary encoder/button handling (on interrupts) where the handlers
//post events to a queue.
//the queue is handled by ui handling task
void mpuiHandlerTask(void*);


esp_err_t initializeMPUI();

void defaultMPUIEventHandler(MPUIEvent ev);

//return current encoder position
int32_t getEncoderPos();
//check if enc button is pressed now
uint8_t isEncoderButtonPressed();


#endif