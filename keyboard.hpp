//----------------------------------------------------------------------------

//Абстрактный класс клавиатуры: header file

//----------------------------------------------------------------------------

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

//----------------------------------------------------------------------------

#include "systimer.hpp"

//----------------------------- Константы: -----------------------------------

enum KeyCode_t //коды кнопок
{
  KEY_NO,     //нет нажатия
  KEY_REC,    //запись
  KEY_STOP,   //стоп
  KEY_PLAYF,  //воспроизведение вперед
  KEY_FFD,    //перемотка вперед
  KEY_REW,    //перемотка назад
  KEY_PLAYR,  //воспроизведение назад
  KEY_PAUSE,  //пауза
  KEY_ROLL,   //откат
  KEY_UNROLL, //выключение отката (отпускание кнопки)
  KEY_PROG,   //вход в програмирование
  //RC only:
  KEY_PLAY,   //воспроизведение
  KEY_REV,    //реверс
  KEY_MUTE,   //Mute
  //модификаторы кода кнопки:
  MSG_REL  = 0x40,
  MSG_HOLD = 0x80,
  //краткий синоним для KEY_NO:
  NC = KEY_NO
};

//----------------------------------------------------------------------------
//-------------------- Абстрактный класс TKeyboard: --------------------------
//----------------------------------------------------------------------------

class TKeyboard
{
private:
  static uint16_t const DEBOUNCE_TM  = 50; //интервал подавления дребезга, ms
  static uint16_t const HOLD_DELAY = 1500; //задержка удержания кнопки, ms
  TSoftTimer<TT_ONESHOT> *KeyTimer;
  uint8_t State;
  enum { ST_DONE, ST_NEW, ST_PRESS, ST_HOLD };
  uint8_t Prev_Key;
  uint8_t LastCode;
  uint8_t Code;
  virtual uint8_t GetCode(void) = 0;
public:
  TKeyboard(void);
  void Execute(void);
  uint8_t GetKeyCode(void);
};

//----------------------------------------------------------------------------

#endif
