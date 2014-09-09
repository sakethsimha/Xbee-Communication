/*
 * XbeeHAL.h
 *
 * Created: 1/15/2014 3:02:11 PM
 *  Author: Waron
 */ 

#ifndef XBEEHAL_H_
#define XBEEHAL_H_

#include "XbeeS2.h"

//#define ATmega328P
#define ATmega2560

// Register Names for selected microcontroller
#ifdef ATmega328P
	#include <avr/io.h>
	#include <avr/interrupt.h>

	// USART Definitions
	#ifdef F_CPU
		#pragma message ("F_CPU already defined.")
	#else
		#define F_CPU 16000000	//be sure to adjust for various clock settings
	#endif
	#define USART_BAUDRATE 9600
	#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

	// Registers
	#define XBEE_UDR UDR0

	// Macros	
	#define RX_BUFFER_IS_FULL() ((UCSR0A & (1 << RXC0)) == 0)
	#define TX_BUFFER_IS_FULL() ((UCSR0A & (1 << UDRE0)) == 0)
	#define TOGGLE_LED() (PORTD ^= 0x80)
	#define LED_OUTPUT() (DDRD |= 0x80)
	
	// Functions
	ISR(USART_RX_vect) { rxISR(); }

	/* Code to initialize USART
	 *
	 * Eventually, this function should receive a USART number (e.g., 0-X, X being the number of USARTs available
	 * on the chip) and a baud rate. For now, we will use the #define variables to set this.
	 */		
	void USART_INIT(void)
	{
		UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
		UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register

		UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); // Use 8-bit character sizes
		UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);   // Turn on the transmission and reception circuitry
		// Enable RX interrupt as well.
	}

#endif

#ifdef ATmega2560
	#include <avr/io.h>
	#include <avr/interrupt.h>

	// USART Definitions
	#ifdef F_CPU
		#pragma message ("F_CPU already defined.")
	#else
		#define F_CPU 8000000	//be sure to adjust for various clock settings
	#endif
	#define USART_BAUDRATE 9600
	#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

	// Registers
	#define USART_NO 3
	#define XBEE_UDR UDR3	

	// Macros
	#define RX_BUFFER_IS_FULL() ((UCSR3A & (1 << RXC3)) == 0)
	#define TX_BUFFER_IS_FULL() ((UCSR3A & (1 << UDRE3)) == 0)
	#define TOGGLE_LED() (PORTD ^= 0x80)
	#define LED_OUTPUT() (DDRD |= 0x80)
	
	// Functions
	ISR(USART3_RX_vect) { rxISR(); }
	
	/* Code to initialize USART
	 *
	 * Eventually, this function should receive a USART number (e.g., 0-X, X being the number of USARTs available
	 * on the chip) and a baud rate. For now, we will use the #define variables to set this.
	 */
	void USART_INIT(void)
	{
		UBRR3L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
		UBRR3H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register

		UCSR3C |= (1 << UCSZ30) | (1 << UCSZ31); // Use 8-bit character sizes
		UCSR3B |= (1 << RXCIE3) | (1 << RXEN3) | (1 << TXEN3);   // Turn on the transmission and reception circuitry
		// Enable RX interrupt as well.
	}
#endif


#endif /* XBEEHAL_H_ */
