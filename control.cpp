//----------------------------------------------------------------------------

//Модуль общего управления

//----------------------- Используемые ресурсы: ------------------------------

//Класс TControl осуществляет общее управление.

//Используемые объекты:
//Eeprom - объект управления энергонезависимой памятью;
//Local - объект местной клавиатуры;
//Remote - объект ИК ДУ;
//Sound - объект управления бипером;
//Transport - объект управления ЛПМ;
//Leds - объект управления светодиодами;

//Класс реализует логику управления аппаратом с местной клавиатуры
//и пульта ДУ. Индикация режимов светодиодами обновляется, когда
//fUpdate = 1 или обновилось значение, возвращаемое Transport->GetState().
//Индикация производится на основе анализа Mode, Transport->GetState() и
//флагов fPause, fRoll.

//Задание параметров и опций производится с компьютера. Для этого на плате БУ
//имеется разъем, к которому можно подключиь любой переходник USB-TTL.
//Все параметры могут быть сохранены в EEPROM и использоваться в дальнейшем
//при автономной работе.

//Особенности работы логики упрвления:
//- светодиод STOP горит при загруженной ленте (нет сигнала ДО), если
//ленты нет - не горит. Во время торможения двигателями светодиод STOP
//быстро мигает.
//- светодиод PLAY быстро мигает во время разгона ведущего двигателя.
//Когда разгон завершен - мигает медленно, индицируя текущее направление
//вращения. При работе в режиме PLAY - горит постоянно. В режиме
//CUE (откат или архивная перемотка без отвода ленты от головок) быстро
//мигает.
//- светодиоды FFD и REW мигают во время архивной перемотки. Во время
//обычной перемотки горят постоянно.
//- светодиод PAUSE может гореть или мегать в режиме паузы, это выбирается
//опцией OPT_PAUSELEDBLINK.
//- кнопка PAUSE работает как триггер: первое нажатие включает паузу,
//следующее выключает. Выключать паузу может также кнопка PLAY, если
//задана опция OPT_PLAYOFFPAUSE.
//- кнопка REC срабатывает только при совместном нажатии REC + PLAYF.
//При этом сразу начинается запись. Если задана опция OPT_AUTORECPAUSE,
//то при включении REC автоматически включается пауза. Запустить запись
//можно нажатием кнопки PAUSE или PLAYF (если задана опция OPT_PLAYOFFPAUSE).
//- кнопка FFD и REW включает перемотку вперед и назад соответственно.
//Если задана опция OPT_USEARCHIVE, то сначала включается архивная перемотка
//(светодиод перемотки при этом мигает), а следующим нажатием - обычная
//(светодиод перемотки при этом горит). Еще одно нажатие снова возвращает
//архивную перемотку через остановку ленты.
//- кнопка ROLLBACK включает откат в режиме воспроизведения.
//Лента перематывается в обратном направлении, пока удерживается кнопка.
//Если задана опция OPT_USEARCHIVE, то используется архивная перемотка,
//иначе - обычная. Если задана опция OPT_ROLLCUE, то при откате лента не
//будет отводиться от головок (только если используется архивная перемотка).
//Если задана опция OPT_ENABLECUE, то нажатие кнопки ROLLBACK будет подводить
//ленту к головкам в режиме архивной перемотки. При отпусканнии кнопки лента
//снова будет отводиться.
//- пульт ДУ вместо двух кнопок PLAYF и PLAYR имеет одну кнопку PLAY и кнопку
//REVERSE. Текущее направление вращения ведущего вала индицируется мигающими
//светодиодами PLAY. Именно в этом направление будет запущен ЛПМ при нажатии
//кнопки PLAY на пульте ДУ. Кнопкой REVERSE можно поменять направление движения
//ленты или направление вращения ведущего вала (в режиме STOP или PAUSE).
//- на пульте ДУ предусмотрена кнопка MUTE, которой в любой момент можно
//включить блокировку УВ.
//- при нажатии на кнопки генерируется звуковой сигнал (короткий beep).
//Его можно отключить, если выбрать опцию OPT_NOSOUND.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "control.hpp"

