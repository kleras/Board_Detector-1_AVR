/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42, by Joerg Wunsch):
 * <dinuxbg .at. gmail.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Dimitar Dimitrov
 * ----------------------------------------------------------------------------
 */

#ifndef DBG_PUTCHAR_H
#define DBG_PUTCHAR_H

#include <stdint.h>

/* User setting -> Whether to enable the software UART */
#define DBG_UART_ENABLE         1

/* User setting -> Output pin the software UART */
#define DBG_UART_TX_PORT        PORTB
#define DBG_UART_TX_DDR         DDRB
#define DBG_UART_TX_PIN         PB2


/* User setting -> Software UART baudrate. */
#define DBG_UART_BAUDRATE       4800


/* ---- DO NOT TOUCH BELOW THIS LINE ---- */

#if DBG_UART_ENABLE

/**
 * @brief Debug software UART initialization.
 */
#define dbg_tx_init()   do { \
                DBG_UART_TX_PORT |= (1<<DBG_UART_TX_PIN); \
                DBG_UART_TX_DDR |= (1<<DBG_UART_TX_PIN); \
        } while(0)

/**
 * @brief Software UART routine for transmitting debug information. 
 *
 * @warning This routine blocks all interrupts until all 10 bits  ( 1 START +
 * 8 DATA + 1 STOP) are transmitted. This would be about 1ms with 9600bps.
 *
 * @note Calculation for the number of CPU cycles, executed for one bit time:
 * F_CPU/BAUDRATE = (3*1+2) + (2*1 + (NDLY-1)*4 + 2+1) + 6*1
 *
 * (NDLY-1)*4 = F_CPU/BAUDRATE - 16
 * NDLY = (F_CPU/BAUDRATE-16)/4+1
 */
extern void dbg_putchar(uint8_t c);
//New Line Add START
extern void dbg_print_array(const char* arr, char size);
extern void dbg_print_char(const char n);
extern void dbg_puts(const char *s );
//New Line END

#else
  #define dbg_tx_init()         
  #define dbg_putchar(A)                ((void)(A))
#endif  /* DBG_UART_ENABLE */

#endif  /* DBG_PUTCHAR_H */