//----------------------------------------------------------------------------

//Модуль управления ЛПМ

//----------------------- Используемые ресурсы: ------------------------------

//Класс TTransport осуществляет управление ЛПМ.

//Используемые объекты:
//SolLift, SolBrake, SolPress - объекты управление электромагнитами;
//Capstan - объект управления ведущим двигателем;
//Spool - объект управления боковыми даигателями;
//MoveSensor - объект поддержки датчика движения ленты;
//EndSensor - объект поддержки датчик наличия ленты;
//Audio - объект управления аудиотрактом.

//Включение требуемого режима ЛПМ производится с помощью метода SetMode().
//Включение производится с помощью последовательности операций.
//Все операции реализованы как неблокирующие функции. Реализован механизм
//их последовательного выполнения, похожий на механизм реализации
//кооперативной многозадачности в Protothreads.

//Перед выполнением последовательности операций надо однократно вызвать метод
//Op->Start(), который присваивает DoNumber = 1.

//Затем в цикле надо вызывать требуемую последовательность операций.
//Перед вызовом последовательности должен быть вызов метода Op->DelayOver().
//Он возвращает true, если запрошенная какой-то из операций задержка истекла.
//Если результат false, последовательность вызываеть не надо.
//Кроме того, метод присваивает Number = 0.

//Каждая операция делает в начале проверку Op->NotDone().
//Если возвращается true, значит данная операция еще не выполнена.
//Если возвращается false, операция пропускается, как выполненная.
//Проверка делается следующим образом: ++Number == DoNumber.
//Значение Number - это порядковый номер операции в последовательности.
//Для каждой следующей вызываемой операции Number будет больше на единицу:
//для первой операции он равен 1, для второй 2, и т.д.
//Значение DoNumber - это номер операции, которая должна выполнится при
//данном вызове последовательности. При выполнении операции этот номер
//увеличивается, поэтому при следующем вызове последовательности будет
//выполнена следующая операция. Поскольку при запуске последовательности
//методом Op->Start() было присвоено DoNumber = 1, выполнение начинается
//с первой операции.

//Если операция выполняется в одно действие, то после его выполнения
//надо вызвать метод Op->Done(). Он делает DoNumber++ и присваивает
//Number = BLOCKED, чтобы следующие операции во время данного вызова
//последовательности не выполнялись (значение Number сдвигается вверх
//на большое число BLOCKED, в результате оно никогда не совпадет с DoNumber
//в ходе вызова остальных операций из последовательности).
//Если операция должна ожидать какое-то событие, то в случае его отсутствия
//вызывать Op->Done() не надо. Тогда при следующем вызове последовательности
//снова будет выполняться эта операция, и так до момента, пока не будет
//обнаружено нужное событие и не будет вызван метод Op->Done().

//Большинство операций позволяют задавать величину задержки, которая будет
//выполняться после выполнения операции. Для этого они имеют параметр del,
//для которого по умолчанию принято значение 0. Т.е. если задержка после
//операции не нужна, этот параметр при вызове можно не указывать.
//В коде операции должен быть вызов Op->StartDelay(del), который запускает
//задержку. Этот вызов можно делать по какому-то условию. Например, если
//в результате проверки выяснилось, что выполнять данную операцию не требуется
//(например, включать электромагнит не надо, если он уже включен), то
//выхов Op->StartDelay(del) можно не делать. Тогда задержка будет отработана
//только в том случае, если операция реально выполнялась.

//Отработку задержки выполняет метод Op->DelayOver(), который вызывается перед
//последовательностью операций (о нем уже говорилось выше). Выполнение задержки
//является неблокирующим. Метод всего лишь проверяет переполнение программного
//таймера, который работает на основе прерываний системного таймера.

//В конце каждой последовательности операций должна стоять специальная
//операция Op_Final(), которая делает присвоение NowMode = NewMode. Это
//блокирует дальнейшие вызовы последовательности операций. На этом всё,
//требуемый режим ЛПМ включен.

