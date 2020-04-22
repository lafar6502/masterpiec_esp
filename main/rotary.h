#ifndef _MPIEC_ROTARY_H_INCLUDED
#define _MPIEC_ROTARY_H_INCLUDED

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20

#define ROT_BTN_PRESS 0x05
#define ROT_BTN_RELEASE 0x06


unsigned char rotary_process(unsigned char pinA, unsigned char pinB);

int8_t read_rotary(unsigned char pinA, unsigned char pinB);
int8_t read_rotary_2(unsigned char pinA, unsigned char pinB);
#endif