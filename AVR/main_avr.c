#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "./avr_common/my_uart.h"
#include "./avr_common/my_adc.h"


ISR(ADC_vect)
{
    //Interrupt che si attiva a conversione completata
}



int main(void){


    //Inizializzo le periferiche
    UART_init();  //inizializzazione uart
    adc_init();   //inizializzazione adc
    sei();        //interrupt abilitati

}