#include "my_adc.h"

void adc_init(void){
	//inizializza adc qui
	//da inizializzare: ADMUX, ADCSRA, ADCSRB,DIDR0, DIDR2

	ADMUX |= (1 << REFS0) //da modificare in seguito per decidere quale/i adc modificare. AVCC da collegare hardware!

	ADCSRA |= (1 << ADEN) | (1 << ADIE) //mancano da impostare le impostazioni da richiedere al pc(trigger mode, partenza e prescaler)

	//gli altri andranno settati dopo aver ricevuto le impostazioni dal pc
}