#ifndef PROJECT_H
#define PROJECT_H

#include <stdint.h>

enum states {WAIT, START, START_TO_RUN, RUN, STOP, RESULT};

extern volatile uint8_t state;
extern volatile uint8_t tenths;
extern volatile uint16_t pulse_count;
extern volatile uint16_t range_1;
extern volatile uint16_t range_2;
extern volatile int32_t speed;
extern volatile uint8_t threshold;
extern volatile char new_state, old_state, changed;

extern volatile char rx_buf[6];
extern volatile uint8_t rx_valid;
extern volatile uint8_t rx_started;
extern volatile uint8_t rx_count;
#endif