TEeprom *Eeprom;

//----------------------------------------------------------------------------
//---------------------------- Класс TControl: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TControl::TControl(void)
{
  HardwareInit();
  Eeprom = new TEeprom();
  Local = new TLocal();
  Remote = new TRemote();
  KeyCode = KEY_NO;
  Sound = new TSound();
  Transport = new TTransport();
  Leds = new TLeds();
  ProgTimer = new TSoftTimer<TT_ONESHOT>(T_PROG);
  TrState = Transport->GetState();
  ServiceCounter = 0;
  fAutostop = 0;
  ProgMode1 = TR_STOP;
  ProgMode2 = TR_STOP;
  Program = PR_OFF;
  Mode = TR_STOP;                //текущий режим STOP
  BackMode = TR_STOP;            //режим возврата - STOP
  fPause = 0;                    //PAUSE выключена
  fRoll = 0;                     //ROLLBACK выключен
  fUpdate = 1;                   //требование обновления индикации
  EERead();                      //чтение EEPROM
  Leds->Set(LED_ALL, LEDS_CONT); //включение всех светодиодов для теста
  Leds->Execute();               //обновление
  TSysTimer::Delay_ms(1000);     //задержка 1 сек.
  Leds->Set(LED_ALL, LEDS_OFF);  //выключение всех светодиодов
  Leds->Execute();               //обновление
  Sound->Beep();                 //начальный звуковой сигнал
}

//------------------------ Выполнение управления: ----------------------------

void TControl::Execute(void)
{
  Local->Execute();
  Remote->Execute();
  Sound->Execute();
  Transport->Execute();

  //считывание кода кнопки:
  KeyCode = Local->GetKeyCode();
  //считывание кода ИК ДУ:
  if(KeyCode == KEY_NO)
    KeyCode = Remote->GetKeyCode();
  //фильтрация кодов кнопок:
  FilterKey(KeyCode);
  //обработка нажатий кнопок:
  if(KeyCode != KEY_NO)
    SetMode(KeyCode);
  //сервисы:
  if(ServiceTimer())   //таймер сервисов
  {
    LedsService();     //сервис светодиодов
    AutoStopService(); //сервис автостопа
    AutoPlayService(); //сервис программного режима и автореверса
  }
}

//----------------------------------------------------------------------------
//--------------------------- Внутренние методы: -----------------------------
//----------------------------------------------------------------------------

//----------------------- Инициализация оборудования: ------------------------

inline void TControl::HardwareInit(void)
{
  Pin_Pvg.DirIn(PIN_PULL);
  //настройка неиспользуемых пинов:
  Pin_Dir.DirOut();
  Pin_SCL.DirIn(PIN_PULL);
  Pin_SDA.DirIn(PIN_PULL);
  Pin_RXD1.DirIn(PIN_PULL);
  Pin_TXD1.DirOut(PIN_HIGH);
  Pin_PB1.DirIn(PIN_PULL);
#ifdef REV_A
  Pin_XT1.DirOut();
  Pin_XT2.DirOut();
  Pin_XT3.DirOut();
  Pin_XT4.DirOut();
  Pin_XT5.DirOut();
  Pin_XT6.DirOut();
#endif
#ifdef REV_C
  Pin_AdcEnd.DirIn();
  Pin_Aux.DirIn();
#endif
  //настройка watchdog-таймера (260 ms):
  __watchdog_reset();
  WDTCR = (1 << WDCE) | (1 << WDE);
  WDTCR = (1 << WDE) | (1 << WDP2);
}

//----------------------- Фильтрация кодов кнопок: ---------------------------

