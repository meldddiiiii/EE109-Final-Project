#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void usart_init(void);
void usart_send_speed(int16_t speed_mm);

#endif