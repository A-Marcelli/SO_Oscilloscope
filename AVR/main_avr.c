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
    buffer = UDR0;
    byte_rec = 1;
}

ISR(USART0_UDRE_vect)
{
    //Interrupt che si attiva quando il dato precendente è stato trasmesso, UDR0 vuoto
}

ISR(USART0_TX_vect)
{
    //Interrupt che si attiva quando è finita la trasmissione
}

//funzioni
void state_machine(void);



//global variables
volatile uint8_t byte_rec = 0;   //1=ricevuto
volatile uint8_t byte_tra = 0;   //1=transmesso
volatile uint8_t buffer          //variabile di appoggio

volatile uint8_t adc_number;
volatile uint8_t frequency;
volatile uint8_t mode;           //1 = continuous sampling, 2 = buffered mode
volatile uint8_t trigger;        //1 = inizia (??)

volatile uint8_t received_setting; //variabile per tenere conto delle impostazioni ricevute:
                                   // bit 0: se numero adc ricevuto -> =1
                                   // bit 1: se frequenza ricevuta -> =1
                                   // bit 2: se modalità di operazione ricevuta -> =1

volatile uint8_t state;            //contiene lo stato in cui mi trovo:
                                   //000 = inizializzazione
                                   //001 = aspettando numero dispositivi
                                   //010 = aspettando frequenza
                                   //011 = aspettando modalità di esecuzione
                                   //100 = se buffered mode, aspettando trigger
                                   //101 = esecuzione continous sampling
                                   //110 = esecuzione buffered mode

int main(void){


    //Inizializzo le periferiche
    UART_init();  //inizializzazione uart
    adc_init();   //inizializzazione adc
    sei();        //interrupt abilitati

    state = 0;
    while(1){

        state_machine();

    }

}


void state_machine(void){
    switch (state)
    {
    case 0:
        state = 1;
        break;

    case 1:
        if(byte_rec == 1){
            adc_number = buffer;
            adc_sel(adc_number); //setto gli adc selezionati
            //da finire
            byte_rec = 0;
            state = 2;
        }
        break;

    case 2:
        if(byte_rec == 1){
            frequency = buffer;
            freq_set(frequency); //setto la frequenza selezionata
            //da finire
            byte_rec = 0;
            state = 3;
        }
        break;

    case 3:
        if(byte_rec == 1){
            mode = buffer;
            mode_set(mode); //setto la modalità selezionata
            //da finire
            byte_rec = 0;
            state = 4;
        }
        break;

    case 4:
        if(byte_rec == 1){
            trigger = buffer;
            //da finire
            byte_rec = 0;
            //if(trigger != 1) break;
            if(mode == 1){
                state = 5;
            } else if(mode == 2){
                state = 6
            }
        }
        break;

    case 5:
        /* code */
        break;

    case 6:
        /* code */
        break;

    default:
        break;
    }




}