# System architecture

The values below are derived from `quartus/nios_system.qsys`, `quartus/thermometer.qsf` and the C sources.

## Processor and clocking

The Nios II Gen2 Fast (/f) processor has a 4 KiB instruction cache and a 2 KiB data cache. Reset and exception vectors are at `sdram.s1 + 0x0` and `sdram.s1 + 0x20`. The system clock is 50 MHz from `CLOCK_50` through `sdram_pll` output `c1`; output `c0`, shifted by -3 ns, clocks SDRAM.

For peripheral accesses the firmware sets address bit 31, for example `0x04000000` becomes `0x84000000`, to bypass the data cache. The build uses `-mno-cache-volatile`.

## Data-master address map

End addresses are exclusive.

| Slave | Start | End | C address | IRQ |
|---|---:|---:|---:|---:|
| `sdram.s1`, 64 MiB | `0x00000000` | `0x04000000` | code and stack | - |
| `Timer2.s1` | `0x04000000` | `0x04000020` | `0x84000000` | 4 |
| `I2C.s1`, 8-bit bidirectional PIO | `0x04000020` | `0x04000030` | `0x84000020` | - |
| `cpu.debug_mem_slave` | `0x04000800` | `0x04001000` | - | - |
| `LEDR.s1`, 10-bit output | `0x04001000` | `0x04001010` | `0x84001000` | - |
| `HEX_PIO.s1`, 16-bit output | `0x04002000` | `0x04002010` | `0x84002000` | - |
| `Onchip_memory.s1`, 8 KiB | `0x09000000` | `0x09002000` | unused | - |
| `JTAG_UART.avalon_jtag_slave` | `0x10001000` | `0x10001008` | `0x10001000` | 8 |
| `Interval_timer.s1` | `0x10002000` | `0x10002020` | `0x90002000` | 0, disabled |

## Interrupts and interfaces

Timer2 runs continuously with a count of `0xBEBC20` (12,500,000 cycles, 250 ms at 50 MHz). Firmware enables IRQ 4 through `ienable = 0x10` and enables global interrupts. `interrupt_handler()` checks `ipending`, acknowledges the timer and calls `Timer2_irq()`, which sets `tick_250ms`. The interval timer supplies I2C delays.

The I2C PIO uses bit 1 for SCL and bit 0 for SDA. SCL is driven directly on `GPIO_1[11]`; SDA is open-drain on `GPIO_1[12]`, driven low or released to high impedance. Both lines require external pull-ups, supplied by the sensor module in the reference setup. The design assumes a single master. Effective SCL timing depends on software execution and has not been measured.

`HEX_PIO` exports four BCD nibbles: bits 15:12 through 3:0 represent tens, units, tenths and hundredths. The `hex7seg` function drives active-low segments on HEX3 through HEX0; invalid BCD digits blank the corresponding display. HEX4 and HEX5 remain blank. LEDR displays a progressing indicator.

## MCP9808 transaction

With A2, A1 and A0 low, the 7-bit address is `0x18`. The firmware writes address byte `0x30` and ambient temperature register pointer `0x05`, issues a repeated START, then writes `0x31` and reads two bytes, ACKing the first and NACKing the second. It converts sixteenths of a degree to hundredths, handles the sign bit, and ignores the alert bits. Temperatures below 0 °C or above 99.99 °C are printed but displayed as zero on HEX.

## Selected FPGA pins

| Signal | Pin | Signal | Pin |
|---|---|---|---|
| `CLOCK_50` | `PIN_AF14` | `KEY[0]` reset | `PIN_AA14` |
| `GPIO_1[11]` SCL | `PIN_AH24` | `GPIO_1[12]` SDA | `PIN_AH27` |
| `HEX0[6:0]` | AH28, AG28, AF28, AG27, AE28, AE27, AE26 | `HEX1[6:0]` | AD27, AF30, AF29, AG30, AH30, AH29, AJ29 |
| `HEX2[6:0]` | AC30, AC29, AD30, AC28, AD29, AE29, AB23 | `HEX3[6:0]` | AB22, AB25, AB28, AC25, AD25, AC27, AD26 |
