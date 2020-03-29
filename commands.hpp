//----------------------------------------------------------------------------

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

//----------------------------------------------------------------------------
//------------------------ Коды специальных команд: --------------------------
//----------------------------------------------------------------------------

enum WakeScmd_t
{
  CMD_DUMMY = CMD_GETADDR,

//------------------------- Управление режимами: -----------------------------

  CMD_SET_MODE,
  //TX: byte M RX:
  //M = KEY_REC, KEY_STOP, KEY_PLAYF, KEY_FFD, KEY_REW, KEY_PLAYR, KEY_PAUSE,
  //KEY_ROLL, KEY_UNROLL

//--------------------- Управление режимами подмотки: ------------------------

  CMD_SET_SPOOL_MODE,
  //TX: byte S RX:
  //S = SPOOL_OFF, SPOOL_BRAKE, SPOOL_FORCEF, SPOOL_FORCER,
  //    SPOOL_PLAYF, SPOOL_PLAYR, SPOOL_FFD, SPOOL_REW,
  //    SPOOL_AFFD, SPOOL_AREW

//-------------------- Управление ведущим двигателем: ------------------------

  CMD_SET_CAPSTAN,
  //TX: byte M RX:
  //M.0 (CAP_ON) = OFF - Capstan off, = ON - Capstan on
  //M.1 (CAP_DIR) = OFF - Capstan forward direction, = ON - Capstan reverse direction

//------------ Переключение постоянной времени датчика движения: -------------

  CMD_SET_TC,
  //TX: byte M RX:
  //M = MOVE_FAST, MOVE_SLOW

//--------------------- Управление электромагнитами: -------------------------

  CMD_SET_SOLENOIDS,
  //TX: byte S RX:
  //S.0 (SOL_BRAKE) = OFF - Brake off, = ON - Brake on
  //S.1 (SOL_PRESS) = OFF - Press off, = ON - Press on
  //S.2 (SOL_LIFT)  = OFF - Lift  off, = ON - Lift  on

//-------------------------- Установка натяжения: ----------------------------

  CMD_SET_TENSION,
  //TX: byte Mode, word T1, word T2 RX:
  //Mode = SPOOL_OFF (MIN_TENSION), SPOOL_BRAKE, SPOOL_PLAYF, SPOOL_PLAYR,
  //       SPOOL_FFD, SPOOL_REW, SPOOL_AFFD, SPOOL_AREW
  //T1 = 0..65535 [0..2560 mV tension sensor 1 (TP1)]
  //T2 = 0..65535 [0..2560 mV tension sensor 2 (TP2)]

//--------------------------- Чтение натяжения: ------------------------------

  CMD_GET_TENSION,
  //TX: byte Mode RX: word T1, word T2
  //Mode = SPOOL_OFF (MIN_TENSION), SPOOL_BRAKE, SPOOL_PLAYF, SPOOL_PLAYR,
  //       SPOOL_FFD, SPOOL_REW, SPOOL_AFFD, SPOOL_AREW
  //T1 = 0..65535 [0..2560 mV tension sensor 1 (TP1)]
  //T2 = 0..65535 [0..2560 mV tension sensor 2 (TP2)]

//-------------------- Управление боковыми двигателями: ----------------------

  CMD_SET_SPOOL,
  //TX: word M1, word M2 RX:
  //M1 = 0..65535 [0..2560 mV motor 1 control (TP3)]
  //M2 = 0..65535 [0..2560 mV motor 2 control (TP4)]

//------------------------ Управление аудиотрактом: --------------------------

  CMD_SET_AUDIO,
  //TX: byte A RX:
  //A.0 (AUD_MUTE) = OFF - PB amp. mute off, = ON - PB amp. mute on
  //A.1 (AUD_REV) = OFF - PB amp. fwd channel, = ON - PB amp. rev channel
  //A.2 (AUD_REC) = OFF - Record off, ON - Record on

//--------------------------- Чтение состояния: ------------------------------

  CMD_GET_STATE,
  //TX: RX: byte L1, byte L2, byte L3, byte L4, byte B, byte A, byte S,
  //        word T1, word T2, word M1, word M2, word R1, word R2
  //L1 (LEDS_CONT), L2 (LEDS_SLOW), L3 (LEDS_NORM), L4 (LEDS_FAST)
  //LX.0 - LED_REC
  //LX.1 - LED_STOP
  //LX.2 - LED_PLAYF
  //LX.3 - LED_FFD
  //LX.4 - LED_REW
  //LX.5 - LED_PLAYR
  //LX.6 - LED_PAUSE
  //B.0 = ON - Brake solenoid on
  //B.1 = ON - Press solenoid on
  //B.2 = ON - Lift solenoid on
  //B.3 = ON - Tape move
  //B.4 = ON - Tape loaded
  //B.5 = ON - Capstan on
  //B.6 = ON - Capstan reverse direction
  //B.7 = ON - Capstan ready
  //A.0 = ON - PB amplifier mute on
  //A.1 = ON - PB amplifier reverse channel on
  //A.2 = ON - Record on
  //S = SPOOL_OFF, SPOOL_BRAKE, SPOOL_FORCEF, SPOOL_FORCER,
  //    SPOOL_PLAYF, SPOOL_PLAYR, SPOOL_FFD, SPOOL_REW,
  //    SPOOL_AFFD, SPOOL_AREW
  //T1 = 0..65535 [0..2560 mV tension sensor 1 (TP1)]
  //T2 = 0..65535 [0..2560 mV tension sensor 2 (TP2)]
  //M1 = 0..65535 [0..2560 mV motor 1 control (TP3)]
  //M2 = 0..65535 [0..2560 mV motor 2 control (TP4)]
  //R1 = 0..65535 [0..2560 mV tension reference 1] - not used
  //R2 = 0..65535 [0..2560 mV tension reference 2] - not used

//------------------------- Установка параметров: ----------------------------

