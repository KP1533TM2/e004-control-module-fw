//----------------------------------------------------------------------------

//Модуль управления светодиодами, заголовочный файл

//----------------------------------------------------------------------------

#ifndef LEDS_HPP
#define LEDS_HPP

//-------------------------------- Константы: --------------------------------

//Маски светодиодов:

enum LedMask_t
{
  LED_NONE  = 0,
  LED_REC   = 1 << 0,
  LED_STOP  = 1 << 1,
  LED_PLAYF = 1 << 2,
  LED_FFD   = 1 << 3,
  LED_REW   = 1 << 4,
  LED_PLAYR = 1 << 5,
  LED_PAUSE = 1 << 6,
  LED_ALL   = 0xFF
};

//Режимы светодиодов:

enum LedMode_t
{
  LEDS_CONT, //непрерывное горение
  LEDS_SLOW, //медленное мигание
  LEDS_NORM, //нормальное мигание
  LEDS_FAST, //быстрое мигание
  LEDS_OFF,  //выключение
  LEDS_MODES = LEDS_OFF
};

typedef struct
{
  uint8_t cont; //непрерывно горящие светодиоды
  uint8_t slow; //медленно мигабщие светодиоды
  uint8_t norm; //нормально мигабщие светодиоды
  uint8_t fast; //быстро мигабщие светодиоды
} leds_t;

//----------------------------------------------------------------------------
//----------------------------- Класс TLeds: ---------------------------------
//----------------------------------------------------------------------------

class TLeds
{
private:
  Led_Rec_t   Led_Rec;
  Led_Stop_t  Led_Stop;
  Led_PlayF_t Led_PlayF;
  Led_Ffd_t   Led_Ffd;
  Led_Rew_t   Led_Rew;
  Led_PlayR_t Led_PlayR;
  Led_Pause_t Led_Pause;

  uint8_t SlowBlink; //счетчик медленного мигания
  uint8_t FastBlink; //счетчик быстрого мигания
  uint8_t NormBlink; //счетчик нормального мигания
  static uint8_t const T_SLOW = 10; //медленные тики, мс
  static uint16_t const SLOW_PERIOD = 2000; //период мигания, мс
  static uint16_t const SLOW_DUTY = 50;     //коэффициент заполнения, %
  static uint16_t const NORM_PERIOD = 1000; //период мигания, мс
  static uint16_t const NORM_DUTY = 50;     //коэффициент заполнения, %
  static uint16_t const FAST_PERIOD = 160;  //период мигания, мс
  static uint16_t const FAST_DUTY = 50;     //коэффициент заполнения, %
  leds_t Mask;               //маски светодиодов для всех режимов
  uint8_t PreLeds;           //предыдущий набор светодиодов
  void Update(uint8_t leds); //включение/выключение светодиодов
public:
  TLeds(void);
  void Execute(void); //выполнение индикации (горения, мигания)
  void Set(uint8_t leds, uint8_t mode = LEDS_OFF); //вкл./выкл. светодиода
  void Get(leds_t &leds); //чтение состояния светодиодов
};

//----------------------------------------------------------------------------

#endif
