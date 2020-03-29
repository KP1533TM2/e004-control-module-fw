//----------------------------------------------------------------------------

//Project:  E-004 control
//Compiler: IAR EWARM 6.12
//Target:   ATmega64

//----------------------- Используемые ресурсы: ------------------------------

//Плата блока управления (БУ) предназначена для замены штатной платы БУ
//магнитофона "Электроника-004". Новая плата БУ устанавливается взамен старой
//и не требует никаких переделок магнитофона.
//Плата может быть установлена и в аппараты "Электроника-003" и "Олимп-003", но
//может потребоваться изменение рапайки разъемов (зависит от версии аппарата).

//Плата БУ обеспечивает:
//- управление режимами магнитофона с помощью кнопок и пульта ДУ (RC-5);
//- индикацию режимов светодиодами;
//- управление боковыми двигателями;
//- стабилизацию натяжения ленты с помощью программных ПИД-регуляторов;
//- задание номинального натяжения ленты с компьютера;
//- управление электромагнитами;
//- формирование форсирующего импульса для каждого электромагнита;
//- регулировку тока удержания подстроечными резисторами для каждого
//  электромагнита;
//- стабилизацию тока удержания, что устраняет пульсации с сетевой частотой и
//  снижает наводки от электромагнитов;
//- управление аналоговыми узлами магнитофона;

//Структура программы:
//
//                 [Main]
//                    |
//             |------------|
//           [Port]     [Control]
//                          |
//    |--------|--------|-------|------|---------|
// [Eeprom] [Local] [Remote] [Sound] [Leds] [Transport]
//                                               |
//    |---------|---------|----------|-----------|---------|
// [Capstan] [Spool] [Solenoid] [MoveSensor] [EndSensor] [Audio]

//----------------------------------------------------------------------------

#include "main.hpp"
#include "control.hpp"
#include "port.hpp"

TControl *Control;
TPort *Port;

//----------------------------------------------------------------------------
//------------------------- Основная программа: ------------------------------
//----------------------------------------------------------------------------

int main(void)
{
  TSysTimer::Init();        //инициализация системного таймера
  Control = new TControl(); //создание объекта управления
  Port = new TPort();       //создание объекта порта

  do                        //основной цикл
  {
    TSysTimer::Sync();      //синхронизация системных тиков с основным циклом
    Control->Execute();     //выполнение задачи управления
    Port->Execute();        //выполнение команд компьютера
  }
  while(1);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