inline void TControl::FilterKey(uint8_t &code)
{
  if(code != KEY_NO)
  {
    //перекодировка для кнопки PLAY (RC only):
    if(code == KEY_PLAY)
      code = Trs(TRS_REV)? KEY_PLAYR : KEY_PLAYF;
    //удаление событий отпускания кнопок:
    if(code & MSG_REL)
    {
      //перекодировка отпускания кнопки ROLL:
      if(code == (KEY_ROLL | MSG_REL))
        code = KEY_UNROLL;
          else code = KEY_NO;
    }
    //удаление событий удержания кнопок:
    if(code & MSG_HOLD)
    {
      //перекодировка удержания кнопки STOP:
      if(code == (KEY_STOP | MSG_HOLD))
        code = KEY_PROG;
          else code = KEY_NO;
    }
  }
}

//-------------- Формирование кода перемотки с учетом архивной: --------------

uint8_t TControl::Arch(uint8_t code)
{
  if(code == TR_FFD) return(Option(OPT_USEARCHIVE)? TR_AFFD : TR_FFD);
  if(code == TR_REW) return(Option(OPT_USEARCHIVE)? TR_AREW : TR_REW);
  return(code);
}

//-------------------------- Переключение режимов: ---------------------------

void TControl::SetMode(uint8_t code)
{
  Sound->Tick();
  //программирование:
  ProgMode(code);
  switch(code)
  {
  //включение режима STOP
  case KEY_STOP:
    Mode = TR_STOP;
    fPause = 0;                          //выключение паузы
    Transport->Audio->Rec(OFF);          //выключение тракта записи
    Transport->SetMode(Mode);            //включение режима
    break;

  //включение режима PLAYF
  case KEY_PLAYF:
    if(Mode == TR_REC)
    {
      if(Option(OPT_RECPLAYEN))
      {
        Mode = TR_PLAYF;                 //включение PLAYF
        Transport->Audio->Rec(OFF);      //выключение тракта записи
      }
    }
    else Mode = TR_PLAYF;                //включение PLAYF
    if(Option(OPT_PLAYEXPAUSE))
      fPause = 0;                        //выключение паузы
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
    else
    {
      Transport->SetMode(TR_CAPF);       //направление вращения тонвала
      Transport->SetCue(ON);             //выключение MUTE
    }
    break;

  //включение режима PLAYR
  case KEY_PLAYR:
    if(Mode == TR_REC)
    {
      if(Option(OPT_RECPLAYEN))
      {
        Mode = TR_PLAYR;                 //включение PLAYR
        Transport->Audio->Rec(OFF);      //выключение тракта записи
      }
    }
    else
    {
      Mode = TR_PLAYR;
      if(Option(OPT_PLAYEXPAUSE))
        fPause = 0;                      //выключение паузы
    }
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
    else
    {
      Transport->SetMode(TR_CAPR);       //направление вращения тонвала
      Transport->SetCue(ON);             //выключение MUTE
    }
    break;

  //включение режима REC
  case KEY_REC:
    if((Mode == TR_STOP) ||
       ((Mode == TR_PLAYF) &&
        (fPause || Option(OPT_PLAYRECEN))))
    {
      Mode = TR_REC;
      if(Option(OPT_AUTORECPAUSE))
        fPause = 1;                      //включение паузы
      Transport->Audio->Rec(ON);         //включение тракта записи
      if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
        else Transport->SetMode(TR_CAPF); //направление вращения тонвала
    }
    break;

  //включение режима PAUSE
  case KEY_PAUSE:
    fPause = !fPause;                    //включение/выключение режима паузы
    if(fPause)                           //если пауза включается, то в режиме
    {
      if((Mode != TR_PLAYF) &&
         (Mode != TR_PLAYR) &&
         (Mode != TR_REC))
        Mode = TR_STOP;
      Transport->SetMode(TR_PAUSE);      //остановка ленты без MUTE
      if(!Option(OPT_MUTEPAUSE))
        Transport->SetCue(ON);           //выключение MUTE
    }
    else                                 //если пауза выключается,
      Transport->SetMode(Mode);          //возобновление режима
    break;

  //включение режима AFFD или FFD
  case KEY_FFD:
    fPause = 0;                          //выключение паузы
    if(Mode == TR_AFFD)                  //если режим арх. перемотки,то
      Mode = TR_FFD;                     //включение обычной перемотки
        else Mode = Arch(TR_FFD);        //иначе включ. арх., если разрешена
    Transport->Audio->Rec(OFF);          //выключение тракта записи
    Transport->SetMode(Mode);            //включение режима
    break;

  //включение режима AREW или REW
  case KEY_REW:
    fPause = 0;                          //выключение паузы
    if(Mode == TR_AREW)                  //если режим арх. перемотки,то
      Mode = TR_REW;                     //включение обычной перемотки
        else Mode = Arch(TR_REW);        //иначе включ. арх., если разрешена
    Transport->Audio->Rec(OFF);          //выключение тракта записи
    Transport->SetMode(Mode);            //включение режима
    break;

  //включение режима ROLLBACK
  case KEY_ROLL:
    if(Mode == TR_REC)                   //если запись,
    {
      Transport->Audio->Rec(OFF);        //выключение тракта записи
      Mode = TR_PLAYF;                   //переход в режим PLAYF
    }
    if(Mode == TR_PLAYF)                 //если PLAYF,
    {
      BackMode = Mode;                   //запоминание режима возврата
      Mode = Arch(TR_REW);               //включение перемотки
      fRoll = 1;                         //флаг отката
    }
    else if(Mode == TR_PLAYR)            //если PLAYR,
    {
      BackMode = Mode;                   //запоминание режима возврата
      Mode = Arch(TR_FFD);               //включение перемотки
      fRoll = 1;                         //флаг отката
    }
    else if((Mode == TR_AFFD) ||         //если AFFD или
            (Mode == TR_AREW))           //AREW
    {
      if(Option(OPT_ENABLECUE))          //если опция обзор при арх. перем., то
        Transport->SetCue(ON);           //управление отводом ленты и MUTE
    }
    if(fRoll)
    {
      Transport->SetMode(Mode);          //включение режима
      if(Option(OPT_ROLLCUE))            //если опция обзора при откате, то
        Transport->SetCue(ON);           //управление отводом ленты и MUTE
    }
    break;

  //выключение режима ROLLBACK
  case KEY_UNROLL:
    if(fRoll)                            //если включен откат, то
    {
      Mode = BackMode;                   //переход в режим возврата
      fRoll = 0;                         //выключение отката
    }
    Transport->SetMode(Mode);            //включение режима
    break;

  //RC only:

  //реверс
  case KEY_REV:
    if(Mode == TR_REC) break;            //если включен режим записи, выход
    if(Mode == TR_PLAYF)                 //если режим PLAYF, то
    {
      Mode = TR_PLAYR;                   //режим PLAYR
      if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
        else Transport->SetMode(TR_CAPR); //иначе реверс тонвала
    }
    else if(Mode == TR_PLAYR)            //если режим PLAYR, то
    {
      Mode = TR_PLAYF;                   //режим PLAYF
      if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
        else Transport->SetMode(TR_CAPF); //иначе реверс тонвала
    }
    else if(Trs(TRS_REV))                 //иначе реверс тонвала
      Transport->SetMode(TR_CAPF);
        else Transport->SetMode(TR_CAPR);
    break;

  //включение/выключение режима Mute
  case KEY_MUTE:
    Transport->Audio->MuteToggle();      //включение/выключение MUTE
    break;
  };
  fUpdate = 1;
  fAutostop = 0;
}

