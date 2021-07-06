//----------------------------------------------------------------------------

//Модуль управления ведущим двигателем

//----------------------- Используемые ресурсы: ------------------------------

//Класс TCapstan осуществляет управление ведущим двигателем.
//Единственный сигнал управления, доступный в штатном блоке управления
//ведущим двигателем (БУВД) - это сигнал реверса. Когда требуется изменить
//направление вращения двигателя, меняется состояние этого сигнала.
//Одновременно запускается программный таймер, который формирует задержку
//на установление скорости двигателя. Величина задержки задается
//параметром CapStart, который можно менять с помощью сервисной программы.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "capstan.hpp"
#include "control.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TCapstan: ------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TCapstan::TCapstan(void)
{
  Pin_Rev.DirOut();
#ifdef REV_C
  Pin_Sleep.DirOut(PIN_HIGH);
  Pin_Lock.DirIn(PIN_PULL);
#endif
  fRun = 1;          //двигатель всегда включен
  fRev = 0;          //начальное направление вращения - вперед
  fLock = 0;         //двигатель не готов
  CapTimer = TSoftTimer<TT_ONESHOT>(NOM_CAP_START);
  CapTimer.Start();
}

//-------------------------- Проверка готовности: ----------------------------

void TCapstan::Execute(void)
{
  if(!fLock)
  {
    if(CapTimer.Over()) //проверка таймера
    {
      fLock = 1;         //готовность двигателя
    }
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//---------------- Запуск двигателя с нужным направлением: -------------------

void TCapstan::Start(bool rev)
{
  if(fRev != rev)
  {
    Pin_Rev = rev;
    fLock = 0;
    CapTimer.Start();
    fRev = rev;
  }
}

//---------------------- Чтение состояния двигателя: -------------------------

bool TCapstan::Running(void)
{
  return(fRun);
}

//---------------------- Чтение направления вращения: ------------------------

bool TCapstan::Reverse(void)
{
  return(fRev);
}

//---------------------- Чтение готовности двигателя: ------------------------

bool TCapstan::Ready(void)
{
  return(fLock);
}

//------------------------- Задание времени старта: --------------------------

void TCapstan::SetTstart(uint16_t t)
{
  CapStart = t;
  CapTimer.Interval = CapStart;
}

//-------------------------- Чтение времени старта: --------------------------

uint16_t TCapstan::GetTstart(void)
{
  return(CapStart);
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TCapstan::EERead(void)
{
  CapStart = Eeprom->Rd16(EE_CAP_START, NOM_CAP_START);
  if(!fLock) CapTimer.Start(CapStart);
    else CapTimer.Interval = CapStart;
}

//------------------- Сохранение параметров в EEPROM: ------------------------

void TCapstan::EESave(void)
{
  Eeprom->Wr16(EE_CAP_START, CapStart);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
