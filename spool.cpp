//----------------------------------------------------------------------------

//Модуль управления боковыми двигателями

//----------------------- Используемые ресурсы: ------------------------------

//Класс TSpool осуществляет управление боковыми двигателями.

//Датчики натяжения поочередно считываются с помощью АЦП, период выборок 1 мс.
//Несколько выборок усредняются, в результате получается период дискретизации,
//задаваемый константой T_SAMPLE. Код АЦП приводится к диапазону 0…65535.
//Используется внутренний ИОН, полная шкала АЦП составляет 2.56 В.
//Подстроечными резисторами на платах УРНЛ надо установить в контрольных
//точках TP1 и TP2 напряжение 2.56 В при крайнем нижнем положении натяжителей
//(нулевое наяжение ленты). В крайнем верхнем положении (максимальное натяжение
//ленты) напряжение должно быть около 0 В.

//Управление двигателями производится аналоговым сигналом, который формируется
//с помощью ШИМ. Используется 14-разрядный ШИМ. Код ШИМ приводится к диапазону
//0…65535, а диапазон выходного напряжения равен напряжению питания процессора.
//Сначала ШИМ-сигнал делится, что приводит его шкалу к диапазону 0…2.56 В.
//Затем он фильтруется с помощью ФНЧ 3-го порядка с частотой среза около 30 Гц.
//Характеристика фильтра подобрана так, чтобы  минимизировать выброс
//на переходной характеристике, который не превышает 1% (для сравнения,
//у фильтра Баттерворта он составляет около 9%). Время установления фильтра
//по уровню 0.9 - около 15 мс. С выхода фильтра (контрольные точки TP3 и TP4)
//управляющий сигнал поступает на выходной каскад, собранный на двух
//транзисторах разной структуры. Этот каскад формирует требуемый ток базы
//для регулирующего транзистора, включенного в цепь двигателя.
//Оба транзистора выходного каскада включены с ОЭ, что позволяет получить
//на выходе размах напряжения, близкий к напряжению питания (несмотря на то,
//что на выходе ОУ активного фильтра размах напряжения заметно ниже).
//Это необходимо для того, чтобы иметь возможность питать выходной каскад от
//источника +5 В и не нагружать источник аналогового питания +15 В пульсирующим
//током. Ток базы регулирующего транзистора промодулирован в форме модуля
//синуса, что дает минимальные искажения формы тока, питающего двигатель.
//Модуляция осуществляется за счет сигнала обратной связи, поступающего
//через резисторы с коллектора регулирующего транзистора. Кроме того, добавлено
//напряжение смещения, которое минимизирует нелинейность, связанную с
//падением напряжения на переходе Б-Э регулирующего транзистора.

//В режиме рабочего хода (SPOOL_PLAY) натяжение ленты стабилизируется
//со стороны каждого из двигателей по своему датчику натяжения. Измеренное
//и требуемое натяжение ленты поступают на PID-регулятор, он формирует
//величину управляющего напряжения для двигателя, которая загружается в ШИМ.
//Требуемое натяжение в процессе настройки аппарата задается с компьютера
//в виде значения в диапазоне 0…65535 и сохраняется в EEPROM.
//Коэффициенты PID имеют диапазон 0…255 и также задаются с компьютера и
//сохраняются в EEPROM. Коэффициенты подбираются по критерию гладкой
//переходной характеристики натяжения ленты в переходных режимах. Натяжение
//должно иметь минимальные выбросы, а колебания должны быстро затухать.
//Графики натяжения в реальном времени можно наблюдать на экране компьютера.
//В режиме рабочего хода натяжения на подающей и приемной стороне могут
//быть установлены разными для балансировки натяжений с двух сторон ведущего
//вала (с учетом роста натяжения в рабочей зоне из-за трения ленты о стойки
//и головки).

//В режиме перемоток принимающий двигатель питается напряжением 100 В и не
//регулируется. Подающий двигатель регулируется по датчику натяжения на
//принимающей стороне.

//В режиме архивной перемотки подающий и принимающий двигатели регулируются
//по датчику натяжения на принимающей стороне. Для принимающего двигателя
//установлено натяжение больше, чем для подающего. В результате принимающий
//двигатель фактически питается напряжением 60 В, в случае необходимости
//оно снижается для ограничения натяжения на заданном уровне. Подающий
//двигатель поддерживает заданное натяжение для равномерной намотки ленты
//на катушку.

