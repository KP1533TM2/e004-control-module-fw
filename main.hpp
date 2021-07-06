//----------------------------------------------------------------------------

//Главный модуль, заголовочный файл

//----------------------------------------------------------------------------

#ifndef MAIN_HPP
#define MAIN_HPP

//----------------------------------------------------------------------------

#include <avr/io.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "gpio.hpp"

//-------------------------- Номер версии firmware: --------------------------

#define VERSION "1.21"

//--------------- Использование портов процессора ATmega64: ------------------

#ifdef REV_A
  #include "board_a.hpp"
#endif

#ifdef REV_C
  #include "board_c.hpp"
#endif

//-------------------- Использование аппаратных блоков: ----------------------

//Timer0:  IR decoder [remote]
//Timer1:  motor control PWM [spool]
//Timer2:  sound generation [sound]
//Timer3:  system timer [systimer]
//USART1:  computer port [wakeport]
//ADC0:    tension sensor 2 [spool]
//ADC1:    tension sensor 1 [spool]
//ACOMP:   tape end sensor [control]

//----------------------- Использование прерываний: --------------------------

//INT4:          IR receiver [remote]
//TIMER0_OVF:    IR decoder [remote]
//TIMER3_COMPA:  system timer [systimer]

//---------------------------- Макроопределения: -----------------------------

#define LO(x)    ((uint8_t)((x) & 0xFF))
#define HI(x)    ((uint8_t)(((x) >> 8) & 0xFF))
#define BYTE1(x) (LO(x))
#define BYTE2(x) (HI(x))
#define BYTE3(x) ((uint8_t)(((uint32_t)(x) >> 16) & 0xFF))
#define BYTE4(x) ((uint8_t)(((uint32_t)(x) >> 24) & 0xFF))

#define WORD(b1,b0)        (((uint16_t)(b1) << 8) | (b0))
#define DWORD(b3,b2,b1,b0) (((uint32_t)WORD(b3,b2) << 16) | WORD(b1,b0))

#define ABS(x) ((x < 0)? -x : x)

#ifndef NULL
  #define NULL _NULL
#endif

//----------------------------------------------------------------------------

#endif