//----------------------- Программирование режимов: --------------------------

inline void TControl::ProgMode(uint8_t &code)
{
  if(!Option(OPT_ENABLEPROG)) return; //выход, если программирование запрещено

  if(Mode == TR_STOP)        //текущий режим - STOP
  {
    if((code == (KEY_PROG)) && //удерживается кнопка STOP
       (Program == PR_OFF))  //и если еще не режим программирования
    {
      Program = PR_STEP1;    //вход в программирование
      ProgTimer->Start();    //запуск таймера
      ProgMode1 = TR_STOP;   //очистка шага 1
      ProgMode2 = TR_STOP;   //очистка шага 2
      fUpdate = 1;           //требование обновления индикации
      return;
    }
    if((code == KEY_STOP) && //нажата кнопка STOP
       (Program != PR_OFF))  //в режиме программирования
    {
      Program = PR_OFF;      //выход из режима программирования
      ProgTimer->Stop();     //остановка таймера
      ProgMode1 = TR_STOP;   //очистка шага 1
      ProgMode2 = TR_STOP;   //очистка шага 2
      fUpdate = 1;           //требование обновления индикации
      return;
    }
    switch(Program)
    {
    //программирование шага 1:
    case PR_STEP1:
      if(code == KEY_PLAYR)      ProgMode1 = TR_PLAYR;
      else if(code == KEY_REW)   ProgMode1 = Arch(TR_REW);
      else if(code == KEY_FFD)   ProgMode1 = Arch(TR_FFD);
      else if(code == KEY_PLAYF) ProgMode1 = TR_PLAYF;
      //запрещенные кнопки:
      else if(code == KEY_PAUSE || code == KEY_REC || code == KEY_ROLL)
      {
        Sound->Bell();      //сигнал ошибки
        code = KEY_STOP;    //сброс кода кнопки
      }
      if(ProgMode1 != TR_STOP) //если шаг 1 запрограммирован
      {
        code = KEY_STOP;    //сброс кода кнопки
        Program = PR_STEP2; //переход к второму шагу
        fUpdate = 1;        //требование обновления индикации
      }
      ProgTimer->Start();   //перезапуск таймера
      break;
    //программирование шага 2:
    case PR_STEP2:
      if(ProgMode1 == TR_AREW && code == KEY_REW)
      {
        ProgMode1 = TR_REW; //замена AREW на REW для шага 1
        ProgTimer->Start(); //перезапуск таймера
        code = KEY_STOP;    //сброс кода кнопки
        break;
      }
      if(ProgMode1 == TR_AFFD && code == KEY_FFD)
      {
        ProgMode1 = TR_FFD; //замена AFFD на FFD для шага 1
        ProgTimer->Start(); //перезапуск таймера
        code = KEY_STOP;    //сброс кода кнопки
        break;
      }
      //если на шаге 1 движение назад
      if(ProgMode1 == TR_AREW || ProgMode1 == TR_REW || ProgMode1 == TR_PLAYR)
      {
        if(code == KEY_PLAYF)      ProgMode2 = TR_PLAYF;
        else if(code == KEY_FFD)   ProgMode2 = Arch(TR_FFD);
        else if(code == KEY_PAUSE) ProgMode2 = TR_PAUSE;
        //запрещенные кнопки:
        else if(code == KEY_PLAYR || code == KEY_REW ||
                code == KEY_REC   || code == KEY_ROLL)
        {
          Sound->Bell();    //сигнал ошибки
          code = KEY_STOP;  //сброс кода кнопки
        }
      }
      //если на шаге 1 движение вперед
      if(ProgMode1 == TR_AFFD || ProgMode1 == TR_FFD || ProgMode1 == TR_PLAYF)
      {
        if(code == KEY_PLAYR)      ProgMode2 = TR_PLAYR;
        else if(code == KEY_REW)   ProgMode2 = Arch(TR_REW);
        else if(code == KEY_PAUSE) ProgMode2 = TR_PAUSE;
        //запрещенные кнопки:
        else if(code == KEY_PLAYF || code == KEY_FFD ||
                code == KEY_REC   || code == KEY_ROLL)
        {
          Sound->Bell();    //сигнал ошибки
          code = KEY_STOP;  //сброс кода кнопки
        }
      }
      if(ProgMode2 != TR_STOP) //если шаг 2 запрограммирован
      {
        code = KEY_STOP;    //сброс кода кнопки
        Program = PR_RUN;   //переход к выполнению программы
        fUpdate = 1;        //требование обновления индикации
      }
      ProgTimer->Start();   //перезапуск таймера
      break;
    //завершение программирования:
    case PR_RUN:
      if(ProgMode2 == TR_AREW && code == KEY_REW)
      {
        ProgMode2 = TR_REW; //замена AREW на REW для шага 2
        ProgTimer->Start(); //перезапуск таймера
        code = KEY_STOP;    //сброс кода кнопки
        break;
      }
      if(ProgMode2 == TR_AFFD && code == KEY_FFD)
      {
        ProgMode2 = TR_FFD; //замена AFFD на FFD для шага 2
        ProgTimer->Start(); //перезапуск таймера
        code = KEY_STOP;    //сброс кода кнопки
        break;
      }
      //запрещенные кнопки:
      if(code == KEY_PLAYR || code == KEY_PLAYF || code == KEY_REW ||
         code == KEY_FFD   || code == KEY_REC   || code == KEY_ROLL)
      {
        Sound->Bell();      //сигнал ошибки
        code = KEY_STOP;    //сброс кода кнопки
      }
      break;
    }
  }
  else //текущий режим - не STOP, сброс программы
  {
    //все кнопки управления режимами ЛПМ, кроме PAUSE
    //другие кнопки не сбрасывают программу
    if(code == KEY_PLAYR || code == KEY_PLAYF || code == KEY_REW ||
       code == KEY_FFD   || code == KEY_REC   || code == KEY_STOP ||
       code == KEY_REV)
    {
      Program = PR_OFF;
      ProgMode1 = TR_STOP;
      ProgMode2 = TR_STOP;
      fUpdate = 1;
    }
  }
}