  CMD_SET_PARAMS,
  //TX: byte Kp, byte Ki, byte Kd,
  //    word CapStart, word EndTau, word EmForce,
  //    word AsBrkDel, word AsPreDel, word AsTenDel,
  //    byte TrOptions, byte CtOptions RX:
  //Kp = 0..255 - пропорциональный коэффициент PID
  //Ki = 0..255 - интегральный коэффициент PID
  //Kd = 0..255 - дифференциальный коэффициент PID
  //CapStart = 0..65535 [ms] время разгона ведущего двигателя
  //EndTau   = 0..65535 [ms] постоянная времени ДО
  //EmForce  = 0..65535 [ms] время старта электромагнитов
  //AsBrkDel = 0..65535 [ms] задержка автостопа при торможении
  //AsPreDel = 0..65535 [ms] задержка автостопа при пред. натяжении
  //AsTenDel = 0..65535 [ms] задержка автостопа по натяжению
  //TrOptions.0 = ON (OPT_BRKASENABLE) разрешение автостопа при торможении
  //TrOptions.1 = ON (OPT_PREASENABLE) разрешение автостопа по пред. натяжению
  //TrOptions.2 = ON (OPT_TENASENABLE) разрешение автостопа по натяжению
  //TrOptions.3 = ON (OPT_MOVASENABLE) разрешение автостопа по ДД
  //CtOptions.0 = ON (OPT_PAUSELEDBLINK) cветодиод паузы мигает, иначе - горит
  //CtOptions.1 = ON (OPT_AUTORECPAUSE) авт. вкл. паузы при вкл. записи
  //CtOptions.2 = ON (OPT_PLAYOFFPAUSE) кнопка Play выключает паузу
  //CtOptions.3 = ON (OPT_USEARCHIVE) использовать архивную перемотку
  //CtOptions.4 = ON (OPT_ROLLCUE) при откате включен обзор (и выключен Mute)
  //CtOptions.5 = ON (OPT_ENABLECUE) разреш. обзора кнопкой Roll при арх. перем.
  //CtOptions.6 = ON (OPT_NOSOUND) запрещение генерации звуковых сигналов
  //CtOptions.7 = ON (OPT_AUTOREVERSE) включение режима автореверса
  //CtOptions.8 = ON (OPT_ENABLEPROG) разрешение программирования режимов
  //CtOptions.9 = ON (OPT_PROGBLINK) прогрммный режим мигает, иначе горит

//-------------------------- Чтение параметров: ------------------------------

  CMD_GET_PARAMS,
  //TX: RX: byte Kp, byte Ki, byte Kd,
  //        word CapStart, word EndTau, word EmForce,
  //        word AsBrkDel, word AsPreDel, word AsTenDel,
  //        byte TrOptions, byte CtOptions
  //Kp = 0..255 - пропорциональный коэффициент PID
  //Ki = 0..255 - интегральный коэффициент PID
  //Kd = 0..255 - дифференциальный коэффициент PID
  //CapStart = 0..65535 [ms] время разгона ведущего двигателя
  //EndTau   = 0..65535 [ms] постоянная времени ДО
  //EmForce  = 0..65535 [ms] время старта электромагнитов
  //AsBrkDel = 0..65535 [ms] задержка автостопа при торможении
  //AsPreDel = 0..65535 [ms] задержка автостопа при пред. натяжении
  //AsTenDel = 0..65535 [ms] задержка автостопа по натяжению
  //TrOptions.0 = ON (OPT_BRKASENABLE) разрешение автостопа при торможении
  //TrOptions.1 = ON (OPT_PREASENABLE) разрешение автостопа по пред. натяжению
  //TrOptions.2 = ON (OPT_TENASENABLE) разрешение автостопа по натяжению
  //TrOptions.3 = ON (OPT_MOVASENABLE) разрешение автостопа по ДД
  //CtOptions.0 = ON (OPT_PAUSELEDBLINK) cветодиод паузы мигает, иначе - горит
  //CtOptions.1 = ON (OPT_AUTORECPAUSE) авт. вкл. паузы при вкл. записи
  //CtOptions.2 = ON (OPT_PLAYOFFPAUSE) кнопка Play выключает паузу
  //CtOptions.3 = ON (OPT_USEARCHIVE) использовать архивную перемотку
  //CtOptions.4 = ON (OPT_ROLLCUE) при откате включен обзор (и выключен Mute)
  //CtOptions.5 = ON (OPT_ENABLECUE) разреш. обзора кнопкой Roll при арх. перем.
  //CtOptions.6 = ON (OPT_NOSOUND) запрещение генерации звуковых сигналов
  //CtOptions.7 = ON (OPT_AUTOREVERSE) включение режима автореверса
  //CtOptions.8 = ON (OPT_ENABLEPROG) разрешение программирования режимов
  //CtOptions.9 = ON (OPT_PROGBLINK) прогрммный режим мигает, иначе горит

//-------------------- Сохранение параметров в EEPROM: -----------------------

  CMD_EE_SAVE,
  //TX: RX:

//------------------- Загрузка параметров по умолчанию: ----------------------

  CMD_DEFAULT,
  //TX: RX:

//----------------------------------------------------------------------------

  CMD_LAST_COMMAND
};

//----------------------------------------------------------------------------

#endif
