#include <stdio.h>

#include "interval_timer.h"
#include "nios2_ctrl_reg_macro.h"
#include "sprint.h"
#include "temperature.h"

#define HEX  *((volatile unsigned int *) 0x84002000)
#define LEDR  *((volatile unsigned short *) 0x84001000)

static const unsigned short led_patterns[] = {
    0x001, 0x003, 0x007, 0x00F, 0x01F, 0x03F, 0x07F, 0x0FF, 0x1FF, 0x3FF, 0x000
};
static volatile char tick_250ms;

void Timer2_irq(void)
{
    tick_250ms = 1;
}

static void init_timer2_interrupt(void)
{
    volatile int *timer2 = (int *) 0x84000000;
    int counter = 0xBEBC20; // 250 ms at 50 MHz

    *(timer2 + 0x2) = (counter & 0xFFFF);
    *(timer2 + 0x3) = (counter >> 16) & 0xFFFF;
    *(timer2 + 1) = 0x7; // START, CONT and ITO
    NIOS2_WRITE_IENABLE(0x10); // Timer2 IRQ 4
    NIOS2_WRITE_STATUS(1); // Enable Nios II interrupts
}

int main(void)
{
    unsigned int led_index = 0;
    unsigned int ticks = 0;
    char buffer[80];

    getperiod();
    start_timer();
    init_timer2_interrupt();
    getsnap();

    sprint(buffer, "Period=%u", (unsigned int)getperiod());
    puts(buffer);

    HEX = 0;

    while (1) {
        if (tick_250ms == 1) {
            tick_250ms = 0;
            LEDR = led_patterns[led_index++];
            led_index = (led_index >= 11) ? 0 : led_index;
            ticks++;
        }

        if (ticks > 3) {
            int hundredths;

            ticks = 0;
            if (read_temperature(&hundredths) == 0) {
                int negative = (hundredths < 0);
                int magnitude = negative ? -hundredths : hundredths;

                if (negative)
                    sprint(buffer, "Temperature = -%u.%u%u C",
                           (unsigned int)(magnitude / 100),
                           (unsigned int)((magnitude % 100) / 10),
                           (unsigned int)(magnitude % 10));
                else
                    sprint(buffer, "Temperature = %u.%u%u C",
                           (unsigned int)(magnitude / 100),
                           (unsigned int)((magnitude % 100) / 10),
                           (unsigned int)(magnitude % 10));

                puts(buffer);
                HEX = temperature_display_bcd(hundredths);
            } else {
                puts("MCP9808 I2C error");
                HEX = 0;
            }
        }
    }
}
