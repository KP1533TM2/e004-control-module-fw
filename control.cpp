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
  KeyMsg = MSG_NO;
  KeyCode = KEY_NO;
  Sound = new TSound();
  Transport = new TTransport();
  Leds = new TLeds();
  SlowCounter = 0;
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

  //считывание сообщения кнопок:
  if(Local->NewMessage())
  {
    KeyMsg = Local->Message;
    KeyCode = Local->Code;
  }
  //считывание сообщения ИК ДУ:
  else if(Remote->NewMessage())
  {
    KeyMsg = Remote->Message;
    KeyCode = Remote->Code;
  }
  //перекодировка отпускания кнопки RollBack:
  if((KeyCode == KEY_ROLL) && (KeyMsg == MSG_REL))
  {
    KeyMsg = MSG_PRESS;
    KeyCode = KEY_UNROLL;
  }
  //обработка нажатий кнопок:
  if(KeyMsg == MSG_PRESS)
  {
    SetMode(KeyCode);
  }

  //сервисы:
  SlowTimerService(); //сервис медленного таймера
  LedsService();      //сервис светодиодов
  AutoStopService();  //сервис автостопа

  //очистка сообщения кнопки:
  KeyMsg = MSG_NO;
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
  Pin_XT6.DirOut();
  Pin_XT5.DirOut();
  Pin_XT4.DirOut();
  Pin_XT3.DirOut();
  Pin_XT2.DirOut();
  Pin_XT1.DirOut();
  Pin_PB1.DirIn(PIN_PULL);
  //настройка watchdog-таймера (260 ms):
  __watchdog_reset();
  WDTCR = (1 << WDCE) | (1 << WDE);
  WDTCR = (1 << WDE) | (1 << WDP2);
}

//-------------------------- Переключение режимов: ---------------------------

