//----------------------------------------------------------------------------

//Модуль порта с протоколом Wake, заголовочный файл

//----------------------------------------------------------------------------

#ifndef WAKEPORT_HPP
#define WAKEPORT_HPP

#include "wake.hpp"

//----------------------------------------------------------------------------
//--------------------------- Класс TWakePort --------------------------------
//----------------------------------------------------------------------------

#define Rx_Int USART0_RX_vect
#define Tx_Int USART0_TX_vect
extern "C" void Rx_Int(void) __attribute__((signal));
extern "C" void Tx_Int(void) __attribute__((signal));

class TWakePort : public TWake
{
private:
  Pin_RXD0_t Pin_RXD0;
  Pin_TXD0_t Pin_TXD0;
  
  static uint16_t const BAUD_RATE = 19200; //скорость обмена, бод
  
  static TWakePort *Wp;
  friend void ::Rx_Int(void);
  friend void ::Tx_Int(void);
protected:
public:
  TWakePort();
  void Start(void);
  void StartTx(uint8_t cmd);
};

//----------------------------------------------------------------------------

#endif
