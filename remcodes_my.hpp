//----------------------------------------------------------------------------

//Табица перекодировки кнопок ДУ

//----------------------------------------------------------------------------

#ifndef REMCODES_HPP
#define REMCODES_HPP

//----------------------------------------------------------------------------

#include "keyboard.hpp"

//----------------------------------------------------------------------------

//коды функций, задействованных для управления:

//KEY_STOP - стоп
//KEY_PLAYF - воспроизведение вперед
//KEY_PLAYR - воспроизведение назад
//KEY_PLAY - воспроизведение в текущем направлении
//KEY_REV - реверс текущего направления
//KEY_REC - запись
//KEY_PAUSE - пауза
//KEY_FFD - перемотка вперед
//KEY_REW - перемотка назад
//KEY_ROLL - откат
//KEY_MUTE - mute
//KEY_PROG - вход в режим программирования последовательности

//таблица перекодировки кода кнопки в код команды:

__flash uint8_t const RC5_Codes[] =
{
/*NSYS*/ 0x13,
/*0x00*/ NC,
/*0x01*/ NC,
/*0x02*/ KEY_ROLL,
/*0x03*/ NC,
/*0x04*/ NC,
/*0x05*/ KEY_REV,
/*0x06*/ NC,
/*0x07*/ KEY_PLAY,
/*0x08*/ NC,
/*0x09*/ NC,
/*0x0A*/ NC,
/*0x0B*/ NC,
/*0x0C*/ NC,
/*0x0D*/ KEY_REC,
/*0x0E*/ NC,
/*0x0F*/ KEY_REW,
/*0x10*/ NC,
/*0x11*/ NC,
/*0x12*/ NC,
/*0x13*/ NC,
/*0x14*/ NC,
/*0x15*/ NC,
/*0x16*/ KEY_PAUSE,
/*0x17*/ NC,
/*0x18*/ NC,
/*0x19*/ NC,
/*0x1A*/ NC,
/*0x1B*/ NC,
/*0x1C*/ NC,
/*0x1D*/ KEY_FFD,
/*0x1E*/ KEY_STOP,
/*0x1F*/ KEY_MUTE,
/*0x20*/ NC,
/*0x21*/ NC,
/*0x22*/ NC,
/*0x23*/ NC,
/*0x24*/ NC,
/*0x25*/ NC,
/*0x26*/ NC,
/*0x27*/ NC,
/*0x28*/ NC,
/*0x29*/ NC,
/*0x2A*/ NC,
/*0x2B*/ NC,
/*0x2C*/ NC,
/*0x2D*/ NC,
/*0x2E*/ NC,
/*0x2F*/ NC,
/*0x30*/ NC,
/*0x31*/ NC,
/*0x32*/ NC,
/*0x33*/ NC,
/*0x34*/ NC,
/*0x35*/ NC,
/*0x36*/ NC,
/*0x37*/ NC,
/*0x38*/ NC,
/*0x39*/ NC,
/*0x3A*/ NC,
/*0x3B*/ NC,
/*0x3C*/ NC,
/*0x3D*/ NC,
/*0x3E*/ NC,
/*0x3F*/ NC
};

//----------------------------------------------------------------------------

#endif