//Такой способ позволяет делать простую и наглядную запись последовательности
//действий для включения того или иного режима. Просто записываются действия
//в виде строчек, следующих друг за другом. А на самом деле их выполнение
//будет неблокирующим и будет происходить постепенно в процессе многократных
//вызовов.

//Реализованы следующие режимы работы ЛПМ:

//TR_STOP (стоп): боковые двигатели выключены, тормоза включены, ролик отведен,
//отвод ленты выключен, MUTE включен. Переход в этот режим делается с помощью
//торможения двигателями.

//TR_PAUSE (пауза): полностью совпадает с TR_STOP, только этот режим работает
//совместно с CUE = ON, поэтому MUTE выключен.

//TR_PLAYF (воспроизведение вперед): ролик подведен, ведущий вал вращается в
//прямом направлении, подмотка/подтормаживание работает, отвод ленты выключен,
//MUTE выключен. Натяжение поддерживается с каждой стороны по своему датчику.
//При старте режима кратковременно увеличивается натяжение (на величину DELTA)
//с принимающей стороны и уменьшается с подающей для уменьшения колебаний
//натяжения. В рабочем режима натяжения слева и справа могут быть установлены
//разными.

//TR_REC (запись): полностью совпадает с TR_PLAYF, только включен тракт записи.

//TR_PLAYR (воспроизведение назад): совпадает с PLAYF, только ведущий вал
//имеет другое направление вращения.

//TR_FFD (перемотка вперед): ролик отведен, отвод ленты включен,  MUTE включен.
//Правый боковой двигатель (M2) питается напряжением 100 В. Левый боковой
//двигатель (M1) регулируется натяжением справа.

//TR_REW (перемотка назад): зеркально совпадает с TR_FFD.

//TR_AFFD (архивная перемотка вперед): ролик отведен, отвод ленты включен,
//MUTE включен. Правый боковой двигатель (M2) регулируется правым датчиком,
//ограничивая натяжение ленты. Левый боковой двигатель (M1) также регулируется
//правым датчиком, создавая натяжением ленты. Натяжение для принимающего
//двигателя устанавливается больше, чем для подающего. Предусмотрен быстрый
//переход из архивной перемотки в обычную с помощью спец. режима TR_FFFD.

//TR_AREW (архивная перемотка назад): зеркально совпадает с TR_AFFD.

//TR_CAPF, TR_CAPR: вспомогательные режимы, которые реверсируют ведущий
//двигатель и переключают прямой и реверсный каналы воспроизведения.
//Переключение делается при активном сигнале MUTE для устранения щелчков.
//Эти режимы задействованы при переключении TR_PLAYF/TR_PLAYR в режиме
//паузы.

//TR_ASTOP (автостоп): аналогичен режиму TR_STOP, только торможение выпоняется
//не двигателями, а механическими тормозами.

//Кроме включения режимов ЛПМ в классе реализована логика автостопа.
//Автостоп анализирует натяжение ленты, состояние датчика окончания ленты
//и датчика движения ленты. С помощью функции CheckAutoStop() состояние логики
//автостопа может быть проверено, в случае получения true должна быть
//произведена остановка ЛПМ с помощью режима TR_ASTOP. Этот режим
//предусматривает быструю остановку с использованием механических тормозов.

//Имеются следующие опции автостопа:

//OPT_PREASENABLE разрешает автостоп при предварительном натяжении
//ленты перед включением любого режима. Если натяжение не будет обнаружено
//за время, заданное параметром AsPreDel, сработает автостоп.
//Если автостоп по пред. натяжению отключен, то пред. натяжение не производится.

//OPT_TENASENABLE разрешает автостоп по натяжению. В случае падения натяжения
//тут же сработает автостоп. Начальная задержка после включения режима задается
//параметром AsTenDel. В режиме рабочего хода контролируются оба натяжения,
//в режиме перемоток - только с приемной стороны.
//Автостоп по натяжению срабатывает максимально быстро, катушки даже
//не успевают раскрутиться после окончания ленты.

