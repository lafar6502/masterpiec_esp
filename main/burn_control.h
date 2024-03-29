#ifndef _BURN_CONTROL_H_INCLUDED_
#define _BURN_CONTROL_H_INCLUDED_

typedef uint8_t TSTATE;

#define STATE_P0 0   //podtrzymanie
#define STATE_P1 1   //grzanie z mocą minimalną
#define STATE_P2 2   //grzanie z mocą nominalną
#define STATE_STOP 3 //tryb ręczny - zatrzymany piec - sterowanie automatyczne powinno zaprzestać działalności 
#define STATE_ALARM 4 // alarm - cos się stało, piec zatrzymany albo włączone zabezpieczenie
#define STATE_REDUCE1 5 //tryb przejścia na niższy stan P2 => P1 => P0. zadaniem tego trybu jest dopalenie pozostałego węgla. W tym celu musimy wiedzieć z jakiego stanu wyszlismy do reduce
#define STATE_REDUCE2 6 //tryb przejścia na niższy stan P1 => P0 
#define STATE_UNDEFINED 255

#define MAX_POWER_STATES 3 //max liczba konfiguracji dla mocy. 1 - tylko podtrzymanie, 2 - podtrzymanie i praca, 3 - podtrzymanie i 2 moce pracy

typedef uint8_t CWSTATE;

#define CWSTATE_OK 1
#define CWSTATE_HEAT 2

#define NEED_HEAT_NONE 0
#define NEED_HEAT_CO   1
#define NEED_HEAT_CWU  2

#define COOLOFF_NONE 0
#define COOLOFF_OVERHEAT 1 //first mode - run cooloff only when above set temp + delta
#define COOLOFF_LOWER 2 //second mode- run cooloff while above the set temp

typedef uint8_t HEATNEED;

//konfiguracja jednego z poziomów mocy
typedef struct BurnParams {
    uint16_t CycleSec; //total cycle, including the fueling time, in seconds. podtrzymanie: okres przedmuchu. podawanie wegla co 1, 2, 3 przedmuchy
    //czas podawania wegla, * 10 (50 = 5 sekund)
    uint16_t FuelSecT10;
    //moc nadmuchu
    uint8_t BlowerPower;
    uint8_t BlowerCycle; //cykl dmuchawy dla zasilania grupowego. 0 gdy fazowe.
} TBurnParams;


//zestaw ustawień pieca (aktualna konfiguracja). Nie zawiera bieżących wartości.
typedef struct ControlConfiguration {
  uint16_t Magic; //should always be 0x6502
  uint8_t TCO; //co temp 
  uint8_t TCWU;  //cwu temp
  uint8_t TCWU2; //cwu2 temp
  uint8_t TMinPomp; //minimum pump run temp.
  uint8_t THistCwu; //histereza cwu
  uint8_t THistCO;  //histereza co
  uint8_t TDeltaCO; //delta co - temp powyzej zadanej przy ktorej przejdzie w podtrzymanie
  uint8_t TDeltaCWU; //delta cwu - temp powyżej bojlera do ktorej rozgrzewamy piec
  uint8_t P0BlowerTime; //czas pracy dmuchawy w podtrzymaniu
  uint8_t P0FuelFreq; //podawanie wegla co x cykli przedmuchu
  uint8_t    _HomeThermostat;
  uint8_t   SummerMode; //tryb letni
  
  TBurnParams BurnConfigs[MAX_POWER_STATES]; //first one [0] is the podtrzymanie
  uint8_t DallasAddress[8][8]; //dallas sensor addresses. if zero - sensor not present
  uint8_t DefaultBlowerCycle;
  uint8_t FeederTempLimit;
  uint8_t NoHeatAlarmCycles; //time needed to deterimine if we have the fire
  uint8_t EnableThermostat; //0 or 1
  uint8_t CooloffTimeM10; //minutes * 10
  uint16_t CooloffPauseM10; //minutes * 10
  uint16_t FuelGrH; //fuel grams per hour of feeder work. 10 kg=10000. 
  uint16_t FuelHeatValueMJ10; //fuel heat in MJ, * 10 (100 = 10MJ)
  uint8_t  CooloffMode; //0 - none
  int8_t  FuelCorrection; //0 - none, fuel feed correction % x 20 (100 = + 20%, -100 = -20%)
  uint8_t CircCycleMin; //60, 30, 15, 10, 6
  uint8_t CircWorkTimeS; //circ pump working time per cycle, sec*10 (10 = 100 sec)
  uint8_t ReductionP2ExtraTime; //in %, how much % of the P2 cycle time to add for reduction (0 = just the P2 cycle, 10 = P2 cycle + 10%)
  uint8_t BlowerMax; //Blower max value that will be our 100
  uint8_t _future[9];
} TControlConfiguration;

