/********************************************
 *
 *  Name: Melodie Peng
 *  Email: mjpeng@usc.edu
 *  Section: Friday 11AM
 *  Assignment: Final Project 
 *
 ********************************************/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "adc.h"
#include "timers.h"
#include "encoder.h"
#include "project.h"
#include "buzzer.h"
#include "serial.h"

volatile char new_state, old_state, changed; //Rotary Encoder state machine

volatile uint16_t range_1; //Max 4000
volatile uint16_t range_2;
volatile uint16_t pulse_count; 
volatile uint8_t tenths; //max 100
volatile int32_t speed; //Speed can have negative value
volatile uint8_t threshold; // Rotary encoder values: 1-99
volatile uint8_t state;
volatile uint8_t tone_step; //0 = idle, 1/2/3 = which tone is playing
volatile uint8_t tone_counter; // Counts down 0.1s per tone
volatile uint8_t tone_pitch; //1 = pitch goes up, 0 = pitch goes down
volatile char rx_buf[6];        // buffer to collect characters as they arrive
volatile uint8_t rx_count;      // how many characters in buffer so far
volatile uint8_t rx_started;    // 1 if we've seen '@' and are collecting
volatile uint8_t rx_valid;      // 1 if we've successfully received a full message

int main(void) {

    //Initialize LCD display and ADC for reading LCD buttons and timer2 for servo
    lcd_init();
    adc_init();
    timer2_init_servo(); //Configure Timer2 for PWM output to servo motor (dial indicator)
    timer0_init();
    usart_init();

    DDRD |= (1<<3); //Set PD3 (trigger pin) as output (for servo)
    DDRC |= (1<<1) | (1<<2) | (1<<3); //Enable LEDS

    PCICR |= (1<<PCIE2); //Enable pin change interrupts for port D
    PCMSK2 |= (1<<PCINT18); //Enable interrupt specfically for PD2

    PCICR |= (1<<PCIE1);  //Enable pin change interrupts for port C
    PCMSK1 |= (1<<PCINT12) | (1<<PCINT13); //Enable on PC4, PC5

    PORTC |= (1 << PC4) | (1 << PC5);  // Enable pull-ups on encoder pins
    encoder_init(); //Read starting position of rotary encoder
    sei(); //Enable gloabl interrupts   

    // Write splash screen
	lcd_writecommand(1); 
	char name[13];
	char *first = "Melodie";
	char *last = "Peng";

	snprintf(name, 13, "%s %s", first, last);

	lcd_moveto(0,2);
	lcd_stringout(name);
	_delay_ms(1000);
	lcd_writecommand(1); 

    PORTC |= (1 << PC1) | (1 << PC2); // Turn red and green off
    PORTC &= ~(1 << PC3);              // Turn blue on  

    state = WAIT;
    
    threshold = eeprom_read_byte((void *) 10); //Use EEPROM to read threshold value
    if (!(threshold <100 && threshold >0)) { //Threshold value must be between 1-99, if out of bounds, reset threshold to 10
        threshold = 10;
    }

    OCR2A = 40;

    while(1) {

        if (state == WAIT) {
            // Left button: trigger first range measurement
            uint8_t adc_val = adc_sample(0);
            if (adc_val > 150 && adc_val < 162) {
                PORTD |= (1<<3); //Set trigger high
                _delay_us(10);
                PORTD &= ~(1<<3); //Set trigger low
                state = START;
            }
        }

        if (state == START_TO_RUN) {
            //Display Range 1 on LCD (top left)
            char buf[17];
            snprintf(buf, 17, "%d.%d", range_1 / 10, range_1 % 10);
            lcd_moveto(0, 0);
            lcd_stringout(buf);

            timer1_init_stopwatch();
            timer1_start_stopwatch();
            state = RUN;
        }

        if (state == RUN) {
            // Update elapsed time on LCD (top right)
            char time_str[17];
            snprintf(time_str, 17, "%d.%d", tenths / 10, tenths % 10);
            lcd_moveto(0, 12);
            lcd_stringout(time_str);

            // Right button: stop timer and trigger second range measurement
            uint8_t adc_val = adc_sample(0);
            if (adc_val < 5) {
                timer1_stop();
                PORTD |= (1<<3); //Set trigger high
                _delay_us(10);
                PORTD &= ~(1<<3); //Set trigger low
                state = STOP;
            }
        }

        if (state == RESULT) {

            if ((range_1 > 400) || (range_2 > 400)) { // Out of bounds
                PORTC |= (1 << PC1) | (1 << PC2); // Red and Green off
                PORTC &= ~(1 << PC3); //Blue on

                char bound[12]; //FIGURE THIS OUT 
	            char *first = "Out of";
	            char *last = "Bounds";

	            snprintf(bound, 13, "%s %s", first, last);

	            lcd_moveto(0,2);
	            lcd_stringout(bound);
                lcd_writecommand(1); 

                state = WAIT;

            } else {
                speed = ((int32_t)range_2 - (int32_t)range_1) * 10 / tenths; //Speed calculations

                //Display values on LCD
                lcd_writecommand(1);
                char buf[17];

                usart_send_speed((int16_t) speed); //Sends calculated speed to the other board

                // Range 1 (top left)
                snprintf(buf, 17, "%d.%d", range_1 / 10, range_1 % 10);
                lcd_moveto(0, 0);
                lcd_stringout(buf);

                // Range 2 (top middle)
                snprintf(buf, 17, "%d.%d", range_2 / 10, range_2 % 10);
                lcd_moveto(0, 8);
                lcd_stringout(buf);

                // Time (bottom right)
                snprintf(buf, 17, "%d.%d", tenths / 10, tenths % 10);
                lcd_moveto(1, 10);
                lcd_stringout(buf);

                // Speed (bottom left)
                if (speed < 0) {
                    int32_t temp = -speed;
                    snprintf(buf, 17, "-%ld.%ld", temp / 10, temp % 10);
                }
                else {
                    snprintf(buf, 17, "%ld.%ld", speed / 10, speed % 10);
                }
                
                lcd_moveto(1, 0);
                lcd_stringout(buf);

                // Turn all LEDs off first
                PORTC |= (1 << PC1) | (1 << PC2) | (1 << PC3);

                // Compare speed magnitude to threshold
                int32_t abs_speed = speed;
                if (abs_speed < 0) abs_speed = -abs_speed;

                if (abs_speed / 10 > threshold) {
                    PORTC &= ~(1 << PC1);  // Red on
                }
                else {
                    PORTC &= ~(1 << PC2);  // Green on
                }

                char thr_str[4];
                snprintf(thr_str, 4, "%2d", threshold);
                lcd_moveto(1, 7);
                lcd_stringout(thr_str);

                state = WAIT;
            }
        }

        }   

        if (changed) { // If encoder turned, clamp threshold to 1-99 and update LCD
            changed = 0;
            if (threshold > 99) threshold = 99;
            if (threshold < 1) threshold = 1;

            eeprom_update_byte((void *) 0, threshold); //EEPROM saves threshold value, don't have to reconfigure on start up

            char thr_str[4];
            snprintf(thr_str, 4, "%2d", threshold);
            lcd_moveto(1, 7);  // bottom middle
            lcd_stringout(thr_str);
    
        }

    }

