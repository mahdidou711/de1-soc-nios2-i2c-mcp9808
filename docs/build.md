# Build and reproduction

## Prerequisites

The reference environment used Intel Quartus Prime 18.1.0 Build 625 Standard Edition with Platform Designer, Intel FPGA Monitor Program 18.1 and its Nios II GCC 5.3.0 toolchain. Hardware consists of a Terasic DE1-SoC with USB-Blaster II and a 3.3 V MCP9808 module with pull-ups on SCL and SDA. Connect SCL to `GPIO_1[11]` (FPGA pin AH24), SDA to `GPIO_1[12]` (AH27), 3.3 V and common ground. Consult the DE1-SoC manual for physical connector positions.

Quartus 18.1 Lite supports the Cyclone V device, but availability of the Nios II/f core with a given license has not been verified.

## Generate and compile hardware

1. Open `quartus/thermometer.qpf` in Quartus Prime and `quartus/nios_system.qsys` in Platform Designer.
2. Check the address map against [architecture](architecture.md). Generate VHDL into `quartus/nios_system/`; generation also creates `quartus/nios_system.sopcinfo` for Monitor Program.
3. Run Processing > Start Compilation. Review the flow and Timing Analyzer reports, including unconstrained paths. The programming file is `quartus/thermometer.sof`.
4. In Tools > Programmer, select the DE-SoC hardware, detect the JTAG chain, associate `thermometer.sof` with the FPGA and start programming. The untracked chain file can be recreated by Programmer.

## Compile and run firmware

1. Open `quartus/software/thermometer.amp` in Intel FPGA Monitor Program. It refers to the generated `../nios_system.sopcinfo` and `../thermometer.sof`.
2. Use Actions > Download System if the FPGA has not already been programmed, then Actions > Compile & Load and Actions > Continue.
3. The tracked CLI makefile uses the reference options `-g -O1 -ffunction-sections -fverbose-asm -fno-inline -mno-cache-volatile -mno-hw-mul -mno-hw-div`.

For a CLI build, put the Nios II tools on `PATH`, set `NIOS2_EDS_ROOT` to
the Nios II EDS installation directory and `MONITOR_BUILD_DIR` to the
directory containing `nios_cpp_build.ld`, then run:

```sh
make -C quartus/software COMPILE
```

This produces `thermometer.elf` and `thermometer.srec` in the software directory.
Monitor Program can generate a separate makefile for its own installation.

Expected observations: the JTAG console prints an initial period and a temperature such as `Temperature = 23.75 C` approximately once per second; HEX3..HEX0 show `2375`; LEDR advances every 250 ms; KEY0 resets the system. A missing acknowledgment produces `MCP9808 I2C error` and clears the display, provided SDA has an external pull-up. If the pull-ups are on the disconnected sensor module, SDA floats and the acknowledgment is indeterminate; this case was not tested on hardware.

See [host checks](verification.md#host-checks) for verification without the
Intel toolchain or a board.
