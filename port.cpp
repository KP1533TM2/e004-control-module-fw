//----------------------------------------------------------------------------

//Модуль поддержки порта

//----------------------------------------------------------------------------

//Модуль реализует набор команд, который используется для управления
//устройством с компьютера, а также для задания и считывания параметров.
//Протокол обмена - Wake, реализован в классе TWake, от которого наследован
//класс TWakePort, привязывающий протокол к аппаратному порту USART0.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "control.hpp"
#include "port.hpp"
#include "commands.hpp"

//----------------------------------------------------------------------------
//------------------------------ Класс TPort ---------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TPort::TPort()
{
}

//----------------------------- Инициализация: -------------------------------
void TPort::Start(void)
{
  WakePort.Start();
}

//-------------------------- Выполнение команд: ------------------------------

void TPort::Execute(void)
{
  char Command = WakePort.GetCmd(); //чтение кода принятой команды
  if(Command != CMD_NOP)             //если есть команда, выполнение
  {
    switch(Command)
    {

//----------------------------------------------------------------------------
//----------------------- Стандартные команды: -------------------------------
//----------------------------------------------------------------------------

    case CMD_ERR: //обработка ошибки
      {
        WakePort.AddByte(ERR_TX);
        break;
      }
    case CMD_ECHO: //эхо
      {
        char cnt = WakePort.GetRxCount();
        for(char i = 0; i < cnt; i++)
          WakePort.AddByte(WakePort.GetByte());
        break;
      }
    case CMD_INFO: //чтение иформации об устройстве
      {
        uint8_t i = 0;
        uint8_t c = 0;
        while(c = pgm_read_byte(&DevName[i++]))
          WakePort.AddByte(c);
        break;
      }

//----------------------------------------------------------------------------
//----------------------- Специальные команды: -------------------------------
//----------------------------------------------------------------------------

    case CMD_SET_MODE: //управление режимами
      {
        Control.SetMode(WakePort.GetByte());
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_SPOOL_MODE: //управление режимами подмотки
      {
        Control.Transport.Spool.SetMode(WakePort.GetByte());
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_CAPSTAN: //управление ведущим двигателем
      {
        uint8_t m = WakePort.GetByte();
        Control.Transport.Capstan.Start(m & CAP_REVERSE);
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_TC: //переключение постоянной времени датчика движения
      {
        uint8_t m = WakePort.GetByte();
        Control.Transport.MoveSensor.SetTC(m);
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_SOLENOIDS: //управление электромагнитами
      {
        uint8_t s = WakePort.GetByte();
        Control.Transport.SolBrake.OnOff(s & SOL_BRAKE);
        Control.Transport.SolPress.OnOff(s & SOL_PRESS);
        Control.Transport.SolLift.OnOff(s & SOL_LIFT);
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_TENSION: //установка натяжения
      {
        uint8_t m = WakePort.GetByte();
        ten_t t;
        t.m1 = WakePort.GetWord();
        t.m2 = WakePort.GetWord();
        Control.Transport.Spool.SetTen(m, t);
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_GET_TENSION: //чтение натяжения
      {
        uint8_t m = WakePort.GetByte();
        ten_t t = Control.Transport.Spool.GetTen(m);
        WakePort.AddByte(ERR_NO);
        WakePort.AddWord(t.m1);
        WakePort.AddWord(t.m2);
        break;
      }
    case CMD_SET_SPOOL: //управление боковыми двигателями
      {
        Control.Transport.Spool.SetMot1(WakePort.GetWord());
        Control.Transport.Spool.SetMot2(WakePort.GetWord());
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_SET_AUDIO: //управление аудиотрактом
      {
        uint8_t a = WakePort.GetByte();
        Control.Transport.Audio.Mute(a & AUD_MUTE);
        Control.Transport.Audio.Rev(a & AUD_REV);
        Control.Transport.Audio.Rec(a & AUD_REC);
        Control.Transport.Audio.MasterMute(OFF);
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_GET_STATE: //чтение состояния
      {
        WakePort.AddByte(ERR_NO);
        leds_t leds;
        Control.Leds.Get(leds);
        WakePort.AddByte(leds.cont);
        WakePort.AddByte(leds.slow);
        WakePort.AddByte(leds.norm);
        WakePort.AddByte(leds.fast);
        uint8_t m = 0; ten_t t;
        if(Control.Transport.SolBrake.Hold())   m |= STB_BRAKE;
        if(Control.Transport.SolPress.Hold())   m |= STB_PRESS;
        if(Control.Transport.SolLift.Hold())    m |= STB_LIFT;
        if(Control.Transport.MoveSensor.Move()) m |= STB_MOVE;
        if(Control.Transport.EndSensor.Tape())  m |= STB_TAPE;
        if(Control.Transport.Capstan.Running()) m |= STB_RUN;
        if(Control.Transport.Capstan.Reverse()) m |= STB_REV;
        if(Control.Transport.Capstan.Ready())   m |= STB_READY;
        WakePort.AddByte(m);
        WakePort.AddByte(Control.Transport.Audio.GetState());
        WakePort.AddByte(Control.Transport.Spool.GetMode());
        t = Control.Transport.Spool.GetSen();
        WakePort.AddWord(t.m1);
        WakePort.AddWord(t.m2);
        t = Control.Transport.Spool.GetMot();
        WakePort.AddWord(t.m1);
        WakePort.AddWord(t.m2);
        t = Control.Transport.Spool.GetTen();
        WakePort.AddWord(t.m1);
        WakePort.AddWord(t.m2);
        break;
      }
    case CMD_SET_PARAMS: //установка параметров
      {
        kpid_t K;
        K.p = WakePort.GetByte();
        K.i = WakePort.GetByte();
        K.d = WakePort.GetByte();
        Control.Transport.Spool.SetPID1(K);
        K.p = WakePort.GetByte();
        K.i = WakePort.GetByte();
        K.d = WakePort.GetByte();
        Control.Transport.Spool.SetPID2(K);
        Control.Transport.Capstan.SetTstart(WakePort.GetWord());
        Control.Transport.EndSensor.SetTau(WakePort.GetWord());
        Control.Transport.SetEmForce(WakePort.GetWord());
        Control.Transport.SetAsBrkDel(WakePort.GetWord());
        Control.Transport.SetAsPreDel(WakePort.GetWord());
        Control.Transport.SetAsTenDel(WakePort.GetWord());
        Control.Transport.SetOptions(WakePort.GetByte());
        Control.SetOptions(WakePort.GetWord());
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_GET_PARAMS: //чтение параметров
      {
        WakePort.AddByte(ERR_NO);
        kpid_t K;
        K = Control.Transport.Spool.GetPID1();
        WakePort.AddByte(K.p);
        WakePort.AddByte(K.i);
        WakePort.AddByte(K.d);
        K = Control.Transport.Spool.GetPID2();
        WakePort.AddByte(K.p);
        WakePort.AddByte(K.i);
        WakePort.AddByte(K.d);
        WakePort.AddWord(Control.Transport.Capstan.GetTstart());
        WakePort.AddWord(Control.Transport.EndSensor.GetTau());
        WakePort.AddWord(Control.Transport.GetEmForce());
        WakePort.AddWord(Control.Transport.GetAsBrkDel());
        WakePort.AddWord(Control.Transport.GetAsPreDel());
        WakePort.AddWord(Control.Transport.GetAsTenDel());
        WakePort.AddByte(Control.Transport.GetOptions());
        WakePort.AddWord(Control.GetOptions());
        break;
      }
    case CMD_EE_SAVE: //сохранение параметров в EEPROM
      {
        Control.EESave();
        WakePort.AddByte(ERR_NO);
        break;
      }
    case CMD_DEFAULT: //загрузка параметров по умолчанию
      {
        Eeprom.Invalidate();
        Control.EERead();
        WakePort.AddByte(ERR_NO);
        break;
      }

//----------------------------------------------------------------------------
//----------------------- Неизвестная команда: -------------------------------
//----------------------------------------------------------------------------

    default:
      {
        WakePort.AddByte(ERR_PA);
      }
    }
    WakePort.StartTx(Command);
  }
}

//----------------------------------------------------------------------------
