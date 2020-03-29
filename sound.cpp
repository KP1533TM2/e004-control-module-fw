//----------------------------------------------------------------------------

//Модуль генерации звуковых сигналов

//----------------------- Используемые ресурсы: ------------------------------

//Звуковой излучатель (без встроенного генератора) через транзисторный ключ
//подключен к выводу PB7 (выход таймера OC2). Активный уровень - ВЫСОКИЙ.
//Таймер работает в режиме CTC с предделителем частоты на 256.
//При включении генерации звука включается режим Toggle OC2 on Compare Match.
//При выключении генерации звука включается режим Clear OC2 on Compare Match.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "sound.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TSound: --------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TSound::TSound(void)
{
  SoundTimer = new TSoftTimer<TT_ONESHOT>();
  TCCR2 = SND_OFF;
  Pin_SND.DirOut();
  fSoundEn = 1;
}

//----------------------------------------------------------------------------
//--------------------------- Внутренние методы: -----------------------------
//----------------------------------------------------------------------------

//--------------------- Формирование длительности звука: ---------------------

void TSound::Execute(void)
{
  if(SoundTimer->Over()) TCCR2 = SND_OFF;
}

//---------------------------- Генерация звука: ------------------------------

//t - полупериод в тиках таймера
//d - длительность d [мс]

void TSound::Play(uint8_t t, uint16_t d)
{
  if(fSoundEn)
  {
    OCR2 = t;
    SoundTimer->Start(d);
    TCCR2 = SND_ON;
  }
  else
  {
    TCCR2 = SND_OFF;
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//----------------------- Включение/выключение звука: ------------------------

void TSound::OnOff(bool on)
{
  fSoundEn = on;
  if(!fSoundEn) TCCR2 = SND_OFF;
}

//--------------------------- Стандартные звуки: -----------------------------

#define HZ2T(x) ((uint8_t)((F_CLK / 256 / 2) / (x)))

//Play(HZ2T(F [Hz]), T [ms])

void TSound::Beep(void)  { Play(HZ2T(1000), 100); }
void TSound::Tick(void)  { Play(HZ2T(1000),  40); }
void TSound::High(void)  { Play(HZ2T(1600), 100); }
void TSound::Click(void) { Play(HZ2T(400),   20); }
void TSound::Bell(void)  { Play(HZ2T(360),  300); }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
