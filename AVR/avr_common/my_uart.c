#include "my_uart.h"
#include "my_variables.h"

void UART_init(void) {
  // Set baud rate
  UBRR0H = (uint8_t)(MYUBRR>>8);
  UBRR0L = (uint8_t)MYUBRR;

  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8-bit data */ 
  UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);   /* Enable RX and TX */  

}

void UART_putChar(uint8_t c){
  // wait for transmission completed
  while ( ! byte_tra);
  // Start transmission
  UDR0 = c;
  byte_tra = 0;
}

void UART_putString(uint8_t* buf){
  for(uint32_t num = 0;num<len;num++){
    UART_putChar(*buf);
    ++buf;
  }
}
