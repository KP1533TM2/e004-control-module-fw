//----------------------------------------------------------------------------

//Модуль системного таймера, заголовочный файл

//----------------------------------------------------------------------------

#ifndef SYSTIMER_HPP
#define SYSTIMER_HPP

//----------------------------- Константы: -----------------------------------

uint64_t const F_CLK = 16000000; //тактовая частота, Гц
uint16_t const T_SYS = 1000;     //системный интервал, мкс

//----------------------------------------------------------------------------
//--------------------------- Класс TSysTimer: -------------------------------
//----------------------------------------------------------------------------

#pragma vector = TIMER3_COMPA_vect
extern "C" __interrupt void SysTick_Handler(void);

class TSysTimer
{
  friend __interrupt void SysTick_Handler(void);
private:
  static volatile bool vTick;
  static volatile uint16_t vCounter;
  static uint16_t Start_ms;
  static uint16_t Interval_ms;
  static uint16_t GetCount(void);
protected:
public:
  static void Init(void);
  static bool Tick;
  static uint16_t Counter;
  static void Sync(void);
  static void Delay_ms(uint16_t d);
  static void TimeoutStart_ms(uint16_t t);
  static bool TimeoutOver_ms(void);
};

//----------------------------------------------------------------------------
//-------------------------- Класс TSoftTimer: -------------------------------
//----------------------------------------------------------------------------

class TSoftTimer
{
private:
  static const bool UseAutoreload = 0;
  uint16_t StartCount;
  bool fEvent;
protected:
public:
  TSoftTimer(uint16_t t = 0);
  uint16_t Interval;
  bool Autoreload;
  bool Oneshot;
  void Start();
  void Start(uint16_t t);
  void Stop();
  void Force(void);
  bool Over(void);
};

//----------------------------------------------------------------------------

#endif
