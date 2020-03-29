//----------------------------------------------------------------------------

//Модуль декодера команд ИК ДУ

//----------------------- Используемые ресурсы: ------------------------------

//Используется пульт ДУ с кодом RC-5 (тип микросхемы контроллера - SAA3010).
//Декодер использует два прерывания: внешнее INT0 от фотоприемника и
//прерывание по переполнению таймера 2.

//После того, как обнаружен стартовый бит (переход из единицы
//в ноль на входе прерывания), в обработчике внешнего прерывания
//разрешается прерывание таймера 2 и загружается интервал до первой
//выборки T_SAMPLE / 2 (состояние ST_SMP2).

//В прерывании таймера делается выборка для второй пловины текущего бита
//(первая половина первого бита посылки всегда равна 1). По значению выборки
//второй половины бита определяется значение принятого бита.
//Для проверки корректности кода Манчестер значения выборок для первой и второй
//половины бита сравниваются. Если они равны, значит произошла ошибка,
//и прием посылки начинается с начала (переход в состояние ST_END).
//После выборки второй половины бита в таймер загружается интервал T_SAMPLE
//для выборки первой половины следующего бита (состояние ST_SMP1).

//В следующем прерывании таймера делается выборка первой половины следующего
//бита. Затем снова разрешается внешнее прерывание и ожидается очередной
//переход входного сигнала. Если он не наступает через время T_SAMPLE,
//то возникает прерывание таймера (ошибка таймаута), и прием посылки
//начинается сначала (переход в состояние ST_END).
//В этом же прерывании таймера проверяется счетчик принятых битов.
//Если принята вся посылка, код системы записывается в переменную System,
//а код команды - в переменную Command.

//Коды кнопок перекодируются по таблице, которая содержится в файле
//remcodes.hpp. В первой строчке этой таблицы задается номер системы RC-5.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "remote.hpp"
#include "remcodes.hpp"

//----------------------------------------------------------------------------
//---------------------------- Класс TRemote: --------------------------------
//----------------------------------------------------------------------------

//------------------------ Статические переменные: ---------------------------

uint8_t volatile TRemote::State;
uint8_t volatile TRemote::System;
uint8_t volatile TRemote::Command;

//------------- Вспомогательные inline-функции (замена макросам): ------------

inline void TRemote::TmrIntEnable(uint8_t delay)
{
  TCNT0 = delay;
  TIFR = (1 << TOV0);
  TIMSK |= (1 << TOIE0);
}

inline void TRemote::TmrIntDisable(void)
{
  TIMSK &= ~(1 << TOIE0);
}

inline void TRemote::ExtIntEnable(void)
{
  EIFR = (1 << INTF4);
  EIMSK |= (1 << INT4);
}

inline void TRemote::ExtIntDisable(void)
{
  EIMSK &= ~(1 << INT4);
}

//----------------------------- Конструктор: ---------------------------------

TRemote::TRemote(void)
{
  Pin_IR.DirIn(PIN_PULL);            //настройка пина
  RemTimer = new TSoftTimer<TT_ONESHOT>(REP_TM); //таймер повторения посылок
  //RemTimer->Oneshot = 1;             //режим одновибратора
  Command = RC5_NO;                  //очистка кода команды
  Prev_Cmd = RC5_NO;                 //очистка предыдущей команды
  TCCR0 = (1 << CS02);               //прескалер CK/64 для таймера 0
  EICRB = (1 << ISC40);              //настройка INT4 по перепаду
  State = ST_END;                    //будет подготовка к приему посылки
  TmrIntEnable(DEL2);                //разрешение прерывания таймера
}

//------------- Обработчик прерывания по сигналу фотоприемника: --------------

#pragma vector = INT4_vect
__interrupt void EdgeIR(void)
{
  TRemote::ExtIntDisable();             //запрещение внешнего прерывания
  TRemote::TmrIntEnable(TRemote::DEL1); //разрешение прерывания таймера
  TRemote::State = TRemote::ST_SMP2;    //будет выборка второй половины бита
}

