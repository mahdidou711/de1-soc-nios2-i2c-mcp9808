#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/* Returns -1 on NACK, leaving the output unchanged. */
int read_temperature(int *hundredths);

/* Packs four decimal digits. A negative or out-of-range value displays 0000. */
unsigned int temperature_display_bcd(int hundredths);

#endif
