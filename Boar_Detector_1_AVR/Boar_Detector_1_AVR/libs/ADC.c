/*
 * ADC.cpp
 *
 * Created: 2014.12.27 15:11:58
 *  Author: Kieran
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>


//#define ADC2 0b00000010 // 2
//#define ADC3 0b00000011 // 3
//#define ADC8 0b00001110 // 14 1.3V
//#define ADC9 0b00001111 // 15 GND



unsigned int get_vbat_voltage()
{
	
	// ADC.
	
	//REFS1 REFS0	Voltage Reference Selection
	//0		0		AREF, Internal Vref turned off
	
	// DO NOT SELECT INT VREF!!!! VOLTAGE IS CONNECTED THERE.
	
	/*
	MUX3...0	Single Ended Input
	0000		ADC0
	0001		ADC1
	0010		ADC2
	0011		ADC3
	0100		ADC4
	0101		ADC5
	0110		ADC6
	0111		ADC7
	1000		ADC8(1)
	*/
	
	ADMUX |= (1<<MUX0) | (1<<MUX1); // ADC3 select on PC3
	
	/*
	ADPS2 ADPS1 ADPS0 Division Factor
	0		0		0	2
	0		0		1	2
	0		1		0	4
	0		1		1	8
	1		0		0	16
	1		0		1	32
	1		1		0	64
	*/
	
	ADCSRA |= (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); // Prescaler - 8
	ADCSRA |= (1<<ADEN); // ADC enable
	
	
	ADCSRA  |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC)); // ADSC will read as one as long as a conversion is in progress. When the conversion is complete, it returns to zero.
	
	ADCSRA &= ~(1 << ADEN); // ADC disable.	
	
	
	
	return ADCW;			
}


