//----------------------------------------------------------------------------

//Использование портов процессора ATmega64 для платы Rev. A:

//----------------------------------------------------------------------------

#ifndef BOARD_A_HPP
#define BOARD_A_HPP

//----------------------------------------------------------------------------

typedef TGpio<PORT_A, PIN_0> Pin_XT4_t;    //PA0 - test pin XT4 [control]
typedef TGpio<PORT_A, PIN_1> Pin_XT5_t;    //PA1 - test pin XT5 [control]
typedef TGpio<PORT_A, PIN_2> Pin_XT6_t;    //PA2 - test pin XT6 [control]
typedef TGpio<PORT_A, PIN_3> Key_Roll_t;   //PA3 - кнопка ОТКАТ [local]
typedef TGpio<PORT_A, PIN_4> Key_Pause_t;  //PA4 - кнопка ПАУЗА [local]
typedef TGpio<PORT_A, PIN_5> Led_Pause_t;  //PA5 - светодиод ПАУЗА [leds]
typedef TGpio<PORT_A, PIN_6> Key_PlayR_t;  //PA6 - кнопка ВОСПР. РЕВЕРС [local]
typedef TGpio<PORT_A, PIN_7> Led_PlayR_t;  //PA7 - светодиод ВОСПР. РЕВЕРС [leds]

typedef TGpio<PORT_B, PIN_0> Pin_Ffd_t;    //PB0 - перемотка вперед [spool]
typedef TGpio<PORT_B, PIN_1> Pin_PB1_t;    //PB1 (SCK) - не используется [control]
typedef TGpio<PORT_B, PIN_2> Pin_Rev_t;    //PB2 - реверс вед. двигателя [capstan]
typedef TGpio<PORT_B, PIN_3> Pin_Tmc_t;    //PB3 - пост. времени ДД [transport]
typedef TGpio<PORT_B, PIN_4> Pin_PressH_t; //PB4 - ЭМ ролика - удерж. [transport]
typedef TGpio<PORT_B, PIN_5> Pin_PwmM2_t;  //PB5 (OC1A) - PWM бок. двиг. 2 [spool]
typedef TGpio<PORT_B, PIN_6> Pin_PwmM1_t;  //PB6 (OC1B) - PWM бок. двиг. 1 [spool]
typedef TGpio<PORT_B, PIN_7> Pin_SND_t;    //PB7 (OC2) - выход звука [sound]

typedef TGpio<PORT_C, PIN_0> Led_Rec_t;    //PC0 - светодиод ЗАПИСЬ [leds]
typedef TGpio<PORT_C, PIN_1> Key_Stop_t;   //PC1 - кнопка СТОП [local]
typedef TGpio<PORT_C, PIN_2> Led_Stop_t;   //PC2 - светодиод СТОП [leds]
typedef TGpio<PORT_C, PIN_3> Key_PlayF_t;  //PC3 - кнопка ВОСПР. [local]
typedef TGpio<PORT_C, PIN_4> Led_PlayF_t;  //PC4 - светодиод ВОСПР. [leds]
typedef TGpio<PORT_C, PIN_5> Led_Ffd_t;    //PC5 - светодиод ПЕРЕМ. ВПЕРЕД [leds]
typedef TGpio<PORT_C, PIN_6> Key_Ffd_t;    //PC6 - кнопка ПЕРЕМ. ВПЕРЕД [local]
typedef TGpio<PORT_C, PIN_7> Led_Rew_t;    //PC7 - светодиод ПЕРЕМ. НАЗАД [leds]

typedef TGpio<PORT_D, PIN_0> Pin_SCL_t;    //PD0 (SCL) - порт I2C [control]
typedef TGpio<PORT_D, PIN_1> Pin_SDA_t;    //PD1 (SDA) - порт I2C [control]
typedef TGpio<PORT_D, PIN_2> Pin_RXD1_t;   //PD2 (RXD1) - порт RS-485 [control]
typedef TGpio<PORT_D, PIN_3> Pin_TXD1_t;   //PD3 (TXD1) - порт RS-485 [control]
typedef TGpio<PORT_D, PIN_4> Pin_LiftH_t;  //PD4 - ЭМ отвода - удерж. [transport]
typedef TGpio<PORT_D, PIN_5> Pin_LiftF_t;  //PD5 - ЭМ отвода - старт [transport]
typedef TGpio<PORT_D, PIN_6> Pin_BrakeF_t; //PD6 - ЭМ тормоз - старт [transport]
typedef TGpio<PORT_D, PIN_7> Pin_PressF_t; //PD7 - ЭМ ролика - старт [transport]

typedef TGpio<PORT_E, PIN_0> Pin_RXD0_t;   //PE0 (RXD0) - порт компьютера [wakeport]
typedef TGpio<PORT_E, PIN_1> Pin_TXD0_t;   //PE1 (TXD0) - порт компьютера [wakeport]
typedef TGpio<PORT_E, PIN_2> Pin_End0_t;   //PE2 - порог ДО (AIN0) [endsensor]
typedef TGpio<PORT_E, PIN_3> Pin_End1_t;   //PE3 - вход ДО (AIN1) [endsensor]
typedef TGpio<PORT_E, PIN_4> Pin_IR_t;     //PE4 - вход ДУ (INT4) [remote]
typedef TGpio<PORT_E, PIN_5> Pin_Move_t;   //PE5 - вход ДД [movesensor]
typedef TGpio<PORT_E, PIN_6> Pin_Rec_t;    //PE6 - включение записи [audio]
typedef TGpio<PORT_E, PIN_7> Pin_Rew_t;    //PE7 - перемотка назад [spool]

typedef TGpio<PORT_F, PIN_0> Pin_AdcT2_t;  //PF0 (ADC0) - датчик натяж. [spool]
typedef TGpio<PORT_F, PIN_1> Pin_AdcT1_t;  //PF1 (ADC1) - датчик натяж. [spool]
typedef TGpio<PORT_F, PIN_2> Pin_Mute_t;   //PF2 - блокировка УВ [audio]
typedef TGpio<PORT_F, PIN_3> Pin_PlayF_t;  //PF3 - вкл. прямого канала УВ [audio]
typedef TGpio<PORT_F, PIN_4> Pin_PlayR_t;  //PF4 - вкл. реверс. канала УВ [audio]
typedef TGpio<PORT_F, PIN_5> Pin_XT1_t;    //PF5 - test pin XT1 [control]
typedef TGpio<PORT_F, PIN_6> Pin_XT2_t;    //PF6 - test pin XT2 [control]
typedef TGpio<PORT_F, PIN_7> Pin_XT3_t;    //PF7 - test pin XT3 [control]

typedef TGpio<PORT_G, PIN_0> Pin_Pvg_t;    //PG0 - вход контроля питания [control]
typedef TGpio<PORT_G, PIN_1> Key_Rec_t;    //PG1 - кнопка ЗАПИСЬ [local]
typedef TGpio<PORT_G, PIN_2> Key_Rew_t;    //PG2 - кнопка ПЕРЕМ. НАЗАД [local]
typedef TGpio<PORT_G, PIN_3> Pin_BrakeH_t; //PG3 - ЭМ тормоз - удерж. [transport]
typedef TGpio<PORT_G, PIN_4> Pin_Dir_t;    //PG4 - направление RS-485 [control]

//----------------------------------------------------------------------------

#endif
