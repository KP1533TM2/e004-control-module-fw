//----------------------------------------------------------------------------

//Реализация шаблонного класса TSolenoid

//----------------------- Используемые ресурсы: ------------------------------

//Данный класс предназначен для управления электромагнитами. Для каждого
//электромагнита выделены два вывода процессора. Один из них отвечает за
//режим старта (PinForce), другой - за режим удержания (PinHold).
//Эти сигналы управляют линейными генераторами тока, собранными на плате БУ.
//Во время действия сигнала Force к электромагниту прикладывается полное
//напряжение питания. В результате электромагнит срабатывает. Для удержания
//в сработавшем состоянии требуется значительно меньший ток. Поэтому через
//определенное время сигнал Force отключается, остается только сигнал Hold.
//Он включает генератор тока, значение которого можно установить подстроечным
//резистором индивидуально для каждого электромагнита. Это уменьшает
//потребление энергии и нагрев электромагнитов, а также снижает поле рассеяния.
//К тому же, ток питания электромагнитов в режиме удержания является
//стабилизированным и не содержит пульсаций сетевой частоты, что снижает
//уровень помех. Длительность сигнала старта электромагнитов задется с помощью
//параметра EmForce, который можно менять с помощью сервисной программы.

//----------------------------------------------------------------------------

#ifndef SOLENOID_HPP
#define SOLENOID_HPP

//----------------------------------------------------------------------------

#include "systimer.hpp"

//----------------------------- Константы: -----------------------------------

static uint16_t NOM_EM_FORCE = 1000; //номинальное время старта, мс

//----------------------------------------------------------------------------
//---------------------- Шаблонный класс TSolenoid: --------------------------
//----------------------------------------------------------------------------

template<class PinForce, class PinHold>
class TSolenoid
{
private:
  TSoftTimer<TT_ONESHOT> *ForceTimer;
public:
  TSolenoid(void);
  void Execute(void);
  void On(void);
  void Off(void);
  void OnOff(bool on);
  bool Hold(void);
  void Force(uint16_t t);
};

//------------------------- Реализация методов: ------------------------------

template<class PinForce, class PinHold>
TSolenoid<PinForce, PinHold>::TSolenoid(void)
{
  PinForce::DirOut();
  PinHold::DirOut();
  ForceTimer = new TSoftTimer<TT_ONESHOT>(NOM_EM_FORCE);
}

template<class PinForce, class PinHold>
void TSolenoid<PinForce, PinHold>::Execute(void)
{
  if(PinForce::GetLatch() && ForceTimer->Over())
  {
    PinForce::Clr();
  }
}

template<class PinForce, class PinHold>
void TSolenoid<PinForce, PinHold>::On(void)
{
  PinHold::Set();
  PinForce::Set();
  ForceTimer->Start();
}

template<class PinForce, class PinHold>
void TSolenoid<PinForce, PinHold>::Off(void)
{
  PinHold::Clr();
  PinForce::Clr();
  ForceTimer->Stop();
}

template<class PinForce, class PinHold>
void TSolenoid<PinForce, PinHold>::OnOff(bool on)
{
  if(on) On(); else Off();
}

template<class PinForce, class PinHold>
bool TSolenoid<PinForce, PinHold>::Hold(void)
{
  return(PinHold::GetLatch());
}

template<class PinForce, class PinHold>
void TSolenoid<PinForce, PinHold>::Force(uint16_t t)
{
  ForceTimer->Interval = t;
}

//----------------------------------------------------------------------------

#endif