//В режиме торможения двигателями натяжение регулируется так же, как в режиме
//рабочего хода. Натяжения справа и слева должны быть установлены одинаковыми,
//чтобы лента при остановке находилась в равновесии.

//Для каждого из режимов и для каждого двигателя натяжение может быть
//установлено индивидуально с помощью вервисной программы. Натяжение задется
//в процентах относительно полного хода рычагов натяжителей. Задаются следующие
//натяжения: Brake (торможение двигателями), PlayF (рабочий ход вперед),
//PlayR (рабочий ход назад), FFD (перемотка вперед), REW (перемотка назад),
//AFFD (архивная перемотка вперед), AREW (архивная перемотка назад),
//Autostop (минимальное натяжение, при достижении которого будет срабатывать
//автостоп).

//----------------------------------------------------------------------------

#include "main.hpp"
#include "spool.hpp"
#include "control.hpp"

//----------------------------------------------------------------------------
//------------------------------ Класс TPid: ---------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TPid::TPid(void)
{
  Yp = 0;
  Xp = 0;
  Xpp = 0;
  K.p = 0;
}

//----------------------------- PID-регулятор: -------------------------------

uint16_t TPid::Execute(uint16_t inp)
{
  //PID-регулятор реализован в дифференциальной форме:
  //Y(n) = Y(n-1) - Kp*[X(n)-X(n-1)] + Ki*Err(n) - Kd*[X(n)-2*X(n-1)+X(n-2)]
  int32_t Y = Yp;
  //учет пропорциональной составляющей:
  int32_t div1 = (int32_t)inp - Xp;
  Y = Y - (int32_t)K.p * div1 * (SCALE_Y / 10);
  //учет интегральной составляющей:
  int32_t err = (int32_t)Ref - inp;
  Y = Y + (int32_t)K.i * err * (SCALE_Y * T_SAMPLE / 1000);
  //учет дифференциальной составляющей:
  if((Y > 0) && (Y < (TEN_MAX * SCALE_Y))) //при ограничении D отключается
  {
    int32_t div2 = (int32_t)inp - 2 * (int32_t)Xp + Xpp;
    Y = Y - (int32_t)K.d * div2 * (SCALE_Y * 10 / T_SAMPLE);
  }
  //ограничение выходного значения:
  if(Y < 0) Y = 0;
  if(Y > (TEN_MAX * SCALE_Y)) Y = (TEN_MAX * SCALE_Y);
  //обновление величин:
  Yp = Y;
  Xpp = Xp;
  Xp = inp;
  return(Y / SCALE_Y);
}

//--------------------- Предустановка PID-регулятора: ------------------------

void TPid::Preset(uint16_t p)
{
  Yp = p * SCALE_Y;
  //Xp = p;
  //Xpp = p;
}

//----------------------------------------------------------------------------
//----------------------------- Класс TSpool: --------------------------------
//----------------------------------------------------------------------------

//------------- Вспомогательные inline-функции (замена макросам): ------------

//Преобразование суммарного кода АЦП в код натяжения 0..TEN_MAX.
//Выходное напряжение датчиков натяжения падает при увеличении натяжения.

inline uint16_t TSpool::Adc2Ten(uint16_t adc)
{
  return(TEN_MAX - (uint32_t)adc * (TEN_MAX / ADC_MAX) / NSAMPLE);
}

//----------------------------- Конструктор: ---------------------------------

TSpool::TSpool(void)
{
  Pin_AdcT1.DirIn();
  Pin_AdcT2.DirIn();
  Pin_PwmM1.DirOut();
  Pin_PwmM2.DirOut();
  Pin_Ffd.DirOut();
  Pin_Rew.DirOut();

  Pid_M1 = new TPid();
  Pid_M2 = new TPid();

  vAdcCounter = 0;
  vAdcCode1 = 0;
  vAdcCode2 = 0;
  Sen1 = 0; Sen2 = 0;
  //настройка АЦП:
  ADMUX = ADMUX_T1;
  ADCSRA = ADC_SET;
  //настройка таймера 1:
  ICR1 = PWM_MAX; //PWM TOP
  SetMot1(0);     //PWM1 = 0
  SetMot2(0);     //PWM2 = 0
  //OC1A, OC1B set on compare, fast PWM:
  TCCR1A = (1<<COM1A1) | (1<<COM1A0) | (1<<COM1B1) | (1<<COM1B0) | (1<<WGM11);
  //Fast PWM, CK/1:
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

  SetMode(SPOOL_OFF);
}

//--------------------------- Прерывание АЦП: --------------------------------

