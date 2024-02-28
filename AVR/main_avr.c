#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "./avr_common/my_uart.h"
#include "./avr_common/my_adc.h"
#include "./avr_common/my_variables.h"

//macro
#define STOP 10
//La macro APPROX è definita in adc.h
//La macro BAUD   è definita in uart.h

//global variables
volatile uint8_t byte_rec = 0;
volatile uint8_t byte_tra = 0;     //1=transmesso
volatile uint8_t ocr_int  = 0;     //1=ocr interrupt

volatile uint8_t buffer_rx[4];     //variabile di appoggio
volatile uint8_t buffer_tx[2];     //variabile di appoggio
uint8_t *buffer;
uint32_t num_conv        = 0;      //numero di conversioni portate a termine, utilizzato nella buffered mode per riempire il vettore
uint32_t len             = 0;
uint32_t max_conv        = 0;

uint8_t adc_number;                //numero di canali adc da utilizzare
uint16_t period;                //numero di ogni quanti decimi di ms si effettuerà un sampling
uint8_t mode;                      //1 = continuous sampling, 2 = buffered mode

uint8_t state;                     //contiene lo stato in cui mi trovo:
                                   //000 = inizializzazione
                                   //001 = aspettando numero dispositivi
                                   //010 = aspettando modalità di esecuzione
                                   //011 = aspettando period
                                   //100 = aspettando trigger
                                   //101 = esecuzione continous sampling
                                   //110 = esecuzione buffered mode
                                   //111 = fine programma

//interrupts

ISR(TIMER5_COMPA_vect) 
{
    //Interrupt che si attiva quando il timer matcha l'OCR
    ocr_int = 1;

}

ISR(USART0_RX_vect)
{
    //Interrupt che si attiva quando è stato ricevuto un byte
    buffer_rx[byte_rec++] = UDR0;
}

ISR(USART0_TX_vect)
{
    //Interrupt che si attiva quando è finita la trasmissione
    byte_tra = 1;
}

//funzioni
void state_machine(void);


int main(void){


    //Inizializzo le periferiche
    UART_init();       //inizializzazione uart  -> funzione nel file my_uart.c
    adc_init();        //inizializzazione adc   -> funzione nel file my_adc.c
    timer_init();      //inizializzazione timer -> funzione nel file my_adc.c
    sei();             //interrupt abilitati

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
            adc_number = buffer_rx[0];
            adc_sel(adc_number);     //setto gli adc selezionati

            state = 2;
        }
        break;
    case 2:
        if(byte_rec == 2){
            mode = buffer_rx[1];

            state = 3;
        }
        break;
    case 3:
        if(byte_rec == 4){
            period |= ((uint16_t) buffer_rx[2])<<8;
            period |= ((uint16_t) buffer_rx[3]);

            period_set(period);     //setto il periodo selezionato

            max_conv = (uint32_t) ((uint32_t) STOP * 10000) / period;
        #ifdef APPROX
            len = max_conv * adc_number;
        #else
            len = max_conv * 2 * adc_number;
        #endif

            if(mode == 2){
                buffer = (uint8_t *) malloc(sizeof(uint8_t) * len);
            }

            state = 4;
        }
        break;

    case 4:
        if(byte_rec == 5){
            //trigger ricevuto
            byte_rec = 0;
            byte_tra = 1;             //prima volta inizializzato a mano

            cli();
            TCNT5 = 0;
            TIMSK5 |= (1 << OCIE5A);  // enable the timer interrupt
            sei();

            if(mode == 1){
                state = 5;
            } else if(mode == 2){
                state = 6;
            }
        }
        break;

    case 5:
        /* continous sampling */
        if(num_conv == max_conv){
            cli();
            state = 7;
            break;
        }

        if(ocr_int == 1){
            //conversione e invio
            for(uint8_t var = 0; var < adc_number;var++){
                //conversione
                adc_conv_ground();   //necessario per scaricare il condensatore prima di una misura, sennò non fa in tempo a scaricarsi e le misure vengono errate
                adc_conv(var);

                //invio
            #ifdef APPROX
                UART_putChar(buffer_tx[0]);
            #else
                UART_putChar(buffer_tx[0]);
                UART_putChar(buffer_tx[1]);
            #endif
            }
            num_conv++;
            ocr_int = 0;
        }


        break;

    case 6:
        /* buffered mode */
        if(num_conv == max_conv){
            //invia tutto
            UART_putString(buffer);
            cli();
            free(buffer);
            state = 7;
            break;
        }

        if(ocr_int == 1){
            //converti e salva
            
            for(uint8_t var = 0; var < adc_number;var++){
                //conversione
                adc_conv_ground();      //necessario per scaricare il condensatore prima di una misura, sennò non fa in tempo a scaricarsi e le misure vengono errate
                adc_conv(var);

                //storage
            #ifdef APPROX
                buffer[num_conv + max_conv*var] = buffer_tx[0];
            #else
                buffer[num_conv*2 + 0 + max_conv*2*var] = buffer_tx[0];  //prima low
                buffer[num_conv*2 + 1 + max_conv*2*var] = buffer_tx[1];  //poi high
            #endif              
            }
            num_conv++;
            ocr_int = 0;
        }

        break;

    case 7:
        /* fine */
        break;

    default:
        break;
    }
}