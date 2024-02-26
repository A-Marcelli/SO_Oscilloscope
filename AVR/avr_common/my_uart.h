#pragma once
#include <stdint.h>
#include <avr/io.h>

//#define BAUD 19200
#define BAUD 38400
#define MYUBRR (F_CPU/16/BAUD-1)

//! @brief initializes the uart peripheral
void UART_init(void);

//! @brief sends a char on the peripheral
void UART_putChar(uint8_t c);

//! @brief high level function (exposed)
//!        writes a string until the first newline or 0
void UART_putString(uint8_t* buf);
