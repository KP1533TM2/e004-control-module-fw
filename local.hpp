//----------------------------------------------------------------------------

//Модуль поддержки местной клавиатуры: header file

//----------------------------------------------------------------------------

#ifndef LOCAL_HPP
#define LOCAL_HPP

//----------------------------------------------------------------------------

#include "keyboard.hpp"
#include "remote.hpp"

//----------------------------------------------------------------------------
//----------------------------- Класс TLocal: --------------------------------
//----------------------------------------------------------------------------

class TLocal : public TKeyboard
{
private:
  Key_Rec_t   Key_Rec;
  Key_Stop_t  Key_Stop;
  Key_PlayF_t Key_PlayF;
  Key_Ffd_t   Key_Ffd;
  Key_Rew_t   Key_Rew;
  Key_PlayR_t Key_PlayR;
  Key_Pause_t Key_Pause;
  Key_Roll_t  Key_Roll;
  virtual uint8_t GetCode(void);
  enum
  {
    K_REC   = 1 << 0,
    K_STOP  = 1 << 1,
    K_PLAYF = 1 << 2,
    K_FFD   = 1 << 3,
    K_REW   = 1 << 4,
    K_PLAYR = 1 << 5,
    K_PAUSE = 1 << 6,
    K_ROLL  = 1 << 7
  };
public:
  TLocal(void);
};

//----------------------------------------------------------------------------

#endif