//OPT_MOVASENABLE разрешает автостоп по датчику движения ленты (ДД).
//Если движение ленты остановится, сработает автостоп. Начальная задержка
//после включения режима задается параметром AsTenDel.

//OPT_BRKASENABLE разрешает автостоп при торможении. Если торможение
//длится дольше, чем задано параметром AsBrkDel, сработает автостоп.
//Дополнительно в режиме торможения работает автостоп по натяжению,
//перед началом торможения его таймер перезапускается.

//Автостоп по датчику окончания ленты (ДО) включен всегда, кроме режима
//предварительного натяжения ленты. Начальная задержка после включения
//режима задается параметром AsTenDel.
//Логика работы автостопа по ДО следующая: он срабатывает в том случае,
//если датчик "видел" ленту, но она исчезла. Если изначально датчик ленту
//"не видит", это не мешает нормальной работе. Благодаря этому можно
//запускать аппарат с прозрачного ракорда или вести перемотку, заправив
//ленту над блоком головок. Даже в этом случае при окончании ленты будет
//останов - сработают другие виды автостопа.
//Чтобы ДО не реагировал на повреждения рабочего слоя ленты, склейки,
//полоски на ракорде, производится фильтрация сигнала датчика с постоянной
//времени Tau, которая задается в виде параметра.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "transport.hpp"
#include "control.hpp"

//----------------------------------------------------------------------------
//--------------------------- Класс TTransport: ------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TTransport::TTransport(void)
{
  Pin_LiftF_t::DirOut();
  Pin_LiftH_t::DirOut();
  Pin_BrakeF_t::DirOut();
  Pin_BrakeH_t::DirOut();
  Pin_PressF_t::DirOut();
  Pin_PressH_t::DirOut();

  SolBrake = new TSolenoid<Pin_BrakeF_t, Pin_BrakeH_t>();
  SolPress = new TSolenoid<Pin_PressF_t, Pin_PressH_t>();
  SolLift = new TSolenoid<Pin_LiftF_t, Pin_LiftH_t>();

  Capstan = new TCapstan();
  MoveSensor = new TMoveSensor();
  EndSensor = new TEndSensor();
  Audio = new TAudio();
  Spool = new TSpool();
  Op = new TOperations();
  BrakeTimer = new TSoftTimer<TT_PLAIN>();
  AutostopTimer = new TSoftTimer<TT_PLAIN>();
  NowMode = TR_STOP;
  NewMode = TR_STOP;
  ReqMode = TR_STOP;
  AsMode = AS_OFF;
  fCue = 0;
  fLowTen = 0;
}

//----------------------------------------------------------------------------
//---------------------- Выполнение управления ЛПМ: --------------------------
//----------------------------------------------------------------------------

