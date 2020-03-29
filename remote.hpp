//----------------------------------------------------------------------------

//Модуль декодера команд ИК ДУ: header file

//----------------------------------------------------------------------------

#ifndef REMOTE_HPP
#define REMOTE_HPP

//----------------------------------------------------------------------------

#include "systimer.hpp"
#include "keyboard.hpp"

//----------------------------------------------------------------------------
//---------------------------- Класс TRemote: --------------------------------
//----------------------------------------------------------------------------

#pragma vector = INT4_vect
extern "C" __interrupt void EdgeIR(void); //прерывание фотоприемника
#pragma vector = TIMER0_OVF_vect
extern "C" __interrupt void TimerIR(void); //прерывание таймера 0

class TRemote : public TKeyboard
{
  friend __interrupt void EdgeIR(void);
  friend __interrupt void TimerIR(void);
private:
  static uint8_t const RC5_LENGTH =   14; //количество принимаемых битов
  struct RC5Struct                        //структура кода RC-5
  {
    uint16_t RC5Command : 6;
    uint16_t RC5System  : 5;
    uint16_t RC5Control : 1;
    uint16_t RC5Start   : 2;
  };
  static uint16_t const RC5_SLOT =  1778; //длительность слота RC-5, мкс
  static uint16_t const REP_TM =     120; //период повторения посылки, мс
  static uint16_t const PRE =         64; //предделитель таймера
  static uint8_t const T_SAMPLE = RC5_SLOT / 2 * F_CLK / PRE / 1000000; //Ts
  static uint8_t const DEL2 = 256 - T_SAMPLE;     //код задержки Ts
  static uint8_t const DEL1 = 256 - T_SAMPLE / 2; //код задержки 1/2 Ts
  static Pin_IR_t Pin_IR;
  enum { ST_SMP1, ST_SMP2, ST_TO, ST_END }; //состояния опроса
  static uint8_t volatile State;
  static uint8_t volatile System;
  static uint8_t volatile Command;
  static uint8_t const RC5_CB = 0x80;  //RC-5 control bit в коде команды
  static uint8_t const RC5_MAX = 0x3F; //максимальный код команды RC-5
  static uint8_t const RC5_NO = 0xFF;  //код отсутствия команды
  uint8_t Prev_Cmd;
  TSoftTimer *RemTimer;
  static void TmrIntEnable(uint8_t delay);
  static void TmrIntDisable(void);
  static void ExtIntEnable(void);
  static void ExtIntDisable(void);
  virtual uint8_t GetCode(void);
public:
  TRemote(void);
};

//----------------------------------------------------------------------------

#endif