//----------------------------------------------------------------------------
//------------------------- Реализация сервисов: -----------------------------
//----------------------------------------------------------------------------

//---------- Сервис медленного таймера процессов и Watchdog-таймера: ---------

inline bool TControl::ServiceTimer(void)
{
  if(TSysTimer::Tick)
  {
    if(++ServiceCounter == T_SLOW)
    {
      ServiceCounter = 0;
      //чтение состояния ЛПМ:
      TrState = Transport->GetState();
      __watchdog_reset();
      return(1);
    }
  }
  return(0);
}

//-------------------- Сервис управления светодиодами: -----------------------

inline void TControl::LedsService(void)
{
  Leds->Execute();
  if(fUpdate || (IndState != TrState))
  {
    IndState = TrState;
    fUpdate = 0;
    //очистка набора светодиодов:
    Leds->Set(LED_ALL, LEDS_OFF);
    //индикация набора программы:
    if(Program == PR_STEP1)
    {
      //начальный набор для программирования:
      Leds->Set(LED_REW + LED_PLAYR + LED_PLAYF + LED_FFD, LEDS_CONT);
      return;
    }
    else if(Program == PR_STEP2 || Program == PR_RUN)
    {
      //набор для движения вперед:
      if(ProgMode1 == TR_AREW || ProgMode1 == TR_REW || ProgMode1 == TR_PLAYR)
        Leds->Set(LED_PLAYF + LED_FFD + LED_PAUSE, LEDS_CONT);
      //набор для движения назад:
      if(ProgMode1 == TR_AFFD || ProgMode1 == TR_FFD || ProgMode1 == TR_PLAYF)
        Leds->Set(LED_PLAYR + LED_REW + LED_PAUSE, LEDS_CONT);
      return;
    }
    //индикация направления Capstan:
    if(Trs(TRS_CAP))                    //если Capstan включен
    {
      uint8_t b;
      if(!Trs(TRS_LOCK))                //если Capstan не готов
        b = LEDS_FAST;                  //быстрое мигание
          else if(Option(OPT_SHOWDIR))  //иначе если разреш. инд. направления
            b = LEDS_SLOW;              //то медленное мигание,
              else b = LEDS_OFF;        //иначе выключение
      if(Trs(TRS_REV))                  //если Capstan вращается назад,
        Leds->Set(LED_PLAYR, b);        //мигает LED_PLAYR,
          else Leds->Set(LED_PLAYF, b); //иначе мигает LED_PLAYF
    }
    //индикация режимов работы:
    switch(Mode)
    {
    case TR_STOP:
      if(Trs(TRS_TAPE))                 //если лента загружена, то
        Leds->Set(LED_STOP, LEDS_CONT); //LED_STOP горит
          else Leds->Set(LED_NONE);     //если ленты нет, то не горит
      break;

    case TR_PLAYF:
      if(fPause && Option(OPT_PAUSELEDBLINK)) //если пауза, то
        Leds->Set(LED_PLAYF, LEDS_NORM);     //LED_PLAYF мигает,
          else if(Trs(TRS_LOCK))             //иначе если Capstan готов,
            Leds->Set(LED_PLAYF, LEDS_CONT); //LED_PLAYF горит
      break;

    case TR_PLAYR:
      if(fPause && Option(OPT_PAUSELEDBLINK)) //если пауза, то
        Leds->Set(LED_PLAYR, LEDS_NORM);     //LED_PLAYR мигает,
          else if(Trs(TRS_LOCK))             //иначе если Capstan готов,
            Leds->Set(LED_PLAYR, LEDS_CONT); //LED_PLAYR горит
      break;

    case TR_REC:
      if(fPause && Option(OPT_PAUSELEDBLINK)) //если пауза, то LED_REC + PLAYF
        Leds->Set(LED_REC + LED_PLAYF, LEDS_NORM); //мигает,
          else Leds->Set(LED_REC + LED_PLAYF, LEDS_CONT); //иначе горит
      break;

    case TR_FFD:
      if(fRoll)
        Leds->Set(LED_PLAYR, LEDS_CONT); //если откат, горит LED_PLAYR
      Leds->Set(LED_FFD, LEDS_CONT);     //включение LED_FFD
      break;

    case TR_REW:
      if(fRoll)
        Leds->Set(LED_PLAYF, LEDS_CONT); //если откат, горит LED_PLAYF
      Leds->Set(LED_REW, LEDS_CONT);     //включение LED_REW
      break;

    case TR_AFFD:
      if(fRoll)
        Leds->Set(LED_PLAYR, LEDS_CONT); //если откат, горит LED_PLAYR
      if(Trs(TRS_CUE))                   //если обзор
        if(Trs(TRS_REV))                 //и если Capstan вращается назад,
          Leds->Set(LED_PLAYR, LEDS_FAST); //то быстро мигает LED_PLAYR,
            else Leds->Set(LED_PLAYF, LEDS_FAST); //иначе LED_PLAYF
      Leds->Set(LED_FFD, LEDS_NORM);     //LED_FFD мигает норм.
      break;

    case TR_AREW:
      if(fRoll)
        Leds->Set(LED_PLAYF, LEDS_CONT); //если откат, горит LED_PLAYF
      if(Trs(TRS_CUE))                   //если обзор
        if(Trs(TRS_REV))                 //и если Capstan вращается назад,
          Leds->Set(LED_PLAYR, LEDS_FAST); //то быстро мигает LED_PLAYR,
            else Leds->Set(LED_PLAYF, LEDS_FAST); //иначе LED_PLAYF
      Leds->Set(LED_REW, LEDS_NORM);     //LED_REW мигает норм.
      break;
    };

    //индикация торможения двигателями:
    if(Trs(TRS_BRAKE) && Trs(TRS_MOVE))  //если торможение, то
      Leds->Set(LED_STOP, LEDS_FAST);    //LED_STOP мигает

    //индикация режима PAUSE:
    if(fPause)                           //если пауза, то
    {
      if(Option(OPT_PAUSELEDBLINK))
        Leds->Set(LED_PAUSE, LEDS_NORM); //LED_PAUSE мигает
          else Leds->Set(LED_PAUSE, LEDS_CONT); //или горит
    }
    //индикация второго программного режима:
    if(ProgMode2 != TR_STOP)
    {
      uint8_t m = Option(OPT_PROGBLINK)? LEDS_SLOW : LEDS_CONT;
      switch(ProgMode2)
      {
      case TR_PAUSE: Leds->Set(LED_PAUSE, m); break;
      case TR_PLAYR: Leds->Set(LED_PLAYR, m); break;
      case TR_REW:
      case TR_AREW:  Leds->Set(LED_REW,   m); break;
      case TR_FFD:
      case TR_AFFD:  Leds->Set(LED_FFD,   m); break;
      case TR_PLAYF: Leds->Set(LED_PLAYF, m); break;
      }
    }
  }
}

