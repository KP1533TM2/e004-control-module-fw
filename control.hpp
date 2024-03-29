//----------------------------------------------------------------------------

//Модуль общего управления, заголовочный файл

//----------------------------------------------------------------------------

#ifndef CONTROL_HPP
#define CONTROL_HPP

//----------------------------------------------------------------------------

#include "local.hpp"
#include "remote.hpp"
#include "sound.hpp"
#include "transport.hpp"
#include "leds.hpp"
#include "port.hpp"
#include "eeprom.hpp"

//----------------------------------------------------------------------------
//---------------------------- Класс TControl: -------------------------------
//----------------------------------------------------------------------------

class TControl
{
private:
  friend class TPort;
  TLocal *Local;       //объект местной клавиатуры
  TRemote *Remote;     //объект ИК ДУ
  TSound *Sound;       //объект управления бипером
  TLeds *Leds;         //объект управления светодиодами
  TTransport *Transport; //объект управления ЛПМ

  static uint8_t const T_SLOW = 10; //медленные тики, мс
  uint8_t ServiceCounter; //программный счетчик таймера сервисов

  uint8_t Mode;        //текущий режим
  uint8_t BackMode;    //режим возврата
  bool fPause;         //флаг режима "Пауза"
  bool fRoll;          //флаг режима "Откат"
  uint8_t KeyCode;     //код кнопки
  bool fUpdate;        //флаг обновления индикации
  uint8_t TrState;     //состояние ЛПМ
  bool Trs(uint8_t mask) { return(TrState & mask); };
  uint8_t IndState;    //текущее состояние индикации
  bool fAutostop;      //флаг срабатывания автостопа
  uint8_t ArMode;      //режим для автореверса
  TSoftTimer<TT_ONESHOT> *ProgTimer; //таймер программирования
  static uint16_t const T_PROG = 3000; //задержка выхода из программировния
  uint8_t Program;     //режим программирования
  enum prog_t
  {
    PR_OFF,
    PR_STEP1,
    PR_STEP2,
    PR_RUN
  };
  void ProgMode(uint8_t &code); //программирование режимов
  uint8_t ProgMode1;   //программный режим 1
  uint8_t ProgMode2;   //программный режим 2

  Pin_Pvg_t Pin_Pvg;   //вход контроля питания
  //unused pins:
  Pin_Dir_t Pin_Dir;   //направление порта RS-485
  Pin_SCL_t Pin_SCL;   //линия SCL порта I2C
  Pin_SDA_t Pin_SDA;   //линия SDA порта I2C
  Pin_RXD1_t Pin_RXD1; //линия RXD порта RS-485
  Pin_TXD1_t Pin_TXD1; //линия TXD порта RS-485
  Pin_PB1_t Pin_PB1;   //не используется
#ifdef REV_A
  Pin_XT1_t Pin_XT1;   //test pin XT1
  Pin_XT2_t Pin_XT2;   //test pin XT2
  Pin_XT3_t Pin_XT3;   //test pin XT3
  Pin_XT4_t Pin_XT4;   //test pin XT4
  Pin_XT5_t Pin_XT5;   //test pin XT5
  Pin_XT6_t Pin_XT6;   //test pin XT6
#endif
#ifdef REV_C
  Pin_AdcEnd_t Pin_AdcEnd;
  Pin_Aux_t Pin_Aux;
#endif

  void HardwareInit(void);      //инициализация оборудования
  void FilterKey(uint8_t &code); //фильтрация кодов кнопок
  uint8_t Arch(uint8_t code);   //формирование кода перемотки с учетом архивной

  bool ServiceTimer(void);      //таймер сервисов
  void LedsService(void);       //сервис управления светодиодами
  void AutoStopService(void);   //сервис автостопа
  void AutoPlayService(void);   //сервис программного режима и автореверса

  uint16_t Options;             //набор опций
  enum ct_opt_t                 //опции
  {
    OPT_PAUSELEDBLINK = 1 << 0,  //cветодиод паузы мигает, иначе - горит
    OPT_AUTORECPAUSE  = 1 << 1,  //автоматическое вкл. паузы при вкл. записи
    OPT_SHOWDIR       = 1 << 2,  //разрешение индикации направления capstan
    OPT_USEARCHIVE    = 1 << 3,  //использовать архивную перемотку
    OPT_ROLLCUE       = 1 << 4,  //при откате включен обзор (и выключен Mute)
    OPT_ENABLECUE     = 1 << 5,  //разреш. обзора кнопкой Roll при арх. перем.
    OPT_NOSOUND       = 1 << 6,  //запрещение генерации звуковых сигналов
    OPT_AUTOREVERSE   = 1 << 7,  //включение режима автореверса
    OPT_ENABLEPROG    = 1 << 8,  //разрешение программирования режимов
    OPT_PROGBLINK     = 1 << 9,  //прогрммный режим мигает, иначе горит
    OPT_MUTEPAUSE     = 1 << 10, //разрешение mute в паузе
    OPT_PLAYRECEN     = 1 << 11, //разрешение включения Rec в режиме Play
    OPT_RECPLAYEN     = 1 << 12, //разрешение включения Play в режиме Rec
    OPT_PLAYEXPAUSE   = 1 << 13  //Play выключает Pause
  };
  //опции по умолчанию:
  static uint16_t const NOM_CT_OPTIONS = OPT_SHOWDIR + OPT_USEARCHIVE +
    OPT_ENABLECUE + OPT_AUTOREVERSE + OPT_ENABLEPROG + OPT_PLAYEXPAUSE;
  bool Option(ct_opt_t mask) { return(Options & mask); }; //чтение опции

public:
  TControl(void);
  void Execute(void);
  void SetMode(uint8_t code);   //включение режима
  void SetOptions(uint16_t t);  //установка опций
  uint16_t GetOptions(void);    //чтение опций
  void EERead(void);            //чтение параметров из EEPROM
  void EESave(void);            //сохранение параметров в EEPROM
};

//----------------------------------------------------------------------------

extern TControl *Control;

//----------------------------------------------------------------------------

#endif
