#include <stdlib.h>
#include <stdint.h>
#include "masterp_hal.h"
#include "burn_control.h"
#include "temp_sensors.h"


#define MAX_TEMP 90


void initializeBurningLoop() {
  g_TargetTemp = g_CurrentConfig.TCO;
  g_HomeThermostatOn = 1;
  forceState(STATE_P0);
}


float g_TargetTemp = 0.1; //aktualnie zadana temperatura pieca (która może być wyższa od temp. zadanej w konfiguracji bo np grzejemy CWU)
float g_TempCO = 0.0;
float g_TempCWU = 0.0; 
float g_TempPowrot = 0.0;  //akt. temp. powrotu
float g_TempSpaliny = 0.0; //akt. temp. spalin
float g_TempFeeder = 0.1;
float g_TempBurner = 0;
TSTATE g_BurnState = STATE_UNDEFINED;  //aktualny stan grzania
CWSTATE g_CWState = CWSTATE_OK; //current cw status
HEATNEED g_needHeat = NEED_HEAT_NONE; //0, 1 or 2
HEATNEED g_initialNeedHeat = NEED_HEAT_NONE; //heat needs at the beginning of current state

uint8_t   g_HomeThermostatOn = 1;  //true - termostat pokojowy kazał zaprzestać grzania
float g_TempZewn = 0.0; //aktualna temp. zewn
char* g_Alarm;
unsigned long g_P1Time = 0;
unsigned long g_P2Time = 0;
unsigned long g_P0Time = 0;

//czas wejscia w bieżący stan, ms
unsigned long g_CurStateStart = 0;
//float  g_CurStateStartTempCO = 0; //temp pieca w momencie wejscia w bież. stan.
uint8_t g_BurnCyclesBelowMinTemp = 0; //number of burn cycles with g_TempCO below minimum pump temperature (for detecting extinction of fire)
unsigned long g_CurBurnCycleStart = 0; //timestamp, w ms, w ktorym rozpoczelismy akt. cykl palenia (ten podajnik+nadmuch)


void setAlarm(const char* txt) {
  if (txt != NULL) g_Alarm = txt;
  forceState(STATE_ALARM);
  
}

float g_dT60; //1-minute temp delta
float g_dTl3; //last 3 readings diff

float interpolate(TReading r1, TReading r2, unsigned long t0) {
  float tdT = r2.Val - r1.Val;
  unsigned long tm0 = r2.Ms - r1.Ms;
  //printf("interpolate (%ld,%f), (%ld, %f) at %ld: dt is %ld\r\n", r1.Ms, r1.Val, r2.Ms, r2.Val, t0, tm0);
  if (tm0==0) tm0 = 1;
  float tx = r1.Val + tdT * (t0 - r1.Ms) / tm0; // (tdT / tm0) * ((t0 - r1.Ms) / tm0);
  return tx;
}

float calcDT60() {
    /*unsigned long m  = millis();
    unsigned long m2 = m - 60 * 1000;
    TReading r2 {m, g_TempCO};
    int i;
    for (i = g_lastCOReads.GetCount() - 1; i >= 0; i--) {
        if (g_lastCOReads.GetAt(i)->Ms < m2) break;
    }
    if (i >= 0) {
        //printf("found reading at %d ms=%d, v=%f\r\n", i, g_reads.GetAt(i)->Ms, g_reads.GetAt(i)->Val);
        const TReading& r1 = *g_lastCOReads.GetAt(i);
        if (i < g_lastCOReads.GetCount() - 1) {
          r2 = *g_lastCOReads.GetAt(i + 1); //next read
        }
        float newPoint = interpolate(r1, r2, m2);
        float dx = (g_TempCO - newPoint);// * 1000 / (m - m2);
        //now interpolate btw r and r2
        return dx;
    }
    else {
        //printf("did not find reading");
        return 0;
    }
    */
   return 0;
}

void processSensorValues() {
  g_TempCO = getDallasTemp(TSENS_BOILER);
  g_TempCWU = getDallasTemp(TSENS_CWU);
  g_TempPowrot = getDallasTemp(TSENS_RETURN);
  g_TempFeeder = getDallasTemp(TSENS_FEEDER);
  g_TempZewn = getDallasTemp(TSENS_EXTERNAL);
  g_TempSpaliny = getThermocoupleTemp(T2SENS_EXHAUST);
  g_TempBurner = getThermocoupleTemp(T2SENS_BURNER);
  if (g_CurrentConfig.EnableThermostat) 
  {
    g_HomeThermostatOn = isThermostatOn();
  }
  //unsigned long ms = millis();
  /*if (g_lastCOReads.IsEmpty() || abs(g_lastCOReads.GetLast()->Val - g_TempCO) >= 0.5 || g_lastCOReads.GetLast()->Ms < (ms - 120000L)) 
  {
    g_lastCOReads.Enqueue({ms, g_TempCO});
  }
  g_dT60 = calcDT60();
  TReading* pr = g_lastCOReads.GetCount() >= 4 ? g_lastCOReads.GetAt(-3) : NULL; //discard the first read
  if (g_lastCOReads.GetFirst()->Ms > (ms - 15000L)) pr = NULL;
  g_dTl3 = pr != NULL ? (g_TempCO - pr->Val) * 60.0 * 1000.0 / (ms - pr->Ms) : 0.0;
  */
}
