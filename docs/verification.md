# Verification evidence

The results below are historical unless otherwise stated. Full reports and generated binaries are not tracked. The reference build used Quartus Prime 18.1.0 Build 625 Standard Edition, top-level `DE1_SoC_Basic_Computer`. See [build and reproduction](build.md) for the procedure.

## Reference Quartus build

Platform Designer generation, analysis and synthesis, fitting, assembly and timing analysis were reported successful. The flow reported no negative timing slack.

| Fitter resource | Reported use |
|---|---:|
| ALMs | 2,153 / 32,070 (7%) |
| Registers | 3,175 |
| Pins | 178 / 457 (39%) |
| Memory bits | 131,200 / 4,065,280 (3%) |
| RAM blocks | 24 / 397 (6%) |
| DSP blocks | 3 / 87 (3%) |
| PLLs | 1 / 6 |

| Timing analysis | Minimum slack, all corners | Domain |
|---|---:|---|
| Setup | +9.530 ns | system clock |
| Hold | +0.126 ns | JTAG clock (`altera_reserved_tck`) |
| Recovery | +14.747 ns | system clock |
| Removal | +0.270 ns | system clock |
| Minimum pulse width | +1.000 ns | PLL |

Reported total negative slack was zero. These margins cover constrained internal paths only. The SDC declares `CLOCK_50`, derived PLL clocks and clock uncertainty; it contains no input or output delay constraints. The report listed 19 unconstrained input ports and 79 unconstrained output ports, so SDRAM, GPIO and display pin timing is not established by these margins.

## Reference firmware correspondence

A prior `readelf` inspection of the reference executable found the temperature read, BCD conversion and interrupt symbols along with the positive and negative console format strings. It recorded GNU C11 5.3.0 with `-O1 -g -mno-cache-volatile -mno-hw-mul -mno-hw-div`. These observations apply to the pre-refactor reference build.

| Untracked reference artifact | SHA-256 |
|---|---|
| FPGA configuration (.sof) | `9765f4e60d2075a77117487b6611603376a69b738c0f65fc0da28ae1461015d3` |
| Nios II executable (.elf) | `cb20df6610844ce8f60838664256d134452fdab3966ac55718cab281b1e45039` |
| Nios II load image (.srec) | `07979987d989dfae65c1a06312d56e242fb6603c4cd310e8843877e22a889549` |

## Board observation

Console temperature output, the LED indicator and HEX display were previously observed on a DE1-SoC board. No capture is published. The hardware was not rerun on the current host. SCL period and disconnected-sensor behavior have not been measured or tested on the board.

## Host checks

Run `make test` for the firmware's MCP9808 register read and BCD display
conversion with a scripted I2C bus. The tests check signed values, all
eight alert-bit combinations for each input, the BCD display bounds, and
STOP and output preservation for NACK on each transmitted byte.

With GHDL installed, the same command checks the top-level VHDL and
SDRAM PLL wrapper. A test-only component declaration provides the
`altpll` interface for the syntax check. FPGA implementation and vendor
IP behavior require the Intel flow.
