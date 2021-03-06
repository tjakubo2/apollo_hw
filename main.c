#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
//#include <util/delay.h>
#include <stdio.h>
#include <avr/sleep.h>

#include "LCD/lcd44780.h"
#include "I2C_TWI/i2c_twi.h"
#include "MMA7455/mma7455_wds.h"
#include "UART/uart_wds.h"

#define STRLEN 40
char out[STRLEN];

void sendMeas()
{
	ADCSRA |= (1<<ADSC);

	static meas_8b meas;
	static uint16_t adcMeas;

	meas = mma_measure8b();

	//sprintf(out, "X:%4d Y:%4d Z:%4d \r\n", meas.x, meas.y, meas.z);
	//uart_puts(out);
	//sprintf(out, "b %d %d %d e", meas.x, meas.y, meas.z);
	while(ADCSRA & (1<<ADSC));
	adcMeas = ADC;
	int16_t checksum = (meas.x ^ meas.y ^ meas.z ^ adcMeas);
	sprintf(out, "b %d %d %d %d %d e", meas.x, meas.y, meas.z, adcMeas, checksum);
	uart_puts(out);
	return;
}

ISR( USART_RXC_vect ) {
    uint8_t tmp_head;
    char data;
    data = UDR;
    tmp_head = ( UART_RxHead + 1) & UART_RX_BUF_MASK;
    if ( tmp_head == UART_RxTail ) {}
    else {
	UART_RxHead = tmp_head;
	UART_RxBuf[tmp_head] = data;
    }
/*
    char rec = uart_getc();
    switch(rec)
    {
    case 0: break;
    case 'm':
    	sendMeas();
    }
*/

}

int main(void) {

	DDRC |= (1<<PC2) | (1<<PC3) | (1<<PC4);
	PORTC |= (1<<PC2) | (1<<PC3);
	PORTC &= ~(1<<PC4);

	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

	i2cSetBitrate( 125 );
	mma_wds_init();
	USART_Init( __UBRR );
	//set_sleep_mode(SLEEP_MODE_IDLE);
	sei();

	//while(1) { sleep_mode(); }
	while(1){
		//sendMeas();
		char rec = uart_getc();
		if(rec == 'm') sendMeas();
	}

}