//bieżąca konfiguracja pieca
extern TControlConfiguration g_CurrentConfig;

typedef struct BurnTransition {
  TSTATE From;
  TSTATE To;
  uint8_t (*fCondition)();
  void (*fAction)(int); //action executed on transition, before state change. int param = transition index in the table
} TBurnTransition;

typedef struct BurnStateConfig {
  TSTATE State;
  char Code;
  void (*fInitialize)(TSTATE s);
  void (*fLoop)();
} TBurnStateConfig;



typedef struct {
  unsigned long Ms;
  float Val;
} TReading;

typedef struct {
  unsigned long Ms;
  uint16_t Val;
} TIntReading;


//initialize the burning automatic algorithm
void initializeBurningLoop();
//periodically run burn control task to handle automatic burning alg.
void burnControlTask();

///functions additional to main boiler control loop
// f.ex. circulation pump
void circulationControlTask();

//wlacz/wylacz tryb automatyczny
void setManualControlMode(uint8_t b);
//czy mamy tryb automatyczny
uint8_t getManualControlMode();
//check if heating is needed anywhere (for home or for cwu). 0 - no, 1 - co needed, 2 - cwu needed
HEATNEED needHeatingNow();

///sets alarm state and shows alarm message
void setAlarm(const char* txt);

void forceState(TSTATE st);

extern const TBurnTransition  BURN_TRANSITIONS[];
extern const TBurnStateConfig BURN_STATES[];
extern const uint8_t N_BURN_TRANSITIONS;
extern const uint8_t N_BURN_STATES;
extern TSTATE g_BurnState;
extern CWSTATE g_CWState;
extern HEATNEED g_needHeat; //status variable: we need heating now (=cwu or co).
//czas wejscia w bieżący BURN stan, ms
extern unsigned long g_CurStateStart;
extern float  g_CurStateStartTempCO; //temp pieca w momencie wejscia w bież. stan.
extern unsigned long g_CurBurnCycleStart; //timestamp, w ms, w ktorym rozpoczelismy akt. cykl palenia

//
// Globalne zmienne reprezentujące bieżący stan pieca, temperatury, dmuchawy itp
// po to żeby np moduł UI mógł sobie je wyswietlać. 
//

extern float g_TargetTemp; //aktualnie zadana temperatura pieca (która może być wyższa od temp. zadanej w konfiguracji bo np grzejemy CWU)
extern float g_TempCO;
extern float g_TempCWU; 
extern float g_TempPowrot;  //akt. temp. powrotu
extern float g_TempSpaliny; //akt. temp. spalin
extern float g_TempFeeder;
extern float g_TempBurner; //temp palnika
extern float g_dT60; //delta temp in 1 minute
extern float g_dTl3; //delta temp in last 3 changes
extern uint8_t g_ReductionsToP0; //reductions P1 -> P0 or P2 -> P0 which we dont ave 
extern uint8_t g_ReductionsToP1; //reductions P2 -> P1

extern TSTATE g_BurnState;  //aktualny stan grzania
extern uint8_t   g_HomeThermostatOn;  //true - termostat pokojowy kazał zaprzestać grzania
extern unsigned long g_P1Time; //accumulated P1 time
extern unsigned long g_P2Time; //accumulated P2 time
extern unsigned long g_P0Time; //accumulated P0 time

extern float g_TempZewn; //aktualna temp. zewn
extern char* g_Alarm;

#endif
