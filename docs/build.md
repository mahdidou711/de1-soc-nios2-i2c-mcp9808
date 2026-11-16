# Build and reproduction

## Prerequisites

The reference environment used Intel Quartus Prime 18.1.0 Build 625 Standard Edition with Platform Designer, Intel FPGA Monitor Program 18.1 and its Nios II GCC 5.3.0 toolchain. Hardware consists of a Terasic DE1-SoC with USB-Blaster II and a 3.3 V MCP9808 module with pull-ups on SCL and SDA. Connect SCL to `GPIO_1[11]` (FPGA pin AH24), SDA to `GPIO_1[12]` (AH27), 3.3 V and common ground. Consult the DE1-SoC manual for physical connector positions.

Quartus 18.1 Lite supports the Cyclone V device, but availability of the Nios II/f core with a given license has not been verified.

## Generate and compile hardware

1. Open `quartus/TP.qpf` in Quartus Prime and `quartus/nios_system.qsys` in Platform Designer.
2. Check the address map against [architecture](architecture.md). Generate VHDL into `quartus/nios_system/`; generation also creates `quartus/nios_system.sopcinfo` for Monitor Program.
3. Run Processing > Start Compilation. Review the flow and Timing Analyzer reports, including unconstrained paths. The programming file is `quartus/TP.sof`.
4. In Tools > Programmer, select the DE-SoC hardware, detect the JTAG chain, associate `TP.sof` with the FPGA and start programming. The untracked chain file can be recreated by Programmer.

## Compile and run firmware

1. Open `quartus/software/tp1.amp` in Intel FPGA Monitor Program. It refers to the generated `../nios_system.sopcinfo` and `../TP.sof`.
2. Use Actions > Download System if the FPGA has not already been programmed, then Actions > Compile & Load and Actions > Continue.
3. Monitor Program regenerates the makefile with local installation paths. The tracked makefile records the reference options: `-g -O1 -ffunction-sections -fverbose-asm -fno-inline -mno-cache-volatile -mno-hw-mul -mno-hw-div`.

Expected observations: the JTAG console prints an initial period and a temperature such as `Temperature = 23.75 C` approximately once per second; HEX3..HEX0 show `2375`; LEDR advances every 250 ms; KEY0 resets the system. A missing acknowledgment produces `Erreur I2C MCP9808` and clears the display, provided SDA has an external pull-up. If the pull-ups are on the disconnected sensor module, SDA floats and the acknowledgment is indeterminate; this case was not tested on hardware.

## Static checks

```sh
ghdl -s --std=08 quartus/DE1_SoC_Basic_Computer.vhd
```

The GHDL command checks top-level syntax only; it does not elaborate Intel IP or replace Quartus compilation or board testing.