void TControl::SetMode(uint8_t code)
{
  Sound->Tick();
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
    if(Option(OPT_PLAYOFFPAUSE))         //если есть опция выключения паузы,
    {
      fPause = 0;                        //выключение паузы
      if(Mode != TR_REC)                 //если не запись, то
        Mode = TR_PLAYF;                 //включение PLAYF
    }
    else Mode = TR_PLAYF;                //включение PLAYF
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
    else
    {
      Transport->SetMode(TR_CAPF);       //направление вращения тонвала
      Transport->SetCue(ON);             //выключение MUTE
    }
    break;

  //включение режима PLAYR
  case KEY_PLAYR:
    Mode = TR_PLAYR;
    if(Option(OPT_PLAYOFFPAUSE))
      fPause = 0;                        //выключение паузы
    Transport->Audio->Rec(OFF);          //выключение тракта записи
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
    else
    {
      Transport->SetMode(TR_CAPR);       //направление вращения тонвала
      Transport->SetCue(ON);             //выключение MUTE
    }
    break;

  //включение режима REC
  case KEY_REC:
    Mode = TR_REC;
    if(Option(OPT_AUTORECPAUSE))
      fPause = 1;                        //включение паузы
    Transport->Audio->Rec(ON);           //включение тракта записи
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
      else Transport->SetMode(TR_CAPF);  //направление вращения тонвала
    break;

  //включение режима PAUSE
  case KEY_PAUSE:
    fPause = !fPause;                    //включение/выключение режима паузы
    if(fPause)                           //если пауза включается, то в режиме
    {
      if((Mode == TR_PLAYF) ||           //PLAYF или
         (Mode == TR_PLAYR) ||           //PLAYR или
         (Mode == TR_REC))               //REC -
      {
        Transport->SetMode(TR_PAUSE);    //остановка ленты без MUTE
        Transport->SetCue(ON);           //выключение MUTE
      }
    }
    else                                 //если пауза выключается,
      Transport->SetMode(Mode);          //возобновление режима
    break;

  //включение режима AFFD или FFD
  case KEY_FFD:
    if(Mode == TR_AFFD ||                //если режим арх. перем. или
       !Option(OPT_USEARCHIVE))          //арх. перем. не используется, то
      Mode = TR_FFD;                     //включение обычной перемотки
        else Mode = TR_AFFD;             //иначе включение архивной
    Transport->Audio->Rec(OFF);          //выключение тракта записи
    Transport->SetMode(Mode);            //включение режима
    break;

  //включение режима AREW или REW
  case KEY_REW:
    if(Mode == TR_AREW ||                //если режим арх. перем. или
       !Option(OPT_USEARCHIVE))          //арх. перем. не используется, то
      Mode = TR_REW;                     //включение обычной перемотки
        else Mode = TR_AREW;             //иначе включение архивной
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
      if(Option(OPT_USEARCHIVE))         //если опция арх. перем., то
        Mode = TR_AREW;                  //включение архивной перемотки
          else Mode = TR_REW;            //иначе включение обычной
      fRoll = 1;                         //флаг отката
    }
    else if(Mode == TR_PLAYR)            //если PLAYR,
    {
      BackMode = Mode;                   //запоминание режима возврата
      if(Option(OPT_USEARCHIVE))         //если опция арх. перем., то
        Mode = TR_AFFD;                  //включение архивной перемотки
          else Mode = TR_FFD;            //иначе включение обычной
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

  //включение режима PLAYF или PLAYR
  case KEY_PLAY:
    if(Transport->Capstan->Reverse())    //если вал вращается назад, то
    {
      Mode = TR_PLAYR;                   //включение режима PLAYR,
      Transport->Audio->Rec(OFF);        //выключение тракта записи
      if(Option(OPT_PLAYOFFPAUSE))       //если есть такая опция, то
        fPause = 0;                      //выключение паузы
    }
    else
    {
      if(Option(OPT_PLAYOFFPAUSE))       //если есть опция выключения паузы,
      {
        fPause = 0;                      //выключение паузы
        if(Mode != TR_REC)               //если не режим записи, то
          Mode = TR_PLAYF;               //переход в PLAYF
      }
      else Mode = TR_PLAYF;              //переход в PLAYF
    }
    if(!fPause) Transport->SetMode(Mode); //если не пауза, включение режима
      else Transport->SetCue(ON);        //выключение MUTE
    break;

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
    else if(Transport->Capstan->Reverse()) //иначе реверс тонвала
      Transport->SetMode(TR_CAPF);
        else Transport->SetMode(TR_CAPR);
    break;

  //включение/выключение режима Mute
  case KEY_MUTE:
    Transport->Audio->MuteToggle();      //включение/выключение MUTE
    break;
  };
  fUpdate = 1;
}

//----------------------------------------------------------------------------
//------------------------- Реализация сервисов: -----------------------------
//----------------------------------------------------------------------------

//---------- Сервис медленного таймера процессов и Watchdog-таймера: ---------

inline void TControl::SlowTimerService(void)
{
  fSlowTick = 0;
  if(TSysTimer::Tick)
  {
    if(++SlowCounter == T_SLOW)
    {
      SlowCounter = 0;
      fSlowTick = 1;
      __watchdog_reset();
    }
  }
}

//-------------------- Сервис управления светодиодами: -----------------------

