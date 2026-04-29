#include <avr/io.h>
#include <avr/interrupt.h>
#include "encoder.h"
#include "project.h"

void encoder_init(void){ // Initialize rotary encoder by reading starting position of knob
    char bits = PINC; //Reads PINC
    char a = bits & (1<<PC4); //Determines whether PC4 is high or low
    char b = bits & (1<<PC5); //Determines whether PC5 is high or low

    if (!a && !b) old_state = 0;
    else if (a && !b) old_state = 1;
    else if (!a && b) old_state = 2;
    else old_state = 3;

    new_state = old_state; // Start both in sync
}

ISR(PCINT1_vect) {
    char bits = PINC;
    char a = bits & (1 << PC4);
    char b = bits & (1 << PC5);

    if (old_state == 0) {
        if (a) {
            new_state = 1;
            threshold++;
        }

        else if (b) {
            new_state = 2;
            threshold--;
        }
    }

    if (old_state == 1) {
        if (!a) {
            new_state = 0;
            threshold--;
        }
        else if (b) {
            new_state= 3;
            threshold++;
        }
    }
    
    if (old_state == 2) {
        if (a) {
            new_state = 3;
            threshold--;
        }
        else if (!b) {
            new_state = 0;
            threshold++;
        }
    }

     if (old_state == 3) {
        if (!a) {
            new_state = 2;
            threshold++;
        }
        else if (!b) {
            new_state = 1;
            threshold--;
        }
    }

    if (new_state != old_state) {
        changed = 1;
        old_state = new_state;
    }
}