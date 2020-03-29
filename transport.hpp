//----------------------------------------------------------------------------

//Модуль управления ЛПМ, заголовочный файл

//----------------------------------------------------------------------------

#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP

//----------------------------------------------------------------------------

#include "systimer.hpp"
#include "solenoid.hpp"
#include "capstan.hpp"
#include "spool.hpp"
#include "movesensor.hpp"
#include "endsensor.hpp"
#include "audio.hpp"

//----------------------------- Константы: -----------------------------------

enum TransportMode_t
{
  TR_STOP,  //стоп с торможением двигателями
  TR_PLAYF, //воспроизведение вперед
  TR_PLAYR, //воспроизведение назад
  TR_REC,   //запись (то же воспр. вперед, но с сигналом REC)
  TR_PAUSE, //пауза (тот же стоп, только без MUTE)
  TR_FFD,   //перемотка вперед
  TR_REW,   //перемотка назад
  TR_AFFD,  //архивная перемотка вперед
  TR_AREW,  //архивная перемотка назад

  //внутренние коды:
  TR_ASTOP, //стоп с механическим торможением
  TR_FFFD,  //быстрый переход из AFFD в FFD
  TR_FREW,  //быстрый переход из AREW в REW
  TR_CAPF,  //включение прямого вращения тонвала
  TR_CAPR,  //включение реверсного вращения тонвала
  TR_NONE   //неопределенное состояние
};

enum TransportState_t
{
  TRS_CAP   = 1 << 0, //capstan вращается
  TRS_REV   = 1 << 1, //capstan в режиме реверс
  TRS_LOCK  = 1 << 2, //capstan разогнался
  TRS_BRAKE = 1 << 3, //торможение двигателями
  TRS_TAPE  = 1 << 4, //лента загружена
  TRS_MOVE  = 1 << 5, //лента движется
  TRS_CUE   = 1 << 6  //режим обзора
};

enum CapMode_t
{
  CAP_START   = 1 << 0, //capstan start bit
  CAP_REVERSE = 1 << 1  //capstan reverse bit
};

enum Solenoids_t
{
  SOL_BRAKE = 1 << 0, //brake solenoid
  SOL_PRESS = 1 << 1, //press solenoid
  SOL_LIFT  = 1 << 2  //lift solenoid
};

enum SateBits_t
{
  STB_BRAKE = 1 << 0, //brake solenoid bit
  STB_PRESS = 1 << 1, //press solenoid bit
  STB_LIFT  = 1 << 2, //lift solenoid bit
  STB_MOVE  = 1 << 3, //tape move bit
  STB_TAPE  = 1 << 4, //tape load bit
  STB_RUN   = 1 << 5, //capstan start bit
  STB_REV   = 1 << 6, //capstan reverse bit
  STB_READY = 1 << 7  //capstan ready bit
};

//----------------------------------------------------------------------------
//--------------------------- Класс TOperations: -----------------------------
//----------------------------------------------------------------------------

class TOperations
{
private:
  uint8_t Number;
  uint8_t DoNumber;
  static const uint8_t BLOCKED = UINT8_MAX / 2;
  TSoftTimer *DelTimer;
public:
  TOperations(void) { DelTimer = new TSoftTimer(); };
  void Start(void) { DoNumber = 1; DelTimer->Force(); };
  bool DelayOver(void) { Number = 0; return(DelTimer->Over()); };
  bool NotDone(void) { return(++Number == DoNumber); };
  void Done(void) { DoNumber++; Number = BLOCKED; };
  void StartDelay(uint16_t del) { DelTimer->Start(del); };
};

//----------------------------------------------------------------------------
//--------------------------- Класс TTransport: ------------------------------
//----------------------------------------------------------------------------

class TTransport
{
private:
  friend class TPort;
  friend class TOperations;
  TSpool *Spool;            //объект боковых двигателей
  TMoveSensor *MoveSensor;  //объект датчика движения ленты
  TEndSensor *EndSensor;    //объект датчика наличия ленты
  TSolenoid<Pin_BrakeF_t, Pin_BrakeH_t> *SolBrake; //ЭМ тормозов
  TSolenoid<Pin_PressF_t, Pin_PressH_t> *SolPress; //ЭМ прижимного ролика
  TSolenoid<Pin_LiftF_t, Pin_LiftH_t> *SolLift;    //ЭМ отвода ленты
  uint8_t NowMode;
  uint8_t NewMode;
  uint8_t ReqMode;
  uint8_t ProMode;
  bool fCue;
  TOperations *Op;

