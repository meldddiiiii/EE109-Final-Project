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

#include "timers.h"
#include "project.h"

void timer1_stop(void) { //Turns timer off (all are set to 0)
    TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
}

void timer1_start_distance(void){ //Turns timer on with prescalar 8
    TCCR1B &= ~((1<<CS12) | (1<<CS10)); 
    TCCR1B |= (1<<CS11); //Setting CS11 alone turns prescalar 8 on

}

void timer1_start_stopwatch(void) { //Tracks time between two range measurements using prescalar 64
    TCCR1B &= ~(1<<CS12);
    TCCR1B |= ((1<<CS11) | (1<<CS10)); 

}

void timer1_init_distance(void) { // Configure TIMER1 to measure echo pulse width with timeout
    TCCR1B &= ~(1<<WGM12); //Normal mode
    TIMSK1 |= (1<<OCIE1A); //enables interrupt
    OCR1A = 46399; // Max range 400cm * 58 us/cm = 23200us, at prescalar 8 (2MHz) = 46400 counts - 1
    TCNT1 = 0; //Reset's counter to 0
}

void timer1_init_stopwatch(void) {
    TCCR1B |= (1<<WGM12); //CTC mode
    TIMSK1 |= (1<<OCIE1A); //enables interrupt
    OCR1A = 24999; // 16MHz / 64 prescaler = 250,000 counts/sec * 0.1s = 25,000 - 1
    TCNT1 = 0; //Reset's counter to 0
    tenths = 0; //Reset elapsed time to 0.0 seconds
    OCR2A = 35; //Reset servo to "0 seconds" position (fully left)
}

void timer2_init_servo(void){
    DDRB |= (1<<PB3); //PB3 output
    TCCR2A |= (1<<WGM21) | (1<<WGM20) | (1<<COM2A1); //Set Timer2 in Fast PWM mode and lets timer output PWM on OC2A pin
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20); //Set prescalar to 1024
    OCR2A = 35; //Start's servo at 0 second position
}

void timer0_init(void){
    DDRB |= (1<<PB5); //Enable PB5 as output
    TCCR0A |= (1<<WGM01); //Set Timer0 in CTC mode
    TIMSK0 |= (1<<OCIE0A); //Enable compare-match interrupt

}

void timer0_start(void) {
    TCCR0B &= ~((1<<CS01) | (1<<CS00)); // make sure CS01, CS00 are 0
    TCCR0B |= (1<<CS02); // set CS02 = 1 → prescaler 256
}

void timer0_stop(void) {
    TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
}

ISR(TIMER0_COMPA_vect) {
    PORTB ^= (1 << PB5);  // toggle PB5
}

ISR(TIMER1_COMPA_vect) { // ISR for when TCNT1 reaches OCR1A: handles timeout in both modes
    if (state == START || state == STOP) { //Echo doesn't comeback so cancels measurement
        timer1_stop();
        state = WAIT;
    }

    if (state == RUN) { // Stopwatch mode, count 0.1s intervals
        tenths++;
        OCR2A = 35 - (23 * tenths) / 100; // Update servo position
        if (tenths >= 100) { // 10 seconds passed, cancel measurement
            timer1_stop();
            PORTC |= (1<<PC1) | (1<<PC2);
            PORTC &= ~(1<<PC3);
            OCR2A = 35;        // reset servo to 0 position (unecessary but i wanna make it cleaner)
            state = WAIT;
        }
    }

    if (tone_step > 0) {
        tone_counter--;
        if (tone_counter == 0) {
            tone_step++;
            if (tone_step == 2) {
                OCR0A = 38;
                tone_counter = 3;
            } else if (tone_step == 3) {
                OCR0A = tone_pitch ? 19 : 77;
                tone_counter = 3;
            } else {                      //  tone_step is now 4
                timer0_stop();             //   buzzer silenced
                tone_step = 0;             //   back to idle
                timer1_stop();             //   TIMER1 no longer needed
            }
        }
    }


}

ISR(PCINT2_vect) { // ISR for echo pin (PD2): measures rangefinder pulse width to calculate distance
    if (PIND & (1 << PD2)) { // Rising edge, echo pulse started
        timer1_init_distance();
        timer1_start_distance();
    }
    else { //Falling edge, echo pulse ended
        timer1_stop();
        pulse_count = TCNT1;
        pulse_count = pulse_count * 5/58;

         if (state == START) {
            range_1 = pulse_count;
            state = START_TO_RUN;
         }
         
         else if (state == STOP) {
            range_2 = pulse_count;
            state = RESULT;
         }
    }
}