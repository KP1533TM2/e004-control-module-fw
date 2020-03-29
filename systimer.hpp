//----------------------------------------------------------------------------

//Модуль системного таймера, заголовочный файл

//----------------------------------------------------------------------------

#ifndef SYSTIMER_HPP
#define SYSTIMER_HPP

//----------------------------- Константы: -----------------------------------

uint64_t const F_CLK = 16000000; //тактовая частота, Гц
uint16_t const T_SYS = 1000;     //системный интервал, мкс

//параметры специализации шаблонного класса TSoftTimer:
enum TimerType_t
{
  TT_PLAIN,
  TT_AUTORELOAD,
  TT_ONESHOT
};

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
//----------------------- Шаблонный класс TSoftTimer: ------------------------
//----------------------------------------------------------------------------

template<TimerType_t T = TT_PLAIN>
class TSoftTimer
{
private:
  uint16_t StartCount;
  bool fEvent;
  bool fRun;
protected:
public:
  TSoftTimer(uint16_t t = 0);
  uint16_t Interval;
  void Start(uint16_t t);
  void Start();
  void Stop();
  void Force(void);
  bool Over(void);
};

//----------------------------------------------------------------------------
//-------------------------- Реализация методов: -----------------------------
//----------------------------------------------------------------------------

//---------------------------- Конструктор: ----------------------------------

template<TimerType_t T>
TSoftTimer<T>::TSoftTimer(uint16_t t)
{
  Interval = t;
  fRun = 0;
  fEvent = 0;
  StartCount = TSysTimer::Counter;
}

//-------------------------------- Старт: ------------------------------------

template<TimerType_t T>
void TSoftTimer<T>::Start(uint16_t t)
{
  Interval = t;
  fRun = 1;
  fEvent = 0;
  StartCount = TSysTimer::Counter;
}

template<TimerType_t T>
void TSoftTimer<T>::Start()
{
  fRun = 1;
  fEvent = 0;
  StartCount = TSysTimer::Counter;
}

//--------------------------------- Стоп: ------------------------------------

template<TimerType_t T>
void TSoftTimer<T>::Stop()
{
  fRun = 0;
}

//---------------------- Принудительное переполнение: ------------------------

template<TimerType_t T>
void TSoftTimer<T>::Force(void)
{
  fRun = 1;
  StartCount = TSysTimer::Counter - Interval;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif
