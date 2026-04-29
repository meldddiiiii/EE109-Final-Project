/********************************************
 *
 *  Name: Melodie Peng
 *  Email: mjpeng@usc.edu
 *  Section: Friday 11AM
 *  Assignment: Project
 *
 ********************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "project.h"

void usart_init(void) {
    DDRB |= (1<<PB4); //PB4 as output
    PORTB &= ~(1<<PB4); //Set PB4 low to enable buffer (???)

    UBR0H = 0; //Baud rate high value
    UBR0L = 103; //Baud rate low byte --> determined with 9600 baud

    UCSR0B |= (1<<RXEN0) | (1<<TXEN0); //Enable receiever + transmitter in B register (do we need RXCIE0 as well???)
    UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00); //One stop bit (i dont really understand this still)

}

void usart_send_speed(int16_t speed_mm) {
    char buf[8]; /// Longest string needed to format is 8 characters because we can only have up to four ASCII digits
    snprintf(buf, 8, "@%d$", speed_mm);

    int i = 0; // Loop counter for stepping through buf
    while (buf[i] != '\0') { // Keep going until we hit the null terminator (end of string)
        while ((UCSR0A & (1 << UDRE0)) == 0) { } // Wait until USART transmitter is ready (UDRE0 bit goes high)
        UDR0 = buf[i]; // Write the current character to the data register, hardware sends it
        i++; // Move to next character in buf
    }
}