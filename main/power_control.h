#ifndef _MASTERPIEC_POWER_CTRL_H_INCLUDED_
#define _MASTERPIEC_POWER_CTRL_H_INCLUDED_
#include <stdint.h>

void handleZeroCross();


void setPowerOutState(uint8_t chan, uint8_t on);
uint8_t getPowerOutState(uint8_t chan);
void initPowerControlModule();

extern uint32_t g_powerCycles;


#endif
