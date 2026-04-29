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
#include <stdio.h>

#include "serial.h"
#include "project.h"

volatile char rx_buf[6];
volatile uint8_t rx_started;
volatile uint8_t rx_count;
volatile uint8_t rx_valid;

void usart_init(void) {
    DDRB |= (1<<PB4); //PB4 as output
    PORTB &= ~(1<<PB4); //Set PB4 low to enable buffer (???)

    UBRR0H = 0; //Baud rate high value
    UBRR0L = 103; //Baud rate low byte --> determined with 9600 baud

    UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0); //Enable receiever + transmitter in B register (do we need RXCIE0 as well???)
    UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00); // 8 data bits per character

}


void usart_send_speed(int16_t speed_mm) {
    char buf[8]; // Longest string needed to format is 8 characters because we can only have up to four ASCII digits
    snprintf(buf, 8, "@%d$", speed_mm);

    int i = 0; // Loop counter for stepping through buf
    while (buf[i] != '\0') { // Keep going until we hit the null terminator (end of string)
        while ((UCSR0A & (1 << UDRE0)) == 0) { } // Wait until USART transmitter is ready (UDRE0 bit goes high)
        UDR0 = buf[i]; // Write the current character to the data register, hardware sends it
        i++; // Move to next character in buf
    }
}

ISR(USART_RX_vect) {

    char c = UDR0; //Read charater from UDR0

    if(c == '@') { //Start of message
        rx_started =1;
        rx_count=0;
        rx_valid =0;
    } else if ((c == '$') && (rx_started == 1) && (rx_count>0)) { //End of valid message
        rx_buf[rx_count] = '\0'; //Null-terminate so sscanf can read it
        rx_valid =1;
        rx_started =0;
    } else if (rx_started) { //Inside a message
        if ((c >= '0' && c <= '9') || (c == '-')) {
            if (rx_count <5) {
            rx_buf[rx_count] = c; 
            rx_count++;
            } else {
                rx_started = 0; //Buffer overflow, so discard
            }
        }
      else { //NOthing so discard
        rx_started = 0;
      }  

    }
     
}