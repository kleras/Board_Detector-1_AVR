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

#define F_CPU 8000000UL // CPU clock define (CPU should know what frequency is he on)
#define MHz F_CPU/1000000
#define UART_BAUD_RATE 38400 //38400 // HW uart BAud rate defined
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

#define MOVEMENT_TIMEOUT_MIN 1 // Timout movement delays sms, MAX 15min // settings 5 5 5 
#define LEGIT_MOVEMENT_TIMEOUT_S 5

#define LEGIT_MOVEMENT_COUNT 5

// SMS modes

#define SMS_MODE_FIRST_FILTER_PASS 0
#define SMS_MODE_MOVEMENT_FINISHED 1
#define SMS_MODE_NOT_SUFFICIENT_FIRST_FILTER 2 



// Include start

//****Standart*****
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> // Flash masyvams
#include <avr/sleep.h>
#include <ADC.h>
#include <CONFIG.h>
#include <avr/wdt.h>
#include <sim900.h>


//*****SW UART*****
#ifdef DEBUG_MODE //DBG_PUT CHAR
#include <dbg_putchar.h>
#endif

//****HW UART****

#include "uart.h"

// ****** Globals ******

volatile unsigned char MOVEMENT_DETECTED = 0;
volatile unsigned int TIME_OUT_COUNT = 0;
volatile unsigned char RING_DETECTED = 0;

const uint8_t number[] = "+37061217788";
//const uint8_t number[] = "+37062907663";
//const uint8_t number[] = "+37068727799";

//const uint8_t sms[] = "SMS test...\r\n";


// Prototypes

//void wdt_delay(int miliseconds);

uint8_t send_sms_template(uint8_t sms_mode);
uint8_t check_if_movement_is_legit(unsigned int legit_movement_count, unsigned int time_out_val);
uint8_t wait_for_movement_to_finish(unsigned int time_out_val);


