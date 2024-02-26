#include "my_adc.h"
#include "my_variables.h"

void adc_init(void){
	//inizializzo adc:

#ifdef APPROX
	ADMUX |= (1 << REFS0) | (1 << REFS1) | (1<<ADLAR);   //left adjusted data, leggerò solo gli 8 bit più significativi del risultato (su 10 disponibili), contenuti in ADCH
#else
	ADMUX |= (1 << REFS0) | (1 << REFS1);                //da modificare in seguito per decidere quale/i adc usare.
#endif
	ADCSRA |= (1 << ADEN);
}

void timer_init(void){
	//inizializzo timer: prescaler 256x, CTC
	TCCR5A = 0;
	TCCR5B = (1 << WGM52) | (1 << CS52);
}

void adc_sel(uint8_t adc_number){

	//disattivo gli input digitali (disattivo il buffer dell'input digitale)
	uint8_t i = 0;
	for(i=0;i<adc_number;i++){
		DIDR0 = (DIDR0<<1) | 0x1;
	} 
}

void freq_set(uint16_t frequency){
	//imposta la frequenza selezionata
	uint16_t ocrval=(uint16_t)(6.25*frequency);  //ogni quanti decimi di ms deve essere effettuato un sampling
	OCR5A = ocrval;
}

void adc_conv(uint8_t var){

	ADMUX = (ADMUX & 0xe0) | (var & 0x07);  //seleziono il canale da cui leggere
	ADCSRA |= (1<<ADSC);                    //faccio partire la conversione

	while(ADCSRA & (1<<ADSC));              //aspetta che finisca la conversione

	ADCSRA |= (1<<ADSC);                    //faccio partire la seconda conversione, necessaria perchè una misura sola non basta per caricare il condensatore
											//completamente. con una sola conversione raggiunge massimo valori di 1.2V
	while(ADCSRA & (1<<ADSC));              //aspetta che finisca la conversione

#ifdef APPROX
	buffer_tx[0] = ADCH;
#else
	buffer_tx[0] = ADCL;                     //va letta prima questo registro
	buffer_tx[1] = ADCH;
#endif
}

void adc_conv_ground(void){
	ADMUX = (ADMUX & 0xe0) | 0x1f;           //seleziono il canale di ground
	ADCSRA |= (1<<ADSC);                     //faccio partire la conversione

	while(ADCSRA & (1<<ADSC)); 
}