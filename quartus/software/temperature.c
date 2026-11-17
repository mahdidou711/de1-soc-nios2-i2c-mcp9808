#include "temperature.h"
#include "I2C.h"

int read_temperature(int *hundredths)
{
    unsigned char high, low;
    int value;

    start_i2c();

    if (write_i2c(0x30)) {
        stop_i2c();
        return -1;
    }

    if (write_i2c(0x05)) {
        stop_i2c();
        return -1;
    }

    start_i2c();

    if (write_i2c(0x31)) {
        stop_i2c();
        return -1;
    }

    high = read_i2c(0);
    low = read_i2c(1);
    stop_i2c();

    /* Bits 15:13 are alerts, bit 12 is the sign and bits 11:0 are the lower bits
       of the 13-bit two's-complement temperature in sixteenths of a degree. */
    value = ((high & 0x0F) << 8) | low;
    value = value * 100 / 16;

    if (high & 0x10)
        value -= 25600;

    *hundredths = value;
    return 0;
}

unsigned int temperature_display_bcd(int hundredths)
{
    unsigned int value, d0, d1, d2, d3;

    if (hundredths < 0 || hundredths > 9999)
        return 0;

    value = (unsigned int)hundredths;
    d0 = value % 10;
    d1 = (value / 10) % 10;
    d2 = (value / 100) % 10;
    d3 = (value / 1000) % 10;

    return (d3 << 12) | (d2 << 8) | (d1 << 4) | d0;
}