//------------------ Обработчик прерывания таймера 0: ------------------------

#pragma vector = TIMER0_OVF_vect
__interrupt void TimerIR(void)
{
  static uint8_t BitCnt;        //счетчик битов посылки
  static bool fPreVal;          //значение первой половины бита
  static union
  {
    uint16_t RC5Code;           //принимаемый код
    TRemote::RC5Struct RC5Bits; //битовые поля принимаемого кода
  };

  bool fVal = TRemote::Pin_IR;  //чтение значения бита

  //прерывание таймера в случае таймаута:
  if(TRemote::State == TRemote::ST_TO)
  {
    TRemote::State = TRemote::ST_END; //ошибка таймаута
  }

  //выборка сделана на первой половине бита:
  if(TRemote::State == TRemote::ST_SMP1)
  {
    fPreVal = fVal;          //сохранение значения первой половины бита
    TRemote::ExtIntEnable(); //разрешение внешнего прерывания
    TRemote::TmrIntEnable(TRemote::DEL2); //разрешение прерывания таймера
    TRemote::State = TRemote::ST_TO; //будет таймаут в случае прерывания
    //проверка количества принятых битов:
    if(!--BitCnt)
    {
      TRemote::System = RC5Bits.RC5System;   //сохранение номера системы
      TRemote::Command = RC5Bits.RC5Command; //сохранение кода команды
      if(RC5Bits.RC5Control)                 //если RC5Control = 1,
        TRemote::Command |= TRemote::RC5_CB; //добавление бита RC5_CB
      TRemote::State = TRemote::ST_END;      //прием закончен
    }
  }

  //выборка сделана на второй половине бита:
  if(TRemote::State == TRemote::ST_SMP2)
  {
    //проверка кода Манчестер:
    if(fVal != fPreVal)
    {
      RC5Code <<= 1;          //сдвиг принятого кода
      if(!fVal) RC5Code |= 1; //вторая половина бита = 0, бит = 1
      TRemote::TmrIntEnable(TRemote::DEL2); //разрешение прерывания таймера
      TRemote::State = TRemote::ST_SMP1; //будет выборка первой половины бита
    }
    else
    {
      TRemote::State = TRemote::ST_END; //ошибка кода Манчестер
    }
  }

  //прием закончен (или ошибка):
  if(TRemote::State == TRemote::ST_END)
  {
    BitCnt = TRemote::RC5_LENGTH; //загрузка счетчика битов
    fPreVal = 1;                  //перед стартовым битом всегда идет единица
    TRemote::TmrIntDisable();     //запрещение прерываний таймера
    TRemote::ExtIntEnable();      //разрешение внешнего прерывания
  }
}

//----------------------- Формирование кода кнопки: --------------------------

uint8_t TRemote::GetCode(void)
{
  uint8_t res = RC5_NO;              //результат
  uint8_t c = Command;               //чтение команды
  uint8_t s = System;                //чтение системы
  if(c != RC5_NO && s == *RC5_Codes) //принята команда ДУ
  {
    Command = RC5_NO;                //очистка кода команды
    RemTimer->Start();               //перезапуск таймера
    if(c == Prev_Cmd)                //если повтор,
      res = c;                       //формирование выходного кода
  }
  else                               //команды не поступало
  {
    c = Prev_Cmd;                    //прежний код команды
    if(RemTimer->Over())             //если сработал таймер,
      c = RC5_NO;                    //сброс кода команды
        else                         //иначе удержание
          res = c;                   //выходного кода
  }
  Prev_Cmd = c;                      //сохранение предыдущего кода
  res = res & ~RC5_CB;               //удаление CB
  if(res > RC5_MAX) res = NC;        //нет нажатия
    else res = RC5_Codes[res + 1];   //перекодировка
  return(res);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
