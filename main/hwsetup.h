#ifndef _HWSETUP_H_INCLUDED_
#define _HWSETUP_H_INCLUDED_



//B (digital pin 8 to 13)
//C (analog input pins)
//D (digital pins 0 to 7)

//82  PK7 ( ADC15/PCINT23 ) Analog pin 15
//83  PK6 ( ADC14/PCINT22 ) Analog pin 14
//84  PK5 ( ADC13/PCINT21 ) Analog pin 13
//85  PK4 ( ADC12/PCINT20 ) Analog pin 12
//86  PK3 ( ADC11/PCINT19 ) Analog pin 11
//87  PK2 ( ADC10/PCINT18 ) Analog pin 10
//88  PK1 ( ADC9/PCINT17 )  Analog pin 9
//89  PK0 ( ADC8/PCINT16 )  Analog pin 8


#define HW_ENCODER_PINA 15
#define HW_ENCODER_PINB 18
#define HW_ENCODER_PINBTN 34
#define HW_ZERO_DETECT_PIN 2

//#define MPIEC_ENABLE_SCRIPT
#define ONEWIRE_PIN GPIO_NUM_25
#define MAX6675_CS_PIN GPIO_NUM_4

#endif