int main(void)
{		
		
		
		
		// Tunr on modem if not yet turned on.
		on(RING_INT1_PORT_R,RING_INT1_PIN);
		
		if(bit_is_clear(RING_INT1_PIN_R,RING_INT1_PIN))
		{
			modem_power_on();
		}	
		
		
		/*	
		// WDT init.
		wdt_enable(WDTO_1S); // Let's try 8s wdt.
		sei();
		#warning Clearing the watchdog reset flag before disabling the watchdog is required, according to the datasheet.
		#warning wdt_reset needs cli instructions before execution.
		wdt_reset();	
		*/			

		DDRB |= (1<<PB0); // debug led
		
		_delay_ms(100); // When woken up from sleep.	
		
		
			#ifdef DEBUG_MODE	// Debug Put Char init
			dbg_tx_init();			
			#endif			
				

	_delay_ms(10);	
	
			#ifdef DEBUG_MODE	
				if(bit_is_set(MCUSR, WDRF))
				{	
					
					//sei();
					dbg_puts(CRLF);
					dbg_puts("Recovery after WDT.\r\n");
					dbg_puts(CRLF);
					//cli();
					//This bit is set if a Watchdog System Reset occurs.
					off(MCUSR,WDRF); //The bit is reset by a Power-on Reset, or by writing a logic zero to the flag 
					
					#warning ka darom kai WDT reset?					
				}
			#endif	
	
	// PINSETUP:
	
			#ifdef DEBUG_MODE
			dbg_puts("Standart init started.\r\n");
			#endif				
	
	standart_init();	// WDT, INT0, TIMER0 (int0_off) - int0 veliau, po initu. 
	
	
		
	// Sleep init.		
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	
			#ifdef DEBUG_MODE
			dbg_puts("Sleep mode set.\r\n");
			#endif	
			
			// battery voltage check
			// tunr on modem, setup, test at, sleep modem
			

	while (1)
	{		
		
			#ifdef DEBUG_MODE 				
			_delay_ms(10);
			
			dbg_puts("Vbat: ");
			dbg_puti(get_vbat_voltage_mV());
			dbg_puts("mV\r\n");			
						
			dbg_puts("Temp.: ");			
			dbg_puti(get_temp_mV());
			dbg_puts("C\r\n");							
			#endif
								

			#ifdef DEBUG_MODE			
			dbg_puts("Monitoring movement.\r\n");
			#endif	
			
			
			Vibration_detect_int_on();
			sei();	// Global interrupt enable. 
			
					
			if(check_if_movement_is_legit(LEGIT_MOVEMENT_COUNT, LEGIT_MOVEMENT_TIMEOUT_S*30))
			{	
				
				// wake up modem, test at
				
				Vibration_detect_int_off();							
				send_sms_template(SMS_MODE_FIRST_FILTER_PASS);	// reik patikrint ar suveike, nes jei ne viskas griuna.
								
				Ring_detection_int_on();
				
				#ifdef DEBUG_MODE
				dbg_puts("Ring detection on.\r\n");
				#endif	
				
				Vibration_detect_int_on();	
				
				// sleeep modem		

				if(wait_for_movement_to_finish(MOVEMENT_TIMEOUT_MIN*1818)) // 1 @ 1MHz tick - 258,65ms // unsigned int so till 32k or 2h 20min.. Roughly (1 * 232) is = 1min timout, (2 * 232) = 2min timout.
				{
					
					// wake up modem, send ATH
					Ring_detection_int_off();
					
					
					#ifdef DEBUG_MODE
					dbg_puts("RING detection off.\r\n");
					#endif
					
					Vibration_detect_int_off();					
					
					#ifdef DEBUG_MODE					
					dbg_puts("Movement finished.\r\n");
					#endif
										
					//SEND_TEST_SMS(); // RIng off going to sleep. Judejimas baigesi.	
					send_sms_template(SMS_MODE_MOVEMENT_FINISHED);	
					
					// sleep modem
				}
				
				else
				{
					#ifdef DEBUG_MODE					
					dbg_puts("No movement detected in wait function.\r\n");
					#endif	
					send_sms_template(SMS_MODE_NOT_SUFFICIENT_FIRST_FILTER);									
					
				}
			}
			
			
			else
			{
				#ifdef DEBUG_MODE				
				dbg_puts("Movement was not legit.\r\n");
				#endif				
			}
													
		
			Ring_detection_int_off();
			Vibration_detect_int_on();									
			
				#ifdef DEBUG_MODE
				dbg_puts("INT0 on, Going to sleep.\r\n");
				#endif
			
			_delay_ms(100); // Fixes uart disapear.			
			sleep_enable();						
			sleep_cpu();			
			sleep_disable();
			
			_delay_ms(100); // When woken-up from sleep.
			
				#ifdef DEBUG_MODE
				dbg_puts("Woke up.\r\n");
				#endif			
							
	}
}


ISR(INT0_vect)
{
	// Vibration sensor.
	_delay_ms(20); // kad maziau int. generuotu, bet reikes sumazint kai bus suderinta sistema arba debounce
	MOVEMENT_DETECTED = 1;	
	
		
}


ISR(INT1_vect)
{
	// debounce?
	_delay_ms(20);
	RING_DETECTED = 1;		
}


