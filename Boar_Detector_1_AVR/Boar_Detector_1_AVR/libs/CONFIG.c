/*
 * PIN_CONFIG.c
 *
 * Created: 2017.03.20 21:05:25
 *  Author: Kieran
 */ 


#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>


//*** MISQ ****
#define off(a,z) (a &= ~_BV(z))
#define on(a,z) (a |= _BV(z))
#define change(a,z) (a ^=_BV(z))
#define bit_is_clear(sfr,bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))


// ***** INPUTs/OUTPUTs ******

#define ADC_VBAT_PIN PC3

#define DIN1_PIN PD4
#define DIN1_PIN_R PIND // Needs pullup //DIN1_PORT_R |= (1<<PD4);
#define DIN1_PORT_R PORTD

//on(DIN1_PORT_R,DIN1_PIN);

#define PWRKEY_PIN PD5
#define PWRKEY_DDR_R DDRD //output
#define PWRKEY_PORT_R PORTD

//on(PWRKEY_DDR_R,PWRKEY_PIN); // Set to output.
//on(PWRKEY_PORT_R,PWRKEY_PIN); // togle pin
//off(PWRKEY_PORT_R,PWRKEY_PIN); // togle pin

#define DTR_PIN PD6
#define DTR_DDR_R DDRD //output
#define DTR_PORT_R PORTD

//on(DTR_DDR_R,PWRKEY_PIN); // Set to output.
//on(DTR_PORT_R,PWRKEY_PIN); // togle pin
//off(DTR_PORT_R,PWRKEY_PIN); // togle pin

#define M_RST_PIN PD7
#define M_RST_DDR_R DDRD //output
#define M_RST_PORT_R PORTD

//on(DTR_DDR_R,PWRKEY_PIN); // Set to output.
//on(DTR_PORT_R,PWRKEY_PIN); // togle pin
//off(DTR_PORT_R,PWRKEY_PIN); // togle pin

#define VIBR_INT0_PIN PD2
#define VIBR_INT0_PORT_R PORTD // Needs pullup //DIN1_PORT_R |= (1<<PD4);

//on(VIBR_INT0_PORT_R,VIBR_INT0_PIN);

#define RING_INT1_PIN PD3
#define RING_INT1_PORT_R PORTD

//on(RING_INT1_PORT_R,RING_INT1);

	
void pin_cfg()

{
	
	//The DDxn bit in the DDRx Register selects the direction of this pin. If DDxn is written logic one, Pxn is
	//configured as an output pin. If DDxn is written logic zero, Pxn is configured as an input pin.
	//If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is activated.
	
	/*
	When switching between tri-state ({DDxn, PORTxn} = 0b00) and output high ({DDxn, PORTxn} = 0b11), an
	intermediate state with either pull-up enabled {DDxn, PORTxn} = 0b01) or output low ({DDxn, PORTxn} = 0b10)
	must occur.
	*/
	
		
	// Inputs
	
	on(DIN1_PORT_R,DIN1_PIN); // Pull- UP
	
	// Outputs.
	
	on(PWRKEY_DDR_R,PWRKEY_PIN); // OUTPUT, start as LOW
	on(DTR_DDR_R, DTR_PIN);
	on(M_RST_DDR_R,M_RST_PIN);
	
	
}

void timer0_init()
{
	//TCCR0 |= (1<<CS00) | (1<<CS02); // precaler	//TIMSK |= (1<<TOIE0); //interuupt enable.s
}

void int_init()
{
	// Interrupts
	
	#warning INT0 turi buti and LOW level.
	
	on(VIBR_INT0_PORT_R,VIBR_INT0_PIN); // INT0 Pull-Up enable.
	on(MCUCR, ISC01);     // The falling edge of INT0 generates an interrupt request.	
	
	on(RING_INT1_PORT_R,RING_INT1_PIN); // INT0 Pull-Up enable.
	on(MCUCR, ISC11);     // The falling edge of INT1 generates an interrupt request.
	
	#warning Maybe INT1 on low level?
	
	on(GICR, INT0); // INT0 external interrupt enable.
	on(GICR, INT1); // INT1 external interrupt enable.
}

void int0_off()
{
	off(GICR, INT0);
}

void int0_on()
{
	on(GICR, INT0);
}

void standart_init()
{
		
	// WDT init.
	wdt_enable(WDTO_1S); // Let's try 8s wdt.
	#warning Clearing the watchdog reset flag before disabling the watchdog is required, according to the datasheet.
	#warning wdt_reset needs cli instructions before execution.
	wdt_reset();	
	
	// Interrupts
	
	#warning INT0 turi buti and LOW level.
	
	on(VIBR_INT0_PORT_R,VIBR_INT0_PIN); // INT0 Pull-Up enable.
	on(GICR, ISC01);     // The falling edge of INT0 generates an interrupt request.
		
	on(GICR, INT0); // INT0 external interrupt enable.
	
	timer0_init();
	
}