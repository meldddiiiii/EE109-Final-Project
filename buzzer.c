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

#include "buzzer.h"
#include "project.h"

void buzzer_play(uint8_t ascending) {
    tone_pitch = ascending;
    tone_step = 1;
    tone_counter = 3; //3 ticks x 0.1s = 0.3s
    
    if (ascending) {
        OCR0A = 77; //High if ascending
    } else {
     CR0A = 19; // Low if descending   
    }

    timer0_start();
}