ISR(TIMER0_OVF_vect){	if(TIME_OUT_COUNT>=32700)	{		TIME_OUT_COUNT = 0;	}	TIME_OUT_COUNT++;			//dbg_puts("Timer tick\r\n");		}
/*void wdt_delay(int miliseconds)
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
*/


uint8_t wait_for_movement_to_finish(unsigned int time_out_val){	uint8_t status = 0;	TIME_OUT_COUNT = 0;	TCNT0 = 0; // reset timer.			#ifdef DEBUG_MODE
	dbg_puts("Waiting for movement to finish.\r\n");
	#endif			// nes jau issiustas ispejantis sms		while(TIME_OUT_COUNT < time_out_val)	{																	if(MOVEMENT_DETECTED == 1)		{			Vibration_detect_int_off();			MOVEMENT_DETECTED = 0;			status = 1;			TIME_OUT_COUNT = 0;														#ifdef DEBUG_MODE
			dbg_puts("Event detected, postponing.\r\n");
			#endif										}							if(RING_DETECTED == 1)
		{
			Ring_detection_int_off();
			_delay_ms(10);			
			
			if(bit_is_set(RING_INT1_PIN_R, RING_INT1_PIN))
			{				
				RING_DETECTED = 0;				
				_delay_ms(500); // 150ms SMS if more then Call.
				
				if(bit_is_set(RING_INT1_PIN_R, RING_INT1_PIN))
				{					
					#ifdef DEBUG_MODE
					dbg_puts("RING detected.\r\n");
					#endif					
					
					sim900_init_uart(38400);
					#ifdef DEBUG_MODE
					dbg_puts("Answering phone call.\r\n");
					#endif
					uart_puts("ATA\r");					
				}					
			}			
		}				Ring_detection_int_on();		Vibration_detect_int_on();							}						return status;}uint8_t check_if_movement_is_legit(unsigned int legit_movement_count, unsigned int time_out_val){		unsigned char movement_count = 0;	TIME_OUT_COUNT = 0;	TCNT0 = 0; // reset timer.		#ifdef DEBUG_MODE
	dbg_puts("Checking is movement is legit.\r\n");
	#endif			while(TIME_OUT_COUNT < time_out_val)	{		if(MOVEMENT_DETECTED == 1)		{			Vibration_detect_int_off();			MOVEMENT_DETECTED = 0;						TIME_OUT_COUNT = 0;							movement_count++;						#ifdef DEBUG_MODE
			dbg_puts("Event detected, postponing.\r\n");
			dbg_puts("Movement count: ");
			
			unsigned int f1;
			f1 = movement_count;
			char str[16];
			itoa(f1, str, 10);
			dbg_puts(str);
			dbg_puts("\r\n");
			#endif						Vibration_detect_int_on();					}				if(movement_count > legit_movement_count)		{						#ifdef DEBUG_MODE
			dbg_puts("Movement is legit, changing status.\r\n");						
			#endif						return 1;					}		}		return 0;}uint8_t send_sms_template(uint8_t sms_mode){			PORTB |= (1<<PB0); // debug led on during gsm task.		#ifdef DEBUG_MODE
	wdt_reset();
	dbg_puts("GSM task...\r\n");
	#endif

	//int0_off();
	
	sim900_init_uart(38400);
	

		#ifdef DEBUG_MODE
		wdt_reset();
		dbg_puts("Checking modem setup.\r\n");
		#endif
		
		if(sim900_setup(SETUP_WAIT_TIMEOUT)) //SETUP_WAIT_INFINITE SETUP_WAIT_TIMEOUT
		{
			#ifdef DEBUG_MODE
			dbg_puts("Setup ok.\r\n");
			#endif
			
			#ifdef DEBUG_MODE
			wdt_reset();
			dbg_puts("Checking if modem is registered to network.\r\n");
			#endif
			
			if(sim900_is_network_registered())
			{
				#ifdef DEBUG_MODE
				dbg_puts("Register to network ok.\r\n");
				#endif
				
				#ifdef DEBUG_MODE
				wdt_reset();
				dbg_puts("Sending SMS.\r\n");
				#endif
				
				if(sim900_send_sms_template(number, sms_mode)) // different func.
				{
					#ifdef DEBUG_MODE
					dbg_puts("SMS sent.\r\n");
					#endif
					
					
					off(PORTB, PB0);
					return 1;
					
					// Wait for ring?
					
				}
				
				else
				{
					#ifdef DEBUG_MODE
					dbg_puts("SMS send failed.\r\n");
					#endif
				}
				
			}
			
			else
			{
				#ifdef DEBUG_MODE
				dbg_puts("Modem is not registered to network.\r\n");
				#endif
			}

		
	} ///		off(PORTB, PB0);			Vibration_detect_int_on();	return 0;}