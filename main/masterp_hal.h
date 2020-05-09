#ifndef _MASTERP_HAL_INCLUDED_
#define _MASTERP_HAL_INCLUDED_
#include <stdint.h>


void setPumpOn(uint8_t num);
void setPumpOff(uint8_t num);
bool isPumpOn(uint8_t num);
bool isPumpEnabled(uint8_t num);

//uruchomienie podajnika
void setFeederOn();
//zatrzymanie podajnika
void setFeederOff();
void setFeeder(bool b);
//czy podajnik działa
bool isFeederOn();

void setBlowerPower(uint8_t power);
void setBlowerPowerWithCycle(uint8_t power, uint8_t powerCycle);
uint8_t getCurrentBlowerPower();
uint8_t getCurrentBlowerCycle();

uint8_t isThermostatOn();
#endif