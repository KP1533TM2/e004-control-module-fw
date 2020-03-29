//----------------------------------------------------------------------------

//Модуль управления аудиотрактом

//------------------------ Используемые ресурсы: -----------------------------

//Управление УВ:
//Pin_Mute = 1 - включение блокировки (MUTE).
//Pin_PlayR = 1 - включение реверсного канала.
//Pin_PlayF = 1 - включение прямого канала.
//Управление УЗ:
//Pin_Rec = 1 - включение режима записи.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "audio.hpp"

//----------------------------------------------------------------------------
//------------------------------ Класс TAudio: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TAudio::TAudio(void)
{
  Pin_Rec.DirOut();
  Pin_PlayR.DirOut();
  Pin_PlayF.DirOut(PIN_HIGH);
  Pin_Mute.DirOut(PIN_HIGH);
  fNormalMute = 1;
  fMasterMute = 0;
}

//----------------- Глобальное включение/выключение MUTE: --------------------

void TAudio::MasterMute(bool on)
{
  fMasterMute = on;
  Pin_Mute = fNormalMute | fMasterMute;
}

//-------------------- Переключение глобального MUTE: ------------------------

void TAudio::MuteToggle(void)
{
  fMasterMute = !fMasterMute;
  Pin_Mute = fNormalMute | fMasterMute;
}

//---------------------- Включение/выключение MUTE: --------------------------

void TAudio::Mute(bool on)
{
  fNormalMute = on;
  Pin_Mute = on | fMasterMute;
}

//--------------------- Включение/выключение REVERSE: ------------------------

void TAudio::Rev(bool on)
{
  Pin_PlayF = !on;
  Pin_PlayR = on;
}

//--------------------- Включение/выключение RECORD: -------------------------

void TAudio::Rec(bool on)
{
  Pin_Rec = on;
}

//--------------------------- Чтение состояния: ------------------------------

uint8_t TAudio::GetState(void)
{
  uint8_t s = 0;
  if(Pin_Mute.GetLatch()) s |= AUD_MUTE;
  if(Pin_PlayR.GetLatch()) s |= AUD_REV;
  if(Pin_Rec.GetLatch()) s |= AUD_REC;
  return(s);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
