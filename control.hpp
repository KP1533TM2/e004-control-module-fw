//----------------------------------------------------------------------------

//Модуль общего управления, заголовочный файл

//----------------------------------------------------------------------------

#ifndef CONTROL_HPP
#define CONTROL_HPP

//----------------------------------------------------------------------------

#include "systimer.hpp"
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
  uint8_t KeyMsg;      //тип сообщения клавиатуры
  uint8_t KeyCode;     //код кнопки
  bool fUpdate;        //флаг обновления индикации
  uint8_t TrState;     //состояние ЛПМ
  bool Trs(uint8_t mask) { return(TrState & mask); };
  uint8_t IndState;    //текущее состояние индикации
  bool fAutostop;      //флаг срабатывания автостопа
  uint8_t ArMode;      //режим для автореверса

  Pin_Pvg_t Pin_Pvg;   //вход контроля питания
  //unused pins:
  Pin_Dir_t Pin_Dir;   //направление порта RS-485
  Pin_SCL_t Pin_SCL;   //линия SCL порта I2C
  Pin_SDA_t Pin_SDA;   //линия SDA порта I2C
  Pin_RXD1_t Pin_RXD1; //линия RXD порта RS-485
  Pin_TXD1_t Pin_TXD1; //линия TXD порта RS-485
  Pin_XT6_t Pin_XT6;   //test pin XT6
  Pin_XT5_t Pin_XT5;   //test pin XT5
  Pin_XT4_t Pin_XT4;   //test pin XT4
  Pin_XT3_t Pin_XT3;   //test pin XT3
  Pin_XT2_t Pin_XT2;   //test pin XT2
  Pin_XT1_t Pin_XT1;   //test pin XT1
  Pin_PB1_t Pin_PB1;   //не используется

  void HardwareInit(void);      //инициализация оборудования

  bool ServiceTimer(void);      //таймер сервисов
  void LedsService(void);       //сервис управления светодиодами
  void AutoStopService(void);   //сервис автостопа
  void AutoRevService(void);    //сервис автореверса

  enum ct_opt_t                 //опции
  {
    OPT_PAUSELEDBLINK = 1 << 0, //cветодиод паузы мигает, иначе - горит
    OPT_AUTORECPAUSE  = 1 << 1, //автоматическое вкл. паузы при вкл. записи
    OPT_PLAYOFFPAUSE  = 1 << 2, //кнопка Play выключает паузу
    OPT_USEARCHIVE    = 1 << 3, //использовать архивную перемотку
    OPT_ROLLCUE       = 1 << 4, //при откате включен обзор (и выключен Mute)
    OPT_ENABLECUE     = 1 << 5, //разреш. обзора кнопкой Roll при арх. перем.
    OPT_NOSOUND       = 1 << 6, //запрещение генерации звуковых сигналов
    OPT_AUTOREVERSE   = 1 << 7  //включение режима автореверса
  };
  uint8_t Options;              //набор опций
  static uint8_t const NOM_CT_OPTIONS = 0; //опции по умолчанию
  bool Option(uint8_t mask) { return(Options & mask); }; //чтение опции

public:
  TControl(void);
  void Execute(void);
  void SetMode(uint8_t code);   //включение режима
  void SetOptions(uint8_t t);   //установка опций
  uint8_t GetOptions(void);     //чтение опций
  void EERead(void);            //чтение параметров из EEPROM
  void EESave(void);            //сохранение параметров в EEPROM
};

//----------------------------------------------------------------------------

extern TControl *Control;

//----------------------------------------------------------------------------

#endif
