//----------------------------------------------------------------------------

//Абстрактный класс клавиатуры: header file

//----------------------------------------------------------------------------

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

//----------------------------------------------------------------------------

#include "gpio.hpp"
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
  KEY_UNROLL, //выключение отката
  //RC only:
  KEY_PLAY,   //воспроизведение
  KEY_REV,    //реверс
  KEY_MUTE,   //Mute
  NC = KEY_NO //краткий синоним для KEY_NO
};

enum KeyMsg_t //коды сообщений клавиатуры
{
  MSG_NO,     //нет нажатия
  MSG_PRESS,  //нажатие кнопки
  MSG_REL,    //отпускание кнопки
  MSG_HOLD,   //удержание кнопки
  MSG_LONG    //длительное удержание кнопки
};

//----------------------------------------------------------------------------
//-------------------- Абстрактный класс TKeyboard: --------------------------
//----------------------------------------------------------------------------

class TKeyboard
{
private:
  static uint16_t const DEBOUNCE_TM  = 50; //интервал подавления дребезга, ms
  static uint16_t const HOLD_DELAY = 1000; //задержка удержания кнопки, ms
  static uint16_t const LONG_DELAY = 3000; //длинная задержка удержания, ms
  TSoftTimer *KeyTimer;
  uint8_t State;
  enum { ST_DONE, ST_NEW, ST_PRESS, ST_HOLD };
  uint8_t New_Msg;
  uint8_t Prev_Key;
  virtual uint8_t GetCode(void) = 0;
public:
  TKeyboard(void);
  void Execute(void);
  bool NewMessage(void);
  uint8_t Message;
  uint8_t Code;
};

//----------------------------------------------------------------------------

#endif
