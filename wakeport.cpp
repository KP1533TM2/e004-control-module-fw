//----------------------------------------------------------------------------

//Модуль порта с протоколом Wake

//----------------------------------------------------------------------------

//Класс TWakePort наследуется от класса TWake, в котором реализован протокол
//Wake. В классе TWakePort осуществляется привязка этого протокола к
//физическому порту, в данном случае, USART0 микроконтроллера ATmega64.
//Никаких конкретных команд этот класс не реализует.

//----------------------------------------------------------------------------

#include "main.hpp"
#include "wakeport.hpp"

//----------------------------------------------------------------------------
//--------------------------- Класс TWakePort --------------------------------
//----------------------------------------------------------------------------

//----------------------------- Конструктор: ---------------------------------

TWakePort::TWakePort(const uint16_t br, const uint8_t frame) : TWake(frame)
{
  TWakePort::Wp = this;
  //настройка портов:
  Pin_RXD0.DirIn(PIN_PULL);
  Pin_TXD0.DirOut(PIN_HIGH);
  //настройка USART0:
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  //8-битная посылка
  //задание скорости обмена:
  UBRR0L = LO(br);
  UBRR0H = HI(br);
  //разрешение прерываний по приему и передаче, разрешение приема и передачи:
  UCSR0B = (1 << RXCIE0) | (1 << TXCIE0) | (1 << RXEN0) | (1 << TXEN0);
}

//--------------------- Прерывание после приема байта: -----------------------

TWakePort* TWakePort::Wp;

#pragma vector = USART0_RXC_vect
__interrupt void Rx_Int(void)
{
  TWakePort::Wp->Rx(UDR0);
}

//-------------------- Прерывание после передачи байта: ----------------------

#pragma vector = USART0_TXC_vect
__interrupt void Tx_Int(void)
{
  uint8_t data;
  if(TWakePort::Wp->Tx(data)) UDR0 = data;
}

//--------------------------- Передача пакета: -------------------------------

void TWakePort::StartTx(uint8_t cmd)
{
  uint8_t data;
  TxStart(cmd, data);
  UDR0 = data;
}

//----------------------------------------------------------------------------
