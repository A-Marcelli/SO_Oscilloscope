#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "./avr_common/my_uart.h"
#include "./avr_common/my_adc.h"

//interrupt
ISR(ADC_vect)
{
    //Interrupt che si attiva a conversione completata
}

ISR(USART0_RX_vect)
{
    //Interrupt che si attiva quando è stato ricevuto un byte
}

ISR(USART0_UDRE_vect)
{
    //Interrupt che si attiva quando il dato precendente è stato trasmesso, UDR0 vuoto
}

ISR(USART0_TX_vect)
{
    //Interrupt che si attiva quando è finita la trasmissione
}


//global variables
volatile uint8_t adc_number;
volatile uint8_t frequency;



int main(void){


    //Inizializzo le periferiche
    UART_init();  //inizializzazione uart
    adc_init();   //inizializzazione adc
    sei();        //interrupt abilitati



}