void TTransport::Execute(void)
{
  SolLift->Execute();
  SolBrake->Execute();
  SolPress->Execute();
  Capstan->Execute();
  EndSensor->Execute();
  Spool->Execute();

  if(NowMode != NewMode)
  {
    if(ReqMode != NewMode)
    {
      //быстрый переход из AFFD в FFD:
      if((NowMode == TR_AFFD) && (NewMode == TR_FFD)) ProMode = TR_FFFD;
      //быстрый переход из AREW в REW:
        else if((NowMode == TR_AREW) && (NewMode == TR_REW)) ProMode = TR_FREW;
          else ProMode = NewMode;
      ReqMode = NewMode;
      NowMode = TR_NONE;
      Op->Start();
    }
    if(Op->DelayOver())
    {
      switch(ProMode)
      {
      case TR_STOP:
      case TR_PAUSE:
        Op_Mute(ON, 10);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Lift(OFF, 400);
        Op_Brake(OFF, 100);
        Op_Spool(SPOOL_OFF, 100);
        Op_Mute(!fCue);
        Op_Final();
        break;

      case TR_PLAYF:
      case TR_REC:
        Op_Mute(ON, 10);
        Op_Capstan(CAP_FWD);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Lift(OFF, 400);
        Op_Brake(ON, 100);
        Op_AutoStop(AS_START);
        Op_WaitTension();
        Op_WaitCapstan();
        Op_AutoStop(AS_PLAY);
        //Op_Spool(SPOOL_FORCEF);
        Op_Spool(SPOOL_PLAYF);
        Op_Press(ON, 300);
        Op_Mute(OFF);
        Op_Final();
        break;

      case TR_PLAYR:
        Op_Mute(ON, 10);
        Op_Capstan(CAP_REV);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Lift(OFF, 400);
        Op_Brake(ON, 50);
        Op_AutoStop(AS_START);
        Op_WaitTension();
        Op_WaitCapstan();
        Op_AutoStop(AS_PLAY);
        //Op_Spool(SPOOL_FORCER);
        Op_Spool(SPOOL_PLAYR);
        Op_Press(ON, 300);
        Op_Mute(OFF);
        Op_Final();
        break;

      case TR_FFD:
        Op_Mute(ON, 20);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Brake(ON, 100);
        Op_AutoStop(AS_START);
        Op_WaitTension();
      case TR_FFFD:
        Op_Lift(ON, 100);
        Op_Spool(SPOOL_FFD);
        Op_AutoStop(AS_FFD);
        Op_Final();
        break;

      case TR_REW:
        Op_Mute(ON, 20);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Brake(ON, 100);
        Op_AutoStop(AS_START);
        Op_WaitTension();
      case TR_FREW:
        Op_Lift(ON, 100);
        Op_Spool(SPOOL_REW);
        Op_AutoStop(AS_REW);
        Op_Final();
        break;

      case TR_AFFD:
        Op_Mute(ON, 20);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Brake(ON, 100);
        Op_AutoStop(AS_START);
        Op_WaitTension();
        Op_Lift(ON, 100);
        Op_Spool(SPOOL_AFFD, 100);
        Op_AutoStop(AS_FFD);
        Op_Mute(!fCue);
        Op_Final();
        break;

      case TR_AREW:
        Op_Mute(ON, 20);
        Op_Press(OFF, 100);
        Op_Spool(SPOOL_BRAKE);
        Op_AutoStop(AS_BRAKE);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Brake(ON, 100);
        Op_AutoStop(AS_START);
        Op_WaitTension();
        Op_Lift(ON, 100);
        Op_Spool(SPOOL_AREW, 100);
        Op_AutoStop(AS_REW);
        Op_Mute(!fCue);
        Op_Final();
        break;

      case TR_CAPF:
        Op_Mute(ON, 10);
        Op_Capstan(CAP_FWD, 10);
        Op_WaitCapstan();
        Op_Mute(!fCue);
        Op_Final();
        break;

      case TR_CAPR:
        Op_Mute(ON, 10);
        Op_Capstan(CAP_REV);
        Op_WaitCapstan();
        Op_Mute(!fCue);
        Op_Final();
        break;

      case TR_ASTOP:
        Op_Mute(ON, 10);
        Op_Press(OFF);
        Op_Spool(SPOOL_BRAKE);
        Op_Brake(OFF);
        Op_WaitStop();
        Op_AutoStop(AS_OFF);
        Op_Lift(OFF, 50);
        Op_Spool(SPOOL_OFF);
        Op_Final(TR_STOP);
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
//--------------------- Операции для управления ЛПМ: -------------------------
//----------------------------------------------------------------------------

//----------- Включение ведущего двигателя в заданном направлении: -----------

void TTransport::Op_Capstan(bool rev, uint16_t del)
{
  if(Op->NotDone())
  {
    if(Capstan->Reverse() != rev)
    {
      Capstan->Start(rev);
      Audio->Rev(rev);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//------------------- Ожидание разгона ведущего двигателя: -------------------

void TTransport::Op_WaitCapstan(void)
{
  if(Op->NotDone())
  {
    if(Capstan->Ready())
    {
      Op->Done();
    }
    /*
    else
    {
      if(Spool->GetMode() != SPOOL_BRAKE)
        Spool->SetMode(SPOOL_BRAKE);
    }
    */
  }
}

//--------------- Управление электромагнитом прижимного ролика: --------------

void TTransport::Op_Press(bool s, uint16_t del)
{
  if(Op->NotDone())
  {
    if(SolPress->Hold() != s)
    {
      SolPress->OnOff(s);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//------------------- Управление электромагнитом тормозов: -------------------

void TTransport::Op_Brake(bool s, uint16_t del)
{
  if(Op->NotDone())
  {
    if(SolBrake->Hold() != s)
    {
      SolBrake->OnOff(s);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//----------------- Управление электромагнитом отвода ленты: -----------------

void TTransport::Op_Lift(bool s, uint16_t del)
{
  if(Op->NotDone())
  {
    if(fCue) s = OFF;
    if(SolLift->Hold() != s)
    {
      SolLift->OnOff(s);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//------------------ Включение режима боковых двигателей: --------------------

void TTransport::Op_Spool(uint8_t m, uint16_t del)
{
  if(Op->NotDone())
  {
    bool deal = Spool->GetMode() != m;
    if(Spool->GetMode() != m)
    {
      Spool->SetMode(m);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//------------------------ Ожидание остановки ленты: -------------------------

void TTransport::Op_WaitStop(void)
{
  if(Op->NotDone())
  {
    if(!MoveSensor->Move() || fLowTen)
    {
      Op->Done();
    }
    /*
    else
    {
      if(Spool->GetMode() != SPOOL_BRAKE)
        Spool->SetMode(SPOOL_BRAKE);
    }
    */
  }
}

//----------------------- Установка режима автостопа: ------------------------

void TTransport::Op_AutoStop(uint8_t m, uint16_t del)
{
  if(Op->NotDone())
  {
    SetAutoStop(m);
    Op->StartDelay(del);
    Op->Done();
  }
}

//------------------------ Ожидание натяжения ленты: -------------------------

void TTransport::Op_WaitTension(void)
{
  if(Op->NotDone())
  {
    if((!Spool->LowT1() &&
        !Spool->LowT2()) ||
        !Option(OPT_PREASENABLE))
    {
      Op->Done();
    }
    /*
    else
    {
      if(Spool->GetMode() != SPOOL_BRAKE)
        Spool->SetMode(SPOOL_BRAKE);
    }
    */
  }
}

//------------------------ Управление сигналом MUTE: -------------------------

void TTransport::Op_Mute(bool m, uint16_t del)
{
  if(Op->NotDone())
  {
    if((bool)(Audio->GetState() & AUD_MUTE) != m)
    {
      Audio->Mute(m);
      Op->StartDelay(del);
    }
    Op->Done();
  }
}

//------------------------------- Задержка: ----------------------------------

void TTransport::Op_Delay(uint16_t del)
{
  if(Op->NotDone())
  {
    Op->StartDelay(del);
    Op->Done();
  }
}

//--------------------- Пропуск n шагов, если b = true: ----------------------

void TTransport::Op_SkipIf(bool b, uint8_t n)
{
  if(Op->NotDone())
  {
    if(b)
      for(uint8_t i = 0; i < n; i++)
        Op->Done();
    Op->Done();
  }
}

//--------------------- Переход на метку внутри процесса: --------------------

void TTransport::Op_GoIf(bool b, uint8_t m)
{
  if(b)
  {
    NewMode = m;
  }
}

//----------------------- Завершение включения режима: -----------------------

void TTransport::Op_Final(uint8_t m)
{
  if(Op->NotDone())
  {
    ReqMode = TR_NONE;
    if(m != TR_NONE)
      NewMode = m;
    NowMode = NewMode;
    Op->Done();
  }
}

//----------------------------------------------------------------------------
//--------------------------- Внутренние методы: -----------------------------
//----------------------------------------------------------------------------

//-------------------------- Включение автостопа: ----------------------------

void TTransport::SetAutoStop(uint8_t m)
{
  fAsBrake = OFF;
  switch(m)
  {
  case AS_OFF:
    AsMode = m;
    break;
  case AS_START:
    AutostopTimer->Start(AsPreDel); //запуск таймера пред. натяжения
    AsMode = m;
    break;
  case AS_PLAY:
    AutostopTimer->Start(AsTenDel); //запуск таймера натяжения
    fTape = EndSensor->Tape();      //запоминание состояния датчика ленты
    MoveSensor->SetTC(MOVE_SLOW);   //управление TC
    AsMode = m;
    break;
  case AS_FFD:
  case AS_REW:
    AutostopTimer->Start(AsTenDel); //запуск таймера натяжения
    fTape = EndSensor->Tape();      //запоминание состояния датчика ленты
    MoveSensor->SetTC(MOVE_FAST);   //управление TC
    AsMode = m;
    break;
  case AS_BRAKE:
    BrakeTimer->Start(AsBrkDel);    //запуск таймера торможения
    AutostopTimer->Start(AsTenDel); //перезапуск таймера автостопа по натяжению
    fAsBrake = ON;                  //флаг включения автостопа по торможению
    //нет изменения AsMode, т.е. продолжает работать текущий режим автостопа
    break;
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//-------------------- Включение требуемого режима ЛПМ: ----------------------

void TTransport::SetMode(uint8_t mode)
{
  NewMode = mode;
  if((mode != TR_CAPF) && (mode != TR_CAPR))
    SetCue(OFF);
}

//--------------------- Чтение включенного режима ЛПМ: -----------------------

uint8_t TTransport::GetMode(void)
{
  return(NowMode);
}

//------------------- Управление отводом ленты и MUTE: -----------------------

void TTransport::SetCue(bool cue)
{
  if(cue) //CUE ON
  {
    if((NewMode == TR_AFFD) || (NewMode == TR_AREW))
      SolLift->OnOff(OFF);
    if(NowMode == NewMode) Audio->Mute(OFF);
  }
  else //CUE OFF
  {
    if((NewMode == TR_AFFD) || (NewMode == TR_AREW))
      SolLift->OnOff(ON);
    Audio->Mute(ON);
  }
  SetAutoStop(AsMode); //перезапуск таймера автостопа
  fCue = cue;
}

//------------ Чтение состояния модулей, влияющих на индикацию: --------------

uint8_t TTransport::GetState(void)
{
  uint8_t state = 0;
  if(Capstan->Running()) state |= TRS_CAP;    //capstan вращается
  if(Capstan->Reverse()) state |= TRS_REV;    //capstan в режиме реверс
  if(Capstan->Ready())   state |= TRS_LOCK;   //capstan разогнался
  if(Spool->GetMode() == SPOOL_BRAKE)
                         state |= TRS_BRAKE;  //торм. двигателями
  if(EndSensor->Tape())  state |= TRS_TAPE;   //лента загружена
  if(MoveSensor->Move()) state |= TRS_MOVE;   //лента движется
  if(fCue)               state |= TRS_CUE;    //режим обзора
  if(fLowTen)            state |= TRS_LOWTEN; //нет натяжения
  return(state);
}

//--------------------- Проверка состояния автостопа: ------------------------

bool TTransport::CheckAutoStop(void)
{
  bool fas = 0;
  switch(AsMode)
  {
  //автостоп по пред. натяжению 1 и 2:
  case AS_START:
    fLowTen = Spool->LowT1T2();   //проверка натяжений слева и справа
    if(fLowTen &&  Option(OPT_PREASENABLE)) //если автостоп разрешен,
      fas = 1;                    //автостоп (при усл. переполн. таймера)
    break;
  //автостоп по натяжению 1 и 2:
  case AS_PLAY:
    fLowTen = Spool->LowT1T2();   //проверка натяжений слева и справа
    if(fLowTen && Option(OPT_TENASENABLE)) //если автостоп разрешен, то
      fas = 1;                    //автостоп (при усл. переполн. таймера)
    break;
  //автостоп по натяжению 2:
  case AS_FFD:
    fLowTen = Spool->LowT2();     //проверка натяжения справа
    if(fLowTen && Option(OPT_TENASENABLE)) //если автостоп разрешен, то
      fas = 1;                    //автостоп (при усл. переполн. таймера)
    break;
  //автостоп по натяжению 1:
  case AS_REW:
    fLowTen = Spool->LowT1();     //проверка натяжения слева
    if(fLowTen && Option(OPT_TENASENABLE)) //если автостоп разрешен, то
      fas = 1;                    //автостоп (при усл. переполн. таймера)
    break;
  default:
    fLowTen = Spool->LowT1T2();   //проверка натяжений слева и справа
  };
  if(fas && AutostopTimer->Over()) //если fas = 1 и интервал истек, то
    return(1);                     //автостоп

  //автостоп при торможении:
  if(fAsBrake)
  {
    if(MoveSensor->Move() &&      //если лента не остановилась,
       BrakeTimer->Over() &&      //интервал торможения истек и
       Option(OPT_BRKASENABLE))   //автостоп при торможении разрешен, то
         return(1);               //автостоп
  }

  if(AsMode > AS_START)
  {
    //автостоп по ДО:
    bool t = EndSensor->Tape();
    if(fTape && !t) return(1);    //если лента была и пропала, автостоп
    fTape = t;
    //атостоп по ДД:
    if(!MoveSensor->Move() &&     //если лента не движется,
       !fAsBrake &&               //это не автостоп при торможении,
       AutostopTimer->Over() &&   //таймер автостопа переполнился и
       Option(OPT_MOVASENABLE))   //автостоп по ДД разрешен, то
         return(1);               //автостоп
  }
  return(0);
}

//----------------- Установка времени старта электромагнитов: ----------------

void TTransport::SetEmForce(uint16_t t)
{
  EmForce = t;
  SolBrake->Force(EmForce);
  SolPress->Force(EmForce);
  SolLift->Force(EmForce);
}

//----------------- Чтение времени старта электромагнитов: -------------------

uint16_t TTransport::GetEmForce(void)
{
  return(EmForce);
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TTransport::EERead(void)
{
  Options  = Eeprom->Rd8(EE_TR_OPTIONS, NOM_TR_OPTIONS);
  AsBrkDel = Eeprom->Rd16(EE_AS_BRK_DEL, NOM_AS_BRK_DEL);
  AsPreDel = Eeprom->Rd16(EE_AS_PRE_DEL, NOM_AS_PRE_DEL);
  AsTenDel = Eeprom->Rd16(EE_AS_TEN_DEL, NOM_AS_TEN_DEL);
  EmForce  = Eeprom->Rd16(EE_EM_FORCE, NOM_EM_FORCE);
  SetEmForce(EmForce);
  Capstan->EERead();
  Spool->EERead();
  EndSensor->EERead();
}

//------------------- Сохранение параметров в EEPROM: ------------------------

void TTransport::EESave(void)
{
  Eeprom->Wr8(EE_TR_OPTIONS, Options);
  Eeprom->Wr16(EE_AS_BRK_DEL, AsBrkDel);
  Eeprom->Wr16(EE_AS_PRE_DEL, AsPreDel);
  Eeprom->Wr16(EE_AS_TEN_DEL, AsTenDel);
  Eeprom->Wr16(EE_EM_FORCE, EmForce);
  Capstan->EESave();
  Spool->EESave();
  EndSensor->EESave();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
