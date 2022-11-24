#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "sounddata.h"
#define SAMPLE_RATE 8000

volatile uint16_t sample;
unsigned char lastSample;

void stopPlayback() {
    TIMSK1 &= ~_BV(OCIE1A);
    TCCR1B &= ~_BV(CS10);
    TCCR2B &= ~_BV(CS10);

    PORTB &= ~(1 << 3);
}

ISR(TIMER1_COMPA_vect) {
    if (sample >= sounddata_length) {
        if (sample == sounddata_length + lastSample) {
            stopPlayback();
        } else {
        	OCR2A = sounddata_length + lastSample - sample;
        }
    } else {
    	OCR2A = pgm_read_byte(&sounddata_data[sample]);
    }

    ++sample;
}

void startPlayback() {
	DDRB |= 1 << 3;
	ASSR &= ~(_BV(EXCLK) | _BV(AS2));
	
	TCCR2A |= _BV(WGM21) | _BV(WGM20);
	TCCR2B &= ~_BV(WGM22);
	
	TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
	TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
	TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
	OCR2A = pgm_read_byte(&sounddata_data[0]);
	
	cli();
	
	TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
	TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
	TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
	OCR1A = F_CPU / SAMPLE_RATE;
	
	TIMSK1 |= _BV(OCIE1A);
	
	lastSample = pgm_read_byte(&sounddata_data[sounddata_length-1]);
	sei();
}

int main(void) {
	
	startPlayback();

	for(;;) {
	}

	return 0;
}
