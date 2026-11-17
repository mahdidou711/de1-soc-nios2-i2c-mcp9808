#include "interval_timer.h"

/*
 * Platform Designer assigns the interval timer base address 0x10002000.
 * Bit 31 selects the uncached address on Nios II.
 */
#define INTERVAL_TIMER_BASE 0x90002000u

unsigned int Timer_period;

int getperiod(void)
{
    unsigned int val;
    val = *(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 2) |
          (*(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 3) << 16);
    Timer_period = val;
    return (int)val;
}

void start_timer(void)
{
    *(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 1) = 0x06u;
}

unsigned int getsnap(void)
{
    unsigned int val;
    *(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 4) = 0u;
    val = *(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 4) |
          (*(((volatile unsigned int *)INTERVAL_TIMER_BASE) + 5) << 16);
    return val;
}

unsigned int getsnapdif(unsigned int v1, unsigned int v2)
{
    return (v1 >= v2) ? (v1 - v2) : (Timer_period - v2 + v1);
}
