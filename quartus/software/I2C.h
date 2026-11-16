
#ifndef I2C_H
#define I2C_H

/*********************************************************/
// bit1 SCL   bit0 SDA

void attente(unsigned x);
void start_i2c(void);
void stop_i2c(void);
char write_i2c(unsigned char x);
unsigned char read_i2c(char ack);

#endif
