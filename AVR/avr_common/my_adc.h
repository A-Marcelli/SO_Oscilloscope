#include <avr/io.h>
#include <avr/interrupt.h>

#define APPROX

void adc_init(void);
void timer_init(void);
void adc_sel(uint8_t adc_number);
void freq_set(uint16_t frequency);
void adc_conv(uint8_t var);
void adc_conv_ground(void);
