/*
* Baliono_datchikas_1.cpp
*
* Created: 2014.11.30 19:00:04
*  Author: Kieran

Spargalkes skyrelis:

DBG UART - nereikia outputo
Masyvo spausdinimas is flasho (auto 0x0A, pridejau bibliotekoj): dbg_print_array(testt, sizeof(testt));
Masyvo irasymas i flash: const char testt[] PROGMEM = {0x55, 0x56};
HW uart spausdinimas (0x0A pridejau bibliotekoj): uart_puts("Veikia?????");
Duomenu priemimas is hw uarto: rec_data = uart_getc();
*/

#define FW_VER "0.1"

#define F_CPU 1000000UL // CPU clock define (CPU should know what frequency is he on)
#define UART_BAUD_RATE 4800 //38400 // HW uart BAud rate defined
#define DEBUG_MODE // Debug Put Char ON/OFF

// ****** SW UART START *******
//#define DBG_UART_TX_PORT        PORTB
//#define DBG_UART_TX_DDR         DDRB
//#define DBG_UART_TX_PIN         PB0 // PB0 OUTPUT
// SW UART END

#define new_line uart_puts("\r\n")
#define CRLF "\r\n" //String
#define LF 0x0A // \n (new line) 0x0A
#define CR 0x0D // \r (Car. return) 0x0D

//*** MISQ ****
#define off(a,z) (a &= ~_BV(z))
#define on(a,z) (a |= _BV(z))
#define change(a,z) (a ^=_BV(z))
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))

// Timing

// Include start

//****Standart*****
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> // Flash masyvams
#include <avr/sleep.h>
#include <ADC.h>
#include <CONFIG.h>

//*****SW UART*****
#ifdef DEBUG_MODE //DBG_PUT CHAR
#include <dbg_putchar.h>
#endif

//****HW UART****

#include "uart.h"

//const char testt[] PROGMEM = {0x4F, 0x4B};
const char my_number[] PROGMEM = "+37061217788";

volatile char mov_det=0;
volatile char TIME_OUT_COUNT=0;

// Prototypes

char wait_for_movement(unsigned char time_out_val);
void wdt_delay(int miliseconds);

int main(void)
{	
	_delay_ms(100); // When woken up from sleep.
	
	// WDT init.
	wdt_enable(WDTO_8S); // Let's try 8s wdt.
	#warning Clearing the watchdog reset flag before disabling the watchdog is required, according to the datasheet.
	#warning wdt_reset needs cli instructions before execution.
	wdt_reset();	
	
	#ifdef DEBUG_MODE
	if(bit_is_set(MCUSR, WDRF))
	{	
		dbg_puts("Recovery after WDT.\r\n");
		//This bit is set if a Watchdog System Reset occurs.
		off(MCUSR,WDRF); //The bit is reset by a Power-on Reset, or by writing a logic zero to the flag 
	}
	#endif
	
	//***** INIT
	#ifdef DEBUG_MODE	// Debug Put Char init
	dbg_tx_init();
	dbg_puts("Debug Put Char up and working!\r\n");
	#endif
	
	//uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); // HW uart init, KAM uartas dabar?
	
	#ifdef DEBUG_MODE
	sei();
	dbg_uart_puts("Main Uart initialized!\r\n");
	cli();
	#endif
	
	_delay_ms(10);
	
	// PINSETUP:
	#ifdef DEBUG_MODE
	dbg_puts("PIN init started.\r\n");
	#endif		
	
	//pin_cfg(); // Kam pin initas dabar.	
		
	// Sleep init.		
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	#ifdef DEBUG_MODE
	dbg_puts("Sleep mode set.\r\n");
	#endif

	timer0_init();
	int_init();

	while (1)
	{			
		#ifdef DEBUG_MODE
			unsigned int f1;			
			char str[16];
			f1 = get_vbat_voltage();
			itoa(f1, str, 10);
			dbg_puts(str);
		#endif	
			
			//dbg_puti(get_vbat_voltage());
			
			_delay_ms(100);			
			
			
			if(wait_for_movement(10))
			{
				
				EIMSK &= ~(1 << INT0);
				
				//PORTB |= (1<<PB0);
				_delay_ms(2000);
				
				EIMSK |= (1<<INT0);
				
			}
			
			
			//PORTB &= ~(1 << PB0);
			
		
			sleep_enable();
			sleep_cpu();			
			sleep_disable();
			_delay_ms(100); // When woken-up from sleep.
					
	}
}


ISR(INT0_vect)
{
	// Vibration sensor.
	mov_det = 1;
}


ISR(INT1_vect)
{
	// Ring indication.
	// ATA
}


ISR(TIMER0_OVF_vect){	if(TIME_OUT_COUNT>=254)	{		TIME_OUT_COUNT = 0;	}	TIME_OUT_COUNT++;}void wdt_delay(int miliseconds)
{
	
	int delay_step = 200;
	int step_count = miliseconds / delay_step ;
	
	dbg_uart_puts("Delaying\r\n");
	
	for (int i = 0; i< step_count ;i++)
	{
		wdt_reset();
		_delay_ms(delay_step);
		wdt_reset();
		
	}
	
}

char wait_for_movement(unsigned char time_out_val){	char status = 0;	TIME_OUT_COUNT = 0;	TCNT0 = 0; // reset timer.		while(TIME_OUT_COUNT < time_out_val)	{		if(mov_det == 1)		{			TIME_OUT_COUNT = 0;			mov_det = 0;			status = 1;		}	}		return status;}