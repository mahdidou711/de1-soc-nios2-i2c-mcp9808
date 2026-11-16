# DE1-SoC Nios II I2C thermometer

A Nios II/f system on the Terasic DE1-SoC reads a Microchip MCP9808 temperature sensor through a software-driven I2C master and displays the result on four seven-segment digits and a JTAG UART console. The FPGA is a Cyclone V 5CSEMA5F31C6.

## System architecture

```text
                   Nios II/f
                       |
                 Avalon-MM interconnect
      +----------+-----+--------+----------+-----------+
      |          |              |          |           |
   SDRAM     JTAG UART       Timer2      I2C PIO     HEX_PIO
  64 MiB      console       IRQ 4         |          16-bit BCD
                            250 ms     GPIO_1           |
                                       SCL/SDA     VHDL decoder
                                          |         HEX3..HEX0
                                       MCP9808
```

Timer2 interrupts every 250 ms; the main loop advances an LED indicator and reads the sensor every four interrupts. The I2C primitives drive SCL and SDA through GPIO. `lire_temperature()` reads the ambient temperature register and converts its signed fixed-point value to hundredths of a degree. `conversion_BCD()` packs four decimal digits for the VHDL decoder. For example, 23.75 °C is displayed as `2375`. Negative values and values above 99.99 °C are printed on the console but result in `0000` on the displays. An I2C acknowledgment error is reported on the console and clears the display.

## Memory and interrupt map

| Peripheral | Avalon base | CPU address used by firmware | IRQ |
|---|---:|---:|---:|
| SDRAM, 64 MiB | `0x00000000` | code and stack | - |
| Timer2 | `0x04000000` | `0x84000000` | 4 |
| I2C PIO | `0x04000020` | `0x84000020` | - |
| LEDR PIO | `0x04001000` | `0x84001000` | - |
| HEX PIO | `0x04002000` | `0x84002000` | - |
| JTAG UART | `0x10001000` | `0x10001000` | 8 |
| Interval timer | `0x10002000` | `0x90002000` | 0, not enabled |
| On-chip memory, 8 KiB | `0x09000000` | unused | - |

The complete map, cache bypass convention, wiring and pin assignments are in [architecture](docs/architecture.md).

## Build and run

The reference environment uses Quartus Prime 18.1 Standard Edition, Platform Designer and Intel FPGA Monitor Program 18.1 with the Nios II toolchain. Connect a 3.3 V MCP9808 module with pull-ups: SCL to `GPIO_1[11]`, SDA to `GPIO_1[12]`, and common ground.

1. Open `quartus/TP.qpf`, then generate VHDL for `quartus/nios_system.qsys` in Platform Designer.
2. Compile the Quartus project and program the generated `TP.sof` onto the FPGA.
3. Open `quartus/software/tp1.amp` in Intel FPGA Monitor Program, compile and load the firmware, then start it.

See [build and reproduction](docs/build.md) for prerequisites, generated files and expected behavior.

## Verification and limits

The reference Quartus 18.1 build reported successful synthesis, fitting and timing analysis. Its timing margins apply only to constrained paths; 19 input and 79 output ports were unconstrained, so external interface timing is not covered. The system was previously verified on a DE1-SoC board, but was not rerun on the current host and no recording is published. SCL timing has not been measured. Historical resource and timing summaries and their limits are in [verification evidence](docs/verification.md).

## Repository and licensing

`quartus/` contains the hardware and firmware sources; `docs/` contains architecture, reproduction and evidence. Generated Intel IP, build reports and programming binaries are not tracked.

The [MIT License](LICENSE) applies to Mehdi Bouama's original contributions only. Starter material and third-party components retain their own terms; `sprint.c` and `sprint.h` are covered by LGPL version 2 or later, with the [LGPL-2.1 text](LICENSE-LGPL-2.1) provided. See [provenance](NOTICE_PROVENANCE.md) for attribution and licensing limits.
