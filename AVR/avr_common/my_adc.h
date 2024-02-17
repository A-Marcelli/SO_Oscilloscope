#include <avr/io.h>
#include <avr/interrupt.h>


void adc_init(void);
void adc_sel(uint8_t adc_number);
void freq_set(uint8_t frequency);
void mode_set(uint8_t mode);