volatile uint8_t TSpool::vAdcCounter;
uint16_t TSpool::vAdcCode1;
uint16_t TSpool::vAdcCode2;

#pragma vector = ADC_vect
__interrupt void Adc_Handler(void)
{
  if(TSpool::vAdcCounter < TSpool::SAMPLES)
  {
    TSpool::vAdcCounter++;
    if(ADMUX == TSpool::ADMUX_T2)
    {
      TSpool::vAdcCode1 += ADC;
      ADMUX = TSpool::ADMUX_T1;
    }
    else
    {
      TSpool::vAdcCode2 += ADC;
      ADMUX = TSpool::ADMUX_T2;
    }
  }
}

//------------------------ Выполнение управления: ----------------------------

void TSpool::Execute(void)
{
  //измерение натяжения Tension1 и Tension2:
  if(vAdcCounter == SAMPLES)
  {
    //чтение АЦП:
    uint16_t code1 = vAdcCode1;
    uint16_t code2 = vAdcCode2;
    vAdcCode1 = 0;
    vAdcCode2 = 0;
    vAdcCounter = 0;
    //вычисление натяжения:
    Sen1 = Adc2Ten(code1);
    Sen2 = Adc2Ten(code2);
    //управление двигателями:
    switch(MotMode)
    {
    case MOT_PLAY:
      SetMot1(Pid_M1->Execute(Sen1)); //M1 <- датчик L
      SetMot2(Pid_M2->Execute(Sen2)); //M2 <- датчик R
      break;
    case MOT_FFD:
      SetMot1(Pid_M1->Execute(Sen2)); //M1 <- датчик R
      break;                          //M2 <- не регулируется
    case MOT_REW:
      SetMot2(Pid_M2->Execute(Sen1)); //M2 <- датчик L
      break;                          //M1 <- не регулируется
    case MOT_AFFD:
      if(Sen1 > Sen2)
        SetMot1(Pid_M1->Execute(Sen1)); //M1 <- датчик L
          else SetMot1(Pid_M1->Execute(Sen2)); //M1 <- датчик R
      SetMot2(Pid_M2->Execute(Sen2)); //M2 <- датчик R
      break;
    case MOT_AREW:
      SetMot1(Pid_M1->Execute(Sen1)); //M1 <- датчик L
      if(Sen2 > Sen1)
        SetMot2(Pid_M2->Execute(Sen2)); //M2 <- датчик R
          else SetMot2(Pid_M2->Execute(Sen1)); //M2 <- датчик L
      break;
    default:
      Pid_M1->Execute(Sen1);
      Pid_M2->Execute(Sen2);
    }
  }
}

//---------------- Задание натяжений для PID-регуляторов: --------------------

void TSpool::SetTension(uint8_t m)
{
  if(m == SPOOL_FORCEF)
  {
    //усиление подмотки и ослабление подтормаживания при старте вперед:
    Pid_M1->Ref = Tensions[SPOOL_PLAYF].m1 - DELTAM;
    Pid_M2->Ref = Tensions[SPOOL_PLAYF].m2 + DELTAP;
  }
  else if(m == SPOOL_FORCER)
  {
    //усиление подмотки и ослабление подтормаживания при старте назад:
    Pid_M1->Ref = Tensions[SPOOL_PLAYR].m1 + DELTAP;
    Pid_M2->Ref = Tensions[SPOOL_PLAYR].m2 - DELTAM;
  }
  else if(m == SPOOL_OFF)
  {
    //выключение боковых моторов (Tensions[SPOOL_OFF] = Min. tension!)
    Pid_M1->Ref = 0;
    Pid_M2->Ref = 0;
  }
  else
  {
    Pid_M1->Ref = Tensions[m].m1;
    Pid_M2->Ref = Tensions[m].m2;
  }
}

//----------------------------------------------------------------------------
//------------------------- Интерфейсные методы: -----------------------------
//----------------------------------------------------------------------------

//-------------------------- Включение режима: -------------------------------