inline void TControl::LedsService(void)
{
  if(fSlowTick)
  {
    Leds->Execute();
    uint8_t state = Transport->GetState();
    if(fUpdate || (IndState != state))
    {
      IndState = state;
      fUpdate = 0;
      //очистка набора светодиодов:
      Leds->Set(LED_ALL, LEDS_OFF);
      //индикация направления Capstan:
      if(Ins(TRS_CAP))                    //если Capstan включен
      {
        uint8_t b;
        if(Ins(TRS_LOCK))                 //если Capstan готов
          b = LEDS_SLOW;                  //медленное мигание,
            else b = LEDS_FAST;           //иначе - быстрое
        if(Ins(TRS_REV))                  //если Capstan вращается назад,
          Leds->Set(LED_PLAYR, b);        //мигает LED_PLAYR,
            else Leds->Set(LED_PLAYF, b); //иначе мигает LED_PLAYF
      }
      //индикация режимов работы:
      switch(Mode)
      {
      case TR_STOP:
        if(Ins(TRS_TAPE))                 //если лента загружена, то
          Leds->Set(LED_STOP, LEDS_CONT); //LED_STOP горит
            else Leds->Set(LED_NONE);     //если ленты нет, то не горит
        break;

      case TR_PLAYF:
        if(fPause && Option(OPT_PAUSELEDBLINK)) //если пауза, то
          Leds->Set(LED_PLAYF, LEDS_NORM);     //LED_PLAYF мигает,
            else if(Ins(TRS_LOCK))             //иначе если Capstan готов,
              Leds->Set(LED_PLAYF, LEDS_CONT); //LED_PLAYF горит
        break;

      case TR_PLAYR:
        if(fPause && Option(OPT_PAUSELEDBLINK)) //если пауза, то
          Leds->Set(LED_PLAYR, LEDS_NORM);     //LED_PLAYR мигает,
            else if(Ins(TRS_LOCK))             //иначе если Capstan готов,
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
        if(Ins(TRS_CUE))                   //если обзор
          if(Ins(TRS_REV))                 //и если Capstan вращается назад,
            Leds->Set(LED_PLAYR, LEDS_FAST); //то быстро мигает LED_PLAYR,
              else Leds->Set(LED_PLAYF, LEDS_FAST); //иначе LED_PLAYF
        Leds->Set(LED_FFD, LEDS_NORM);     //LED_FFD мигает норм.
        break;

      case TR_AREW:
        if(fRoll)
          Leds->Set(LED_PLAYF, LEDS_CONT); //если откат, горит LED_PLAYF
        if(Ins(TRS_CUE))                   //если обзор
          if(Ins(TRS_REV))                 //и если Capstan вращается назад,
            Leds->Set(LED_PLAYR, LEDS_FAST); //то быстро мигает LED_PLAYR,
              else Leds->Set(LED_PLAYF, LEDS_FAST); //иначе LED_PLAYF
        Leds->Set(LED_REW, LEDS_NORM);     //LED_REW мигает норм.
        break;
      };

      //индикация торможения двигателями:
      if(Ins(TRS_BRAKE) && Ins(TRS_MOVE))  //если торможение, то
        Leds->Set(LED_STOP, LEDS_FAST);    //LED_STOP мигает

      //индикация режима PAUSE:
      if(fPause)                           //если пауза, то
      {
        if(Option(OPT_PAUSELEDBLINK))
          Leds->Set(LED_PAUSE, LEDS_NORM); //LED_PAUSE мигает
            else Leds->Set(LED_PAUSE, LEDS_CONT); //или горит
      }
    }
  }
}

//--------------------------- Сервис автостопа: -----------------------------

inline void TControl::AutoStopService(void)
{
  if(fSlowTick)
  {
    if(Transport->CheckAutoStop())
    {
      Mode = TR_STOP;
      fUpdate = 1;
      Transport->SetMode(TR_ASTOP); //механическое торможение
    }
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//--------------------------- Установка опций: -------------------------------

void TControl::SetOptions(uint8_t t)
{
  Options = t;
  Sound->OnOff(!Option(OPT_NOSOUND));
  fUpdate = 1;
}

//---------------------------- Чтение опций: ---------------------------------

uint8_t TControl::GetOptions(void)
{
  return(Options);
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TControl::EERead(void)
{
  Options = Eeprom->Rd8(EE_CT_OPTIONS, NOM_CT_OPTIONS);
  Sound->OnOff(!Option(OPT_NOSOUND)); //включение/выключение звука
  Transport->EERead();
}

//------------------------ Save Params to EEPROM: ----------------------------

void TControl::EESave(void)
{
  Eeprom->Wr8(EE_CT_OPTIONS, Options);
  Transport->EESave();
  Eeprom->Validate();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
