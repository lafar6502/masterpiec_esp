#ifndef _MASTERP_TEMP_SENSORS_H_INCLUDED_
#define _MASTERP_TEMP_SENSORS_H_INCLUDED_

void initDallasSensors();

void scanOneWire();
void initThermocoupleSensors();

//request conversion on all dallas sensors
void requestDataRead();
//read temperature from sensors (without waiting for conversion)
//first call requestDataRead
void readTemperaturesFromSensors();

void getDallasTemp(uint8_t idx);
void getThermocoupleTemp(uint8_t idx);


#endif