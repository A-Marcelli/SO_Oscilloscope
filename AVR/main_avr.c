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
//#define max_conv 20             //prova, da spostare

//global variables
volatile uint8_t byte_rec = 0;   //1=ricevuto
volatile uint8_t byte_tra = 0;   //1=transmesso
//volatile uint8_t conv_fin = 0;   //1=conversione adc finita
volatile uint8_t ocr_int  = 0;   //1=ocr interrupt

volatile uint8_t buffer_rx;      //variabile di appoggio
volatile uint8_t buffer_tx[2];   //variabile di appoggio
uint8_t *buffer;
volatile uint32_t num_int  = 0;    //numero di interrupt del timer5 accaduti, utilizzato per attivare la condizione di stop
volatile uint32_t num_conv = 0;    //numero di conversioni portate a termine, utilizzato nella buffered mode per riempire il vettore
volatile uint32_t len               = 0;
uint32_t max_conv          = 0;
volatile uint32_t prova = 0;

volatile uint8_t adc_number;     //numero di canali adc da utilizzare
volatile uint8_t frequency;      //numero di ogni quanti ms si effettuerà un sampling
volatile uint8_t mode;           //1 = continuous sampling, 2 = buffered mode
//volatile uint8_t trigger;        //1 = inizia (??)

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
                                   //111 = fine programma

//interrupts

//ISR(ADC_vect)
//{
//    //Interrupt che si attiva a conversione completata
//    conv_fin = 1;
//}

ISR(TIMER5_COMPA_vect) 
{
    //Interrupt che si attiva quando il timer matcha l'OCR
    ocr_int = 1;
    num_int++;

}

ISR(USART0_RX_vect)
{
    //Interrupt che si attiva quando è stato ricevuto un byte
    buffer_rx = UDR0;
    byte_rec = 1;
}

//ISR(USART0_UDRE_vect)
//{
//    //Interrupt che si attiva quando il dato precendente è stato trasmesso, UDR0 vuoto
//}

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
            adc_number = buffer_rx;
            adc_sel(adc_number);     //setto gli adc selezionati

            byte_rec = 0;
            state = 2;
        }
        break;

    case 2:
        if(byte_rec == 1){
            frequency = buffer_rx;
            freq_set(frequency);     //setto la frequenza selezionata, max 255 ms cosi!!!!!!! (nel programma pc controlla sia frequency >0!)

            max_conv = (uint32_t) (STOP * 1000) / frequency;
            len = max_conv * 2 * adc_number;
            byte_rec = 0;
            state = 3;
        }
        break;

    case 3:
        if(byte_rec == 1){
            mode = buffer_rx;
            if(mode == 2){
                buffer = (uint8_t *) malloc(sizeof(uint8_t) * len);
            }

            byte_rec = 0;
            state = 4;
        }
        break;

    case 4:
        if(byte_rec == 1){
            //trigger ricevuto
            byte_rec = 0;
            byte_tra = 1;           //prima volta inizializzato a mano

            cli();
            TCNT5 = 0;
            TIMSK5 |= (1 << OCIE5A); // enable the timer interrupt
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
        //if((uint16_t) num_int*frequency >= (uint16_t) STOP*1000){   //se sono passati STOP secondi, stop conversioni e fine programma
        if(num_conv == max_conv){
            cli();
            state = 7;
            break;
        }

        if(ocr_int == 1){
            //conversione e invio
            for(uint8_t var = 0; var < adc_number;var++){
                //conversione
                adc_conv(var);

                //invio
                UART_putChar(buffer_tx[0]);
                UART_putChar(buffer_tx[1]);
                //UART_putChar(prova);   //prova
                //prova++;
                //UART_putChar(prova);   //prova
                //prova++;

            }
            num_conv++;
            ocr_int = 0;
        }


        break;

    case 6:
        /* buffered mode */
        if(num_conv == max_conv){
            //invia tutto e azzera num_conv
            UART_putString(buffer);
            cli();
            state = 7;
            break;
            //num_conv = 0;
        }

        //if((uint16_t) num_int*frequency >= (uint16_t) STOP*1000){   //se sono passati STOP secondi, stop conversioni e fine programma
        //    //INVIA QUELLO CHE è RIMASTO NEL BUFFER!!
        //    //UART_putChar((uint8_t ) num_conv);
        //    cli();
        //    state = 7;
        //    break;
        //}

        if(ocr_int == 1){
            //converti e salva
            
            for(uint8_t var = 0; var < adc_number;var++){
                //conversione
                adc_conv(var);

                //storage
                buffer[num_conv*2 + 0 + max_conv*2*var] = buffer_tx[0];  //prima low
                buffer[num_conv*2 + 1 + max_conv*2*var] = buffer_tx[1];  //poi high
                //buffer[num_conv*2 + 0 + max_conv*2*var] = (uint8_t ) prova;  //PROVA
                //prova++;
                //buffer[num_conv*2 + 1 + max_conv*2*var] = (uint8_t ) 1;  //PROVA
                //prova++;
                               
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