#include <avr/io.h>

#include "adc.h"



void adc_init(void)
{
    // Initialize the ADC
    ADCSRA |= (1 << ADEN);
    ADMUX |= (1<<REFS0) | (1<<ADLAR);
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1) | (1<<ADPS0);
    
}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1<<ADSC);

    // Convert an analog input and return the 8-bit result
    while (ADCSRA & (1 << ADSC)); 
    return ADCH;   

}