void TSpool::SetMode(uint8_t m)
{
  Pin_Ffd = 0;
  Pin_Rew = 0;
  SetTension(m);

  switch(m)
  {
  case SPOOL_OFF:
    SetMot1(0);
    SetMot2(0);
    Pid_M1->Preset(0);
    Pid_M2->Preset(0);
    MotMode = MOT_STOP;
    break;

  case SPOOL_FORCEF:
  case SPOOL_PLAYF:
    Pid_M1->Preset(0);
    Pid_M2->Preset(TEN_MAX);
    MotMode = MOT_PLAY;
    break;

  case SPOOL_FORCER:
  case SPOOL_PLAYR:
    Pid_M1->Preset(TEN_MAX);
    Pid_M2->Preset(0);
    MotMode = MOT_PLAY;
    break;

  case SPOOL_BRAKE:
    Pid_M1->Preset(0);
    Pid_M2->Preset(0);
    MotMode = MOT_PLAY;
    break;

  case SPOOL_FFD:
    SetMot2(0);
    Pid_M1->Preset(0);
    Pin_Ffd = 1;
    MotMode = MOT_FFD;
    break;

  case SPOOL_REW:
    SetMot1(0);
    Pid_M2->Preset(0);
    Pin_Rew = 1;
    MotMode = MOT_REW;
    break;

  case SPOOL_AFFD:
    Pid_M1->Preset(0);
    Pid_M2->Preset(TEN_MAX);
    MotMode = MOT_AFFD;
    break;

  case SPOOL_AREW:
    Pid_M1->Preset(TEN_MAX);
    Pid_M2->Preset(0);
    MotMode = MOT_AREW;
    break;
  }
  Mode = m;
}

//--------------------------- Чтение режима: ---------------------------------

uint8_t TSpool::GetMode(void)
{
  return(Mode);
}

//--------------------- Установка коэффициентов PID: -------------------------

void TSpool::SetPID(kpid_t k)
{
  K = k;
  Pid_M1->K = K;
  Pid_M2->K = K;
}

//----------------------- Чтение коэффициентов PID: --------------------------

kpid_t TSpool::GetPID(void)
{
  return(K);
}

//------------------------- Управление мотором M1: ---------------------------

void TSpool::SetMot1(uint16_t m)
{
  Mot1 = m;
  OCR1B = (TEN_MAX - m) / PWM_SCALE;
}

//------------------------- Управление мотором M2: ---------------------------

void TSpool::SetMot2(uint16_t m)
{
  Mot2 = m;
  OCR1A = (TEN_MAX - m) / PWM_SCALE;
}

//---------------------------- Чтение моторов: -------------------------------

ten_t TSpool::GetMot(void)
{
  ten_t m;
  m.m1 = Mot1;
  m.m2 = Mot2;
  return(m);
}

//--------------------------- Установка натяжения: ---------------------------

void TSpool::SetTen(uint8_t m, ten_t t)
{
  if(m < SPOOL_MODES)
  {
    Tensions[m].m1 = t.m1;
    Tensions[m].m2 = t.m2;
    if(m == Mode)
    {
      Pid_M1->Ref = t.m1;
      Pid_M2->Ref = t.m2;
    }
  }
}

//---------------------------- Чтение натяжения: -----------------------------

ten_t TSpool::GetTen(uint8_t mode)
{
  return(Tensions[mode]);
}

//---------------------- Установка текущего натяжения: -----------------------

void TSpool::SetTen(ten_t t)
{
  Pid_M1->Ref = t.m1;
  Pid_M2->Ref = t.m2;
}

//----------------------- Чтение текущего натяжения: -------------------------

ten_t TSpool::GetTen(void)
{
  ten_t t;
  t.m1 = Pid_M1->Ref;
  t.m2 = Pid_M2->Ref;
  return(t);
}

//-------------- Чтение реального натяжения (чтение датчиков): ---------------

ten_t TSpool::GetSen(void)
{
  ten_t t;
  t.m1 = Sen1;
  t.m2 = Sen2;
  return(t);
}

//-------------- Проверка падения натяжения T1 ниже минимума: ----------------

bool TSpool::LowT1(void)
{
  return(Sen1 <= Tensions[SPOOL_OFF].m1);
}

//-------------- Проверка падения натяжения T2 ниже минимума: ----------------

bool TSpool::LowT2(void)
{
  return(Sen2 <= Tensions[SPOOL_OFF].m2);
}

//----------- Проверка падения натяжения T1 или T2 ниже минимума: ------------

bool TSpool::LowT1T2(void)
{
  return((Sen1 <= Tensions[SPOOL_OFF].m1) || (Sen2 <= Tensions[SPOOL_OFF].m2));
}

//--------------------- Чтение параметров из EEPROM: -------------------------

