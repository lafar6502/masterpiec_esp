#ifndef _MASTERP_TEMP_SENSORS_H_INCLUDED_
#define _MASTERP_TEMP_SENSORS_H_INCLUDED_

#define TSENS_BOILER 0 //temp pieca
#define TSENS_CWU 1    //temp bojlera cwu
#define TSENS_FEEDER 2 //temp podajn.
#define TSENS_RETURN 3  //temp powrotu
#define TSENS_EXTERNAL 4  //temp zewnetrzna
#define TSENS_CWU2 5  //temp user 1
#define TSENS_USR1 6  //temp user 2 
#define TSENS_USR2 7  //temp user 3

#define MAX_THERMOCOUPLES 2
#define T2SENS_EXHAUST 0  //temp. spalin
#define T2SENS_BURNER  1  //temp. palnika


void initDallasSensors();

void scanOneWire();
void initThermocoupleSensors();

//request conversion on all dallas sensors
void requestDataRead();
//read temperature from sensors (without waiting for conversion)
//first call requestDataRead
void readTemperaturesFromSensors();

float getDallasTemp(uint8_t idx);
float getThermocoupleTemp(uint8_t idx);


#endif