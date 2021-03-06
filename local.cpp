//----------------------------------------------------------------------------

//Модуль поддержки местной клавиатуры

//----------------------- Используемые ресурсы: ------------------------------

//Класс TLocal наследуется от абстрактного класса TKeyboard и определяет
//функцию GetCode(), которая сканирует местную клавиатуру.
//Есть подозрение, что у этого класса не очень хорошая наследственность.
//Кнопки подключены непосредственно к портам микроконтроллера. При нажатии
//любой кнопки на соответствующем выводе появляется НИЗКИЙ уровень.
//Код кнопки генерируется только при одной нажатой кнопке. Если нажаты
//две и более кнопок, генерируется код KEY_NO. Исключение составляет
//кнопка REC. Ее одиночное нажатие генерирует код KEY_NO, а совместное
//с кнопкой PLAYF нажатие генерирует код KEY_REC. Т.е. включить режим
//записи можно только одновременным нажатием кнпок REC + PLAYF.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "local.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TLocal: --------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TLocal::TLocal(void)
{
  Key_Rec.DirIn(PIN_PULL);
  Key_Stop.DirIn(PIN_PULL);
  Key_PlayF.DirIn(PIN_PULL);
  Key_Ffd.DirIn(PIN_PULL);
  Key_Rew.DirIn(PIN_PULL);
  Key_PlayR.DirIn(PIN_PULL);
  Key_Pause.DirIn(PIN_PULL);
  Key_Roll.DirIn(PIN_PULL);
}

//------------------------ Сканирование клавиатуры: --------------------------

uint8_t TLocal::GetCode(void)
{
  uint8_t k = 0;
  if(!Key_Rec)   k |= K_REC;
  if(!Key_Stop)  k |= K_STOP;
  if(!Key_PlayF) k |= K_PLAYF;
  if(!Key_Ffd)   k |= K_FFD;
  if(!Key_Rew)   k |= K_REW;
  if(!Key_PlayR) k |= K_PLAYR;
  if(!Key_Pause) k |= K_PAUSE;
  if(!Key_Roll)  k |= K_ROLL;

  if(k == 0) return(KEY_NO);

  uint8_t code;
  switch(k)
  {
  case K_RECPL: code = KEY_REC;   break;
  case K_STOP:  code = KEY_STOP;  break;
  case K_PLAYF: code = KEY_PLAYF; break;
  case K_FFD:   code = KEY_FFD;   break;
  case K_REW:   code = KEY_REW;   break;
  case K_PLAYR: code = KEY_PLAYR; break;
  case K_PAUSE: code = KEY_PAUSE; break;
  case K_ROLL:  code = KEY_ROLL;  break;
  default:      code = KEY_NO;
  }
  return(code);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
