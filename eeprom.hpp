//----------------------------------------------------------------------------

//Модуль поддержки EEPROM, заголовочный файл

//----------------------------------------------------------------------------

#ifndef EEPROM_HPP
#define EEPROM_HPP

//----------------------------- Карта EEPROM: --------------------------------

enum EE_Map_t
{
  EE_SIGN,        EE_SIGN_H,
  //Capstan:
  EE_CAP_START,   EE_CAP_START_H,
  //EndSensor:
  EE_END_TAU,     EE_END_TAU_H,
  //Spool:
  EE_KP1,
  EE_KI1,
  EE_KD1,
  EE_KP2,
  EE_KI2,
  EE_KD2,
  EE_TEN_MIN1,    EE_TEN_MIN1_H,
  EE_TEN_MIN2,    EE_TEN_MIN2_H,
  EE_TEN_BRAKE1,  EE_TEN_BRAKE1_H,
  EE_TEN_BRAKE2,  EE_TEN_BRAKE2_H,
  EE_TEN_PLAYF1,  EE_TEN_PLAYF1_H,
  EE_TEN_PLAYF2,  EE_TEN_PLAYF2_H,
  EE_TEN_PLAYR1,  EE_TEN_PLAYR1_H,
  EE_TEN_PLAYR2,  EE_TEN_PLAYR2_H,
  EE_TEN_FFD1,    EE_TEN_FFD1_H,
  EE_TEN_FFD2,    EE_TEN_FFD2_H,
  EE_TEN_REW1,    EE_TEN_REW1_H,
  EE_TEN_REW2,    EE_TEN_REW2_H,
  EE_TEN_AFFD1,   EE_TEN_AFFD1_H,
  EE_TEN_AFFD2,   EE_TEN_AFFD2_H,
  EE_TEN_AREW1,   EE_TEN_AREW1_H,
  EE_TEN_AREW2,   EE_TEN_AREW2_H,
  //Transport:
  EE_TR_OPTIONS,
  EE_AS_BRK_DEL,  EE_AS_BRK_DEL_H,
  EE_AS_PRE_DEL,  EE_AS_PRE_DEL_H,
  EE_AS_TEN_DEL,  EE_AS_TEN_DEL_H,
  EE_EM_FORCE,    EE_EM_FORCE_H,
  //Control:
  EE_CT_OPTIONS,  EE_CT_OPTIONS_H,
  //Size:
  EE_PARAMS
};

//----------------------------------------------------------------------------
//----------------------------- Класс TEeprom: -------------------------------
//----------------------------------------------------------------------------

class TEeprom
{
private:
  static __no_init __eeprom uint8_t EEData[EE_PARAMS];
  static uint16_t const EE_SIGNATURE = 0xBE20;   //сигнатура EEPROM
  bool fValid;
protected:
public:
  TEeprom(void);
  uint8_t Rd8(uint8_t addr, uint8_t def = 0);    //чтение 8 бит
  void Wr8(uint8_t addr, uint8_t val);           //запись 8 бит
  uint16_t Rd16(uint8_t addr, uint16_t def = 0); //чтение 16 бит
  void Wr16(uint8_t addr, uint16_t val);         //запись 16 бит
  void Validate(void);                           //запись сигнатуры
  void Invalidate(void);                         //сброс флага валидности
};

//----------------------------------------------------------------------------

extern TEeprom *Eeprom;

//----------------------------------------------------------------------------

#endif
