//----------------------------------------------------------------------------

//Модуль системного таймера

//----------------------------------------------------------------------------

#include "main.hpp"
#include "systimer.hpp"

//----------------------- Используемые ресурсы: ------------------------------

//Системный таймер реализован на основе прерываний таймера с периодом 1 мс.
//Системный тик с таким периодом раздается всем процессам. Кроме того,
//в этой сетке работают 16-разрядные програмные таймеры (TSoftTimer),
//которые могут создаваться по мере необходимости.

//----------------------------------------------------------------------------
//--------------------------- Класс TSysTimer --------------------------------
//----------------------------------------------------------------------------

//-------------------- Инициализация системного таймера: ---------------------

void TSysTimer::Init(void)
{
  TCCR3B = (1 << WGM32) | (1 << CS30); //режим таймера 3: CTC, CK/1
  OCR3A = (F_CPU * T_SYS / 1000000) - 1; //загрузка регистра сравнения
  ETIFR = (1 << OCF3A);   //сброс отложенных прерываний
  ETIMSK = (1 << OCIE3A); //разрешение прерывания по совпадению
  vCounter = 0;
  Counter = 0;
  __enable_interrupt();   //разрешение прерываний
}

//--------------------- Прерывание системного таймера: -----------------------

volatile bool TSysTimer::vTick;
volatile uint16_t TSysTimer::vCounter;

#pragma vector = TIMER3_COMPA_vect
__nested __interrupt void SysTick_Handler(void)
{
  TSysTimer::vCounter++;
  TSysTimer::vTick = 1;
}

//-------------------- Синхронизация системных тиков: ------------------------

bool TSysTimer::Tick;
uint16_t TSysTimer::Counter;

__monitor void TSysTimer::Sync(void)
{
  Counter = vCounter;
  Tick = vTick;
  vTick = 0;
}

//---------------------- Атомарное чтение счетчика: --------------------------

__monitor uint16_t TSysTimer::GetCount(void)
{
  return(vCounter);
}

//-------------- Функция задержки миллисекундного диапазона: -----------------

void TSysTimer::Delay_ms(uint16_t d)
{
  uint16_t DelayStart = GetCount();
  do { Sync(); __watchdog_reset(); }
    while(Counter - DelayStart < d);
}

//------------------ Таймаут миллисекундного диапазона: ----------------------

uint16_t TSysTimer::Start_ms;
uint16_t TSysTimer::Interval_ms;

void TSysTimer::TimeoutStart_ms(uint16_t t)
{
  Start_ms = GetCount();
  Interval_ms = t;
}

bool TSysTimer::TimeoutOver_ms(void)
{
  return(GetCount() - Start_ms >= Interval_ms);
}

//----------------------------------------------------------------------------
//-------- Специализация метода Over шаблонного класса TSoftTimer: -----------
//----------------------------------------------------------------------------

//------------------------- Чтение переполнения: -----------------------------

template<>
bool TSoftTimer<TT_PLAIN>::Over(void)
{
  bool fevent = TSysTimer::Counter - StartCount >= Interval;
  if(fevent) fEvent = 1;
  return(fEvent && fRun);
}

template<>
bool TSoftTimer<TT_AUTORELOAD>::Over(void)
{
  bool fevent = TSysTimer::Counter - StartCount >= Interval;
  if(fevent) StartCount = TSysTimer::Counter;
  return(fevent && fRun);
}

template<>
bool TSoftTimer<TT_ONESHOT>::Over(void)
{
  bool fevent = TSysTimer::Counter - StartCount >= Interval;
  if(fevent) { if(fEvent) fevent = 0; else fEvent = 1; }
  return(fevent && fRun);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
