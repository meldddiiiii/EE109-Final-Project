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
        OCR0A = 77; //Start LOW for ascending (low → mid → high)
    } else {
     OCR0A = 19; //Start HIGH for descending (high → mid → low)
    }

    timer0_start();

    // Make sure TIMER1 is ticking at 0.1 sec for buzzer bookkeeping
    timer1_init_stopwatch();
    timer1_start_stopwatch();
}