  void Op_Capstan(bool rev, uint16_t del = 0);
  void Op_WaitCapstan(void);
  void Op_Press(bool s, uint16_t del = 0);
  void Op_Brake(bool s, uint16_t del = 0);
  void Op_Lift(bool s, uint16_t del = 0);
  void Op_SetTension(uint16_t t1, uint16_t t2);
  void Op_Spool(uint8_t m, uint16_t del = 0);
  void Op_WaitStop(void);
  void Op_AutoStop(uint8_t m);
  void Op_WaitTension(void);
  void Op_Mute(bool m, uint16_t del = 0);
  void Op_Delay(uint16_t del);
  void Op_Final(void);

  uint16_t AsBrkDel; //задержка торможения
  static const uint16_t NOM_AS_BRK_DEL = 5000; //макс. время торможения, мс
  uint16_t AsPreDel; //задержка пред. натяжения
  static const uint16_t NOM_AS_PRE_DEL = 800; //макс. время пред. натяжения, мс
  uint16_t AsTenDel; //задержка натяжения
  static const uint16_t NOM_AS_TEN_DEL = 1000; //макс. время без натяж./движ., мс
  uint16_t EmForce;  //параметр времени старта электромагнитов
  uint16_t ForceDel; //длительность старта FORCE
  static uint16_t const NOM_FORCE_DEL = 200; //ном. длительность FORCE, мс

  enum tr_opt_t               //опции
  {
    OPT_BRKASENABLE = 1 << 0, //разрешение автостопа при торможении
    OPT_PREASENABLE = 1 << 1, //разрешение автостопа по пред. натяжению
    OPT_TENASENABLE = 1 << 2, //разрешение автостопа по натяжению
    OPT_MOVASENABLE = 1 << 3  //разрешение автостопа по ДД
  };
  uint8_t Options;            //набор опций
  static uint8_t const NOM_TR_OPTIONS = OPT_BRKASENABLE +
    OPT_PREASENABLE + OPT_TENASENABLE + OPT_MOVASENABLE; //опции по умолчанию
  bool Option(uint8_t mask) { return(Options & mask); }; //чтение опции
  TSoftTimer *BrakeTimer; //таймер торможения
  bool fAsBrake; //флаг включения автостопа при торможении
  TSoftTimer *AutostopTimer; //таймер автостопа
  uint8_t AsMode; //текущий режим автостопа
  enum AsMode_t //режимы работы автостопа
  {
    AS_OFF,     //автостоп выключен
    AS_START,   //контролируются оба натяжения
    AS_PLAY,    //контролируются оба натяжения, ДО, ДД
    AS_FFD,     //контролируется натяжение справа, ДО, ДД
    AS_REW,     //контролируется натяжение слева, ДО, ДД
    AS_BRAKE    //доп. режим, контролируется ДД на неподвижность ленты
  };
  void SetAutoStop(uint8_t m);  //установка режима автостопа
  bool fTape;                   //флаг наличия ленты

public:
  TTransport(void);
  TAudio *Audio;                //объект управления аудиотрактом
  TCapstan *Capstan;            //объект ведущего двигателя

  void Execute(void);
  void SetMode(uint8_t mode);   //включение требуемого режима ЛПМ
  void SetCue(bool cue);        //управление отводом ленты и MUTE
  uint8_t GetState(void);       //чтение состояния для индикации
  bool CheckAutoStop(void);     //проверка состояния автостопа
  void SetAsBrkDel(uint16_t t) { AsBrkDel = t; };   //установка AsBrkDel
  uint16_t GetAsBrkDel(void) { return(AsBrkDel); }; //чтение AsBrkDel
  void SetAsPreDel(uint16_t t) { AsPreDel = t; };   //установка AsPreDel
  uint16_t GetAsPreDel(void) { return(AsPreDel); }; //чтение AsPreDel
  void SetAsTenDel(uint16_t t) { AsTenDel = t; };   //установка AsTenDel
  uint16_t GetAsTenDel(void) { return(AsTenDel); }; //чтение AsTenDel
  void SetOptions(uint8_t t)   { Options = t; };    //установка Options
  uint8_t GetOptions(void)   { return(Options); };  //чтение Options
  void SetEmForce(uint16_t t);  //установка времени старта электромагнитов
  uint16_t GetEmForce(void);    //чтение времени старта электромагнитов
  void EERead(void);            //чтение параметров из EEPROM
  void EESave(void);            //сохранение параметров в EEPROM
};

//----------------------------------------------------------------------------

#endif