void TSpool::EERead(void)
{
  K.p = Eeprom->Rd8(EE_KP, NOM_KP);
  K.i = Eeprom->Rd8(EE_KI, NOM_KI);
  K.d = Eeprom->Rd8(EE_KD, NOM_KD);
  SetPID(K);
  Tensions[SPOOL_OFF].m1   = Eeprom->Rd16(EE_TEN_MIN1,   NOM_TEN_MIN);
  Tensions[SPOOL_OFF].m2   = Eeprom->Rd16(EE_TEN_MIN2,   NOM_TEN_MIN);
  Tensions[SPOOL_BRAKE].m1 = Eeprom->Rd16(EE_TEN_BRAKE1, NOM_TEN_PLAY);
  Tensions[SPOOL_BRAKE].m2 = Eeprom->Rd16(EE_TEN_BRAKE2, NOM_TEN_PLAY);
  Tensions[SPOOL_PLAYF].m1 = Eeprom->Rd16(EE_TEN_PLAYF1, NOM_TEN_PLAY);
  Tensions[SPOOL_PLAYF].m2 = Eeprom->Rd16(EE_TEN_PLAYF2, NOM_TEN_PLAY);
  Tensions[SPOOL_PLAYR].m1 = Eeprom->Rd16(EE_TEN_PLAYR1, NOM_TEN_PLAY);
  Tensions[SPOOL_PLAYR].m2 = Eeprom->Rd16(EE_TEN_PLAYR2, NOM_TEN_PLAY);
  Tensions[SPOOL_FFD].m1   = Eeprom->Rd16(EE_TEN_FFD1,   NOM_TEN_PLAY);
  Tensions[SPOOL_FFD].m2   = Eeprom->Rd16(EE_TEN_FFD2,   NOM_TEN_LIMT);
  Tensions[SPOOL_REW].m1   = Eeprom->Rd16(EE_TEN_REW1,   NOM_TEN_LIMT);
  Tensions[SPOOL_REW].m2   = Eeprom->Rd16(EE_TEN_REW2,   NOM_TEN_PLAY);
  Tensions[SPOOL_AFFD].m1  = Eeprom->Rd16(EE_TEN_AFFD1,  NOM_TEN_ARSP);
  Tensions[SPOOL_AFFD].m2  = Eeprom->Rd16(EE_TEN_AFFD2,  NOM_TEN_ARTU);
  Tensions[SPOOL_AREW].m1  = Eeprom->Rd16(EE_TEN_AREW1,  NOM_TEN_ARTU);
  Tensions[SPOOL_AREW].m2  = Eeprom->Rd16(EE_TEN_AREW2,  NOM_TEN_ARSP);
}

//------------------- Сохранение параметров в EEPROM: ------------------------

void TSpool::EESave(void)
{
  Eeprom->Wr8(EE_KP, K.p);
  Eeprom->Wr8(EE_KI, K.i);
  Eeprom->Wr8(EE_KD, K.d);
  Eeprom->Wr16(EE_TEN_MIN1,   Tensions[SPOOL_OFF].m1);
  Eeprom->Wr16(EE_TEN_MIN2,   Tensions[SPOOL_OFF].m2);
  Eeprom->Wr16(EE_TEN_BRAKE1, Tensions[SPOOL_BRAKE].m1);
  Eeprom->Wr16(EE_TEN_BRAKE2, Tensions[SPOOL_BRAKE].m2);
  Eeprom->Wr16(EE_TEN_PLAYF1, Tensions[SPOOL_PLAYF].m1);
  Eeprom->Wr16(EE_TEN_PLAYF2, Tensions[SPOOL_PLAYF].m2);
  Eeprom->Wr16(EE_TEN_PLAYR1, Tensions[SPOOL_PLAYR].m1);
  Eeprom->Wr16(EE_TEN_PLAYR2, Tensions[SPOOL_PLAYR].m2);
  Eeprom->Wr16(EE_TEN_FFD1,   Tensions[SPOOL_FFD].m1);
  Eeprom->Wr16(EE_TEN_FFD2,   Tensions[SPOOL_FFD].m2);
  Eeprom->Wr16(EE_TEN_REW1,   Tensions[SPOOL_REW].m1);
  Eeprom->Wr16(EE_TEN_REW2,   Tensions[SPOOL_REW].m2);
  Eeprom->Wr16(EE_TEN_AFFD1,  Tensions[SPOOL_AFFD].m1);
  Eeprom->Wr16(EE_TEN_AFFD2,  Tensions[SPOOL_AFFD].m2);
  Eeprom->Wr16(EE_TEN_AREW1,  Tensions[SPOOL_AREW].m1);
  Eeprom->Wr16(EE_TEN_AREW2,  Tensions[SPOOL_AREW].m2);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
