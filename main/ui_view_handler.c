#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ui_control.h"
#include "ui_view_handler.h"
#include "ui_screen.h"

#define DISPLAY_TEXT_LINES 4
#define DISPLAY_TEXT_LEN   20

uint8_t g_CurrentUIState;
uint8_t g_CurrentUIView;
char g_displ[DISPLAY_TEXT_LINES][DISPLAY_TEXT_LEN + 1];
char* g_DisplayBuf[DISPLAY_TEXT_LINES];

void clearDipslayBuf() {
  memset(g_displ, 0, sizeof(g_displ));
  for(int i=0; i<DISPLAY_TEXT_LINES; i++) {
    g_DisplayBuf[i] = ((char*) g_displ) + (i * (DISPLAY_TEXT_LEN + 1));
  }
}

void initializeDisplay() {
  clearDipslayBuf();
}



void eraseDisplayToEnd(char* buf) {
  bool f = false;
  for(int i=0; i<=DISPLAY_TEXT_LEN; i++) 
  {
    if (f) 
      buf[i] = ' ';
    else if (buf[i] == '\0') {
      f = true;
      buf[i] = ' ';
    }
  }
  buf[DISPLAY_TEXT_LEN] = 0;
}

void processUIEvent(MPUIEvent* ev) 
{
  uint8_t cs = g_CurrentUIState;
  uint8_t cv = g_CurrentUIView;
  
  if (ev->Type != MPUI_IDLE) 
  {
    //_hbCountSinceLastEvent = 0;
    //_idleReported = false;
  }

  //assert(g_CurrentUIState >= 0 && g_CurrentUIState < sizeof(UI_STATES) / sizeof(TUIStateEntry));

  if (UI_STATES[g_CurrentUIState].HandleEvent != NULL) UI_STATES[g_CurrentUIState].HandleEvent(ev);

  if (cs != g_CurrentUIState || cv != g_CurrentUIView) 
  {
  }
  
}

void updateView() {
  
  //assert(g_CurrentUIState >= 0 && g_CurrentUIState < sizeof(UI_STATES) / sizeof(TUIStateEntry));
  if (UI_STATES[g_CurrentUIState].UpdateView != NULL) 
  {
    clearDipslayBuf();
    UI_STATES[g_CurrentUIState].UpdateView();
  }
  else 
  {
    if (UI_SCREENS[g_CurrentUIView].UpdateView != NULL) 
    {
      clearDipslayBuf();
      UI_SCREENS[g_CurrentUIView].UpdateView(g_CurrentUIView, g_DisplayBuf);
      eraseDisplayToEnd(g_DisplayBuf[0]);
      eraseDisplayToEnd(g_DisplayBuf[1]);
    }
  }
}

void changeUIState(char code) {
  uint8_t oldState = g_CurrentUIState;
  for(int i=0; i<N_UI_STATES; i++) {
    if (UI_STATES[i].Code == code) {
      g_CurrentUIState = i;
      g_CurrentUIView = UI_STATES[i].DefaultView;
      MPUIEvent ev;
      ev.Type = MPUI_NONE;
      if (UI_STATES[g_CurrentUIState].HandleEvent != NULL) UI_STATES[g_CurrentUIState].HandleEvent(&ev);//, oldState);

      return;
    }
  }
}


void user_input_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

}

///rtos task for updating the UI
void uiUpdateTask() 
{
  ESP_ERROR_CHECK(esp_event_handler_register(MPUI_EVENT, ESP_EVENT_ANY_ID, user_input_event_handler, NULL));
}