//--------------------------- Сервис автостопа: -----------------------------

inline void TControl::AutoStopService(void)
{
  uint8_t as = Transport->CheckAutoStop();
  if(as)
  {
    if(!fAutostop && (as == ASR_END)) //если был автостоп по ДО
    {
      //сохранение режима для автореверса:
      if(Mode == TR_PLAYF) ArMode = TR_PLAYR;
        else if(Mode == TR_PLAYR) ArMode = TR_PLAYF;
          else ArMode = TR_STOP;
      fAutostop = 1; //установка флага автостопа
    }
    Mode = TR_STOP;
    Transport->SetMode(TR_ASTOP); //механическое торможение
    fUpdate = 1;
  }
}

//---------------- Сервис программного режима и автореверса: -----------------

inline void TControl::AutoPlayService(void)
{
  //проверка таймера программы:
  if(ProgTimer->Over())         //таймер переполнился,
  {
    if(Program == PR_RUN)       //если введены 2 шага программы, то
    {
      Mode = ProgMode1;
      ProgMode1 = TR_STOP;
      Transport->SetMode(Mode); //выполнение шага 1
    }
    else                        //иначе
    {
      Sound->Tick();
      ProgMode1 = TR_STOP;      //сброс программы
      ProgMode2 = TR_STOP;
    }
    Program = PR_OFF;
    fUpdate = 1;
  }
  //проверка срабатывания автостопа:
  if(fAutostop &&               //если сработал автостоп,
     (Transport->GetMode() == TR_STOP)) //остановка завершена
  {
    if(!Trs(TRS_MOVE + TRS_LOWTEN)) //если лента не закончилась и неподвижна,
    {
      //программный режим (у него приоритет):
      if(ProgMode2 != TR_STOP)
      {
        Mode = ProgMode2;
        ProgMode2 = TR_STOP;
        Transport->SetMode(Mode);
        fUpdate = 1;
      }
      //автореверс:
      else
      {
        if(Option(OPT_AUTOREVERSE) && //если включена опция автореверса
           (ArMode != TR_STOP))       //и есть режим для автореверса
        {
          Mode = ArMode;
          Transport->SetMode(Mode);   //включение реверса
          Transport->ArDelay();       //дополнительная задержка ДО
          fUpdate = 1;
        }
      }
    }
    fAutostop = 0;              //сброс флага автостопа
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//--------------------------- Установка опций: -------------------------------

void TControl::SetOptions(uint16_t t)
{
  Options = t;
  Sound->OnOff(!Option(OPT_NOSOUND));
  fUpdate = 1;
}

//----------------------------- Чтение опций: ----------------------------------

uint16_t TControl::GetOptions(void)
{
  return(Options);
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TControl::EERead(void)
{
  Options = Eeprom->Rd16(EE_CT_OPTIONS, NOM_CT_OPTIONS);
  Sound->OnOff(!Option(OPT_NOSOUND)); //включение/выключение звука
  Transport->EERead();
}

//------------------------ Save Params to EEPROM: ----------------------------

void TControl::EESave(void)
{
  Eeprom->Wr16(EE_CT_OPTIONS, Options);
  Transport->EESave();
  Eeprom->Validate();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
