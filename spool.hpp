//----------------------------------------------------------------------------

//Модуль управления боковыми двигателями, заголовочный файл

//----------------------------------------------------------------------------

#ifndef SPOOL_HPP
#define SPOOL_HPP

//----------------------------- Константы: -----------------------------------

static uint8_t const T_SAMPLE = 4; //период дискретизации PID-регуляторов, мс
static int32_t const TEN_MAX = UINT16_MAX; //макс. значение кода натяжения
static uint16_t const T_UNITS = 1000; //дискретность задания натяжения 0.1%

//преобразование процентов в код:
#define PERCENT2CODE(x) ((uint16_t)(x * 10.0 * TEN_MAX / T_UNITS))

enum SpoolMode_t //режимы подмотки
{
  SPOOL_OFF,
  SPOOL_BRAKE,
  SPOOL_PLAYF,
  SPOOL_PLAYR,
  SPOOL_FFD,
  SPOOL_REW,
  SPOOL_AFFD,
  SPOOL_AREW,
  SPOOL_MODES,
  //дополнительные режимы:
  SPOOL_FORCEF,
  SPOOL_FORCER,
};

typedef struct //коэффициенты PID
{
  uint8_t p;
  uint8_t i;
  uint8_t d;
} kpid_t;

typedef struct //набор натяжений для двигателей M1 и M2
{
  uint16_t m1;
  uint16_t m2;
} ten_t;

//----------------------------------------------------------------------------
//------------------------------ Класс TPid: ---------------------------------
//----------------------------------------------------------------------------

class TPid
{
private:
  uint16_t Xp;  //inp(n-1)
  uint16_t Xpp; //inp(n-2)
  int32_t Yp;   //out(n-1) * SCALE_Y
  static int32_t const SCALE_Y = 500; //масштаб Y
public:
  TPid(void);
  kpid_t K;     //коэффициенты PID
  uint16_t Ref; //задание для регулятора
  uint16_t Execute(uint16_t inp); //вычисление PID
  void Preset(uint16_t p); //предустановка PID-регулятора
};

//----------------------------------------------------------------------------
//---------------------------- Класс TSpool: ---------------------------------
//----------------------------------------------------------------------------

class TSpool
{
private:
  //значения по умолчанию:
  static uint8_t const NOM_KP = 70;
  static uint8_t const NOM_KI = 30;
  static uint8_t const NOM_KD = 40;
  static uint16_t const NOM_TEN_MIN  = PERCENT2CODE(8.0);
  static uint16_t const NOM_TEN_PLAY = PERCENT2CODE(25.0);
  static uint16_t const NOM_TEN_LIMT = PERCENT2CODE(100.0);
  static uint16_t const NOM_TEN_ARSP = PERCENT2CODE(15.0);
  static uint16_t const NOM_TEN_ARTU = PERCENT2CODE(30.0);
  static uint16_t const DELTAP = PERCENT2CODE(5.0); //play force delta (Wind)
  static uint16_t const DELTAM = PERCENT2CODE(5.0); //play force delta (Back)
  //константы:
  static uint8_t const ADC_DIV = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  static uint8_t const ADC_ST =  ADC_DIV | (1 << ADEN) | (1 << ADSC);
  static uint8_t const ADC_REF = (1 << REFS1) | (1 << REFS0);
  static uint8_t const ADMUX_T1 = ADC_REF | (1 << MUX0); //вход ADC1 (TEN1)
  static uint8_t const ADMUX_T2 = ADC_REF;         //вход ADC0 (TEN2)
  static uint8_t const CHANS = 2;                  //количество каналов АЦП
  static uint8_t const NSAMPLE = T_SAMPLE / CHANS; //количество сэмплов на канал
  static uint8_t const SAMPLES = NSAMPLE * CHANS;  //общее количество сэмплов
  static uint16_t const ADC_MAX = 0x3FF;           //максимальный код АЦП
  static uint16_t const PWM_MAX = 0x3FFF;          //14-bit PWM
  static uint16_t const PWM_SCALE = TEN_MAX / PWM_MAX;

  uint8_t Mode;             //режим подмотки
  uint8_t MotMode;          //режим работы двигателей
  enum MotMode_t            //коды режимов работы двигателей
  {
    MOT_STOP,
    MOT_PLAY,
    MOT_FFD,
    MOT_REW,
    MOT_AREW,
    MOT_AFFD
  };
  uint8_t SampCnt;          //счетчик сэмплов АЦП
  uint16_t AdcCode1;        //код АЦП канала 1
  uint16_t AdcCode2;        //код АЦП канала 2
  bool fUpd1, fUpd2;        //флаги обновления натяжений
  uint16_t Adc2Ten(uint16_t adc); //преобразование кода АЦП в код натяжения
  uint16_t Sen1;            //текущее значение датчика натяжения 1
  uint16_t Sen2;            //текущее значение датчика натяжения 2
  uint16_t Mot1;            //текущее управляющее воздействие мотора 1
  uint16_t Mot2;            //текущее управляющее воздействие мотора 2

  Pin_AdcT1_t Pin_AdcT1;    //вход ADC1 (датчик натяжения TEN1)
  Pin_AdcT2_t Pin_AdcT2;    //вход ADC0 (датчик натяжения TEN2)
  Pin_PwmM1_t Pin_PwmM1;    //выход OC1B (PWM MOTOR1)
  Pin_PwmM2_t Pin_PwmM2;    //выход OC1A (PWM MOTOR2)
  Pin_Ffd_t Pin_Ffd;        //включение мотора перемотки вперед
  Pin_Rew_t Pin_Rew;        //включение мотора перемотки назад
  TPid *Pid_M1;             //PID-регулятор мотора M1
  TPid *Pid_M2;             //PID-регулятор мотора M2
  kpid_t K;                 //коэффициенты PID

  ten_t Tensions[SPOOL_MODES]; //массив натяжений
  void SetTension(uint8_t m);  //задание натяжений для PID-регуляторов

public:
  TSpool(void);
  void Execute(void);
  void SetMode(uint8_t m);  //включение режима подмотки
  uint8_t GetMode(void);    //чтение режима подмотки
  void SetPID(kpid_t k);    //установка коэффициентов PID
  kpid_t GetPID(void);      //чтение коэффициентов PID
  void SetMot1(uint16_t m); //управление мотором M1
  void SetMot2(uint16_t m); //управление мотором M2
  ten_t GetMot(void);       //чтение моторов
  void SetTen(uint8_t m, ten_t t); //установка натяжения
  ten_t GetTen(uint8_t m);  //чтение натяжения
  void SetTen(ten_t t);     //установка текущего натяжения
  ten_t GetTen(void);       //чтение текущего натяжения
  ten_t GetSen(void);       //чтение датчиков натяжения
  bool LowT1(void);         //проверка натяжения T1
  bool LowT2(void);         //проверка натяжения T2
  bool LowT1T2(void);       //проверка натяжений T1 и T2
  void EERead(void);        //чтение параметров из EEPROM
  void EESave(void);        //сохранение параметров в EEPROM
};

//----------------------------------------------------------------------------

#endif
