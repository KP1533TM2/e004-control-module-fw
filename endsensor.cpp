//----------------------------------------------------------------------------

//Модуль обслуживания датчика окончания ленты

//----------------------- Используемые ресурсы: ------------------------------

//Датчик окончания ленты формирует на выходе аналоговый сигнал, который
//подается на вход компаратора AIN1. На второй вход компаратора AIN0
//подается опорное напряжение с подстроечного резистора, который позволяет
//регулировать порог срабатывания.

//Штатный датчик окончания ленты выполнен в виде пары излучатель-приемник
//света. В оригинальной конструкции в качестве источника света используется
//лампочка накаливания, а в качестве приемника - фотодиод. Когда лента
//заканчивается, фотодиод засвечивается. При этом на входе AIN1 напряжение
//повышается, и когда оно превысит опорное напряжение на входе AIN0,
//компаратор переключится в состояние ACO = 0. Это состояние означает
//отсутствие ленты.

//Для пары в виде лампочки накаливания и фотодиода прозрачность многих
//ракордов не отличается от прозрачности ленты, что не позволяет определить
//начало ракорда, если не сделать прозрачные "окна", смыв рабочий слой.

//Если перейти на пару ИК светодиод-фотодиод, то контраст ракорда увеличивается,
//хотя все равно находятся ленты, которые в ИК диапазоне прозрачней ракордов.
//В случае применения других типов излучателя и приемника света может
//потребоваться корректировка номинала нагрузочного резистора приемника
//и пределов регулировки порога.

//При наличии небольших повреждений магнитного слоя ленты или склеек,
//могут появляться небольшие окна прозрачности. На некоторых ракордах
//возможна обратная ситуация - иногда на них нанесены цветные полоски,
//что дает чередование участков прозрачности и непрозрачности.
//Чтобы предотвратить ложные срабатывание датчика, предусмотрен
//специальный таймер. Он разрешает датчику реагировать только на
//непрерывные прозрачные и непрозрачные участки с длительностью не
//меньше заданной. Длительность задается в виде параметра EndTau и может
//меняться из сервисной программы.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "endsensor.hpp"
#include "control.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TCapstan: ------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TEndSensor::TEndSensor(void)
{
  Pin_End0.DirIn(PIN_FLOAT);
  Pin_End1.DirIn(PIN_FLOAT);
  fSensorState = 0;
  fIsTape = 0;
  EndTimer = new TSoftTimer(NOM_END_TAU);
  EndTimer->Oneshot = 1;
}

//------------------------ Выполнение управления: ----------------------------

void TEndSensor::Execute(void)
{
  bool fSensor = ACSR & (1 << ACO);
  if(fSensorState != fSensor)    //если состояние датчика изменилось
  {
    fSensorState = fSensor;      //сохранение состояния
    EndTimer->Start();           //перезапуск таймера
  }
  else
  {
    if(EndTimer->Over())
    {
      fIsTape = fSensorState;
    }
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//-------------------- Чтение датчика окончания ленты: -----------------------

bool TEndSensor::Tape(void)
{
  return(fIsTape);
}

//------------------ Задание постоянной времени датчика: ---------------------

void TEndSensor::SetTau(uint16_t tau)
{
  EndTau = tau;
  EndTimer->Interval = EndTau;
}

//------------------ Чтение постоянной времени датчика: ----------------------

uint16_t TEndSensor::GetTau(void)
{
  return(EndTau);
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TEndSensor::EERead(void)
{
  EndTau = Eeprom->Rd16(EE_END_TAU, NOM_END_TAU);
  EndTimer->Interval = EndTau;
}

//------------------- Сохранение параметров в EEPROM: ------------------------

void TEndSensor::EESave(void)
{
  Eeprom->Wr16(EE_END_TAU, EndTau);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------