#include "interval_timer.h"
#include "I2C.h"

#define I2C  *((volatile unsigned int *)0x84000020)
/*********************************************************/
// bit1 SCL   bit0 SDA

void delay_us(unsigned x){
		unsigned v1;
		v1= getsnap();
		do {
		} while(getsnapdif(v1,getsnap())<x); //  The interval timer uses 1 us ticks.

}

void start_i2c(void){
		I2C = 3;
		delay_us(1); // 1 us
		I2C = 2;
		delay_us(1); // 1 us
		I2C = 0;
		delay_us(1); // 1 us
}
void stop_i2c(void){
		I2C = 0; //Drive SDA and SCL low
		delay_us(1);
		I2C= 2; // Drive SCL high
		delay_us(1);
		I2C= 3;
		delay_us(2); // 2 us
}

char write_i2c(unsigned char x){
		unsigned short i;
		char bit,ack;
		
		for(i=0; i<8; i++){
		    bit=((x&0x80) ? 1 : 0);
				I2C = bit;
				delay_us(2);
				I2C = 2| bit;
				x=x<<1;
				delay_us(2);
				I2C = bit;
				delay_us(1);
		}
		I2C=1; // ACK bit
		delay_us(1);
		I2C=3;
		delay_us(1);
		ack = I2C&0x01;
		delay_us(1);
		I2C=01;
		delay_us(1);
		return ack;
}

unsigned char read_i2c(char ack){
		unsigned short i;
		unsigned char x = 0;
		for(i=0; i<8; i++){
				I2C = 3;
				delay_us(1);
				x=x<<1;
				x= x| (I2C&0x01);
				delay_us(1);
				I2C= 01;
				delay_us(1);
		}
		I2C=(ack&1); // ACK bit
		delay_us(1);
		I2C= 2 |  (ack&1);
		delay_us(1);
		I2C =(ack&1);
		delay_us(1);
		I2C = 01;
		return x;	
}
