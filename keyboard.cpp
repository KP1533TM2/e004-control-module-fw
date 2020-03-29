//----------------------------------------------------------------------------

//Абстрактный класс клавиатуры

//----------------------- Используемые ресурсы: ------------------------------

//Код кнопки получаем с помощью чисто виртуальной функции GetCode(),
//которая должна быть определена в производных классах.
//Дребезг кнопок подавляется программно: кнопка считается нажатой, если
//ее состояние не меняется в течение 50 мс. Кнопки способны генерировать
//несколько событий: нажатие, отпускание, удержание в течение 1 сек.,
//длинное удержание в течение 4 сек.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "keyboard.hpp"

//----------------------------------------------------------------------------
//--------------------------- Класс TKeyboard: -------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TKeyboard::TKeyboard(void)
{
  State = ST_DONE;
  Prev_Key = KEY_NO;
  LastCode = KEY_NO;
  Code = KEY_NO;
  KeyTimer = new TSoftTimer<TT_ONESHOT>();
}

//--------------------------- Обработка кнопок: ------------------------------

void TKeyboard::Execute(void)
{
  if(TSysTimer::Tick) //каждый системный тик (чаще не нужно)
  {
    uint8_t Key = GetCode();           //чтение кода кнопки
    if(Key != Prev_Key)                //состояние отличается
    {
      KeyTimer->Start(DEBOUNCE_TM);    //запуск таймера подавления дребезга
      Prev_Key = Key;                  //сохранение нового кода
      State = ST_NEW;                  //новое состояние клавиатуры
    }
    else                               //состояние совпадает
    {
      if(State != ST_DONE)             //если кнопка не обработана
      {
        if(KeyTimer->Over())           //если дребезг закончился
        {
          if(State == ST_NEW)          //если новое состояние
          {
            if(Key != KEY_NO)          //если есть нажатие
            {
              if(!(Code & MSG_REL))    //если не было сообщения отпускания
              {
                Code &= ~MSG_HOLD;     //удаление флага удержания кнопки
                Code |= MSG_REL;       //добавление флага отпускания кнопки
                KeyTimer->Start(DEBOUNCE_TM); //защитный интервал
              }
              else
              {
                Code = Key;            //сохранение кода кнопки
                KeyTimer->Start(HOLD_DELAY); //запуск таймера удержания кнопки
                State = ST_PRESS;      //нажатие обработано
              }
            }
            else                       //если нет нажатой кнопки
            {
              Code &= ~MSG_HOLD;       //удаление флага удержания кнопки
              Code |= MSG_REL;         //добавление флага отпускания кнопки
              State = ST_DONE;         //кнопка обработана
            }
          }
          else if(State == ST_PRESS)   //если кнопка удерживается нажатой
          {
            Code |= MSG_HOLD;          //сообщение - удержание кнопки
            State = ST_DONE;           //кнопка обработана
          }
        }
      }
    }
  }
}

//--------------------- Проверка нового сообщения: --------------------------

uint8_t TKeyboard::GetKeyCode(void)
{
  if(LastCode == Code) return(KEY_NO);
  LastCode = Code; return(Code);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
