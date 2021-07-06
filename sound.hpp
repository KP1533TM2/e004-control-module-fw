//----------------------------------------------------------------------------

//Модуль генерации звуковых сигналов, заголовочный файл

//----------------------------------------------------------------------------

#ifndef SOUND_H
#define SOUND_H

//----------------------------------------------------------------------------

#include "systimer.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TSound: --------------------------------
//----------------------------------------------------------------------------

class TSound
{
private:
  Pin_SND_t Pin_SND;
  TSoftTimer<TT_ONESHOT> SoundTimer;
  void Play(uint8_t t, uint16_t d);
  static uint8_t const PRE_256 = (1 << CS22);
  static uint8_t const SND_ON = ((1 << COM20) | (1  <<  WGM21) | PRE_256);
  static uint8_t const SND_OFF = ((1 << COM21) | (1  <<  WGM21) | PRE_256);
  bool fSoundEn;
public:
  TSound(void);
  void Execute(void);
  void OnOff(bool on);
  void Beep(void);
  void Tick(void);
  void High(void);
  void Click(void);
  void Bell(void);
};

//----------------------------------------------------------------------